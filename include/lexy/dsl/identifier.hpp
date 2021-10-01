// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_IDENTIFIER_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/lexeme.hpp>

//=== identifier ===//
namespace lexy
{
/// Error when we matched a reserved.
struct reserved_identifier
{
    static LEXY_CONSTEVAL auto name()
    {
        return "reserved identifier";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Leading, typename Trailing>
struct _idp : token_base<_idp<Leading, Trailing>>
{
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Leading, Reader> leading;
        typename Reader::iterator               end;

        constexpr explicit tp(const Reader& reader) : leading(reader), end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match Leading character.
            if (!leading.try_parse(reader))
                return false;
            reader.set_position(leading.end);
            end = leading.end;

            // Match zero or more trailing characters.
            while (lexy::try_match_token(Trailing{}, reader))
            {}

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            leading.report_error(context, reader);
        }
    };
};

template <typename R>
struct _contains : token_base<_contains<R>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            while (true)
            {
                if (lexy::try_match_token(lexy::dsl::token(R{}), reader))
                    // We've found it.
                    break;
                else if (reader.peek() == Reader::encoding::eof())
                    // Haven't found it.
                    return false;
                else
                    // Try again.
                    reader.bump();
            }

            // Consume everything else.
            lexy::try_match_token(lexy::dsl::any, reader);

            end = reader.position();
            return true;
        }

        // report_error() not actually needed.
    };
};

template <typename Id, typename CharT, CharT... C>
struct _kw;

template <typename Leading, typename Trailing, typename... Reserved>
struct _id : branch_base
{
    using _impl = _capt<_idp<Leading, Trailing>>;

    template <typename NextParser>
    struct _pc_impl
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Last argument is the captured lexeme.
            auto lexeme = (args, ...);

            // Check whether we have a reserved identifier.
            auto id_reader = lexy::partial_reader(reader, lexeme.begin(), lexeme.end());
            if (lexy::try_match_token((Reserved{} / ...), id_reader)
                && id_reader.position() == lexeme.end())
            {
                // We found a reserved identifier.
                auto err
                    = lexy::error<Reader, lexy::reserved_identifier>(lexeme.begin(), lexeme.end());
                context.on(_ev::error{}, err);
                // But we can trivially recover, as we've still matched a well-formed
                // identifier.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    // If we don't have any reserved identifiers, we immediately continue with the next parser.
    template <typename NextParser>
    using _pc = std::conditional_t<sizeof...(Reserved) == 0, NextParser, _pc_impl<NextParser>>;

    template <typename NextParser>
    using p = lexy::parser_for<_impl, _pc<NextParser>>;
    template <typename Context, typename Reader>
    using bp = lexy::continuation_branch_parser<_impl, Context, Reader, _pc>;

    template <typename R>
    constexpr auto _make_reserve(R r) const
    {
        return lexyd::token(r);
    }
    template <typename Id, typename CharT, CharT... C>
    constexpr auto _make_reserve(_kw<Id, CharT, C...>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // We turn the keyword into a literal to be able to use a trie for matching.
        return _lit<CharT, C...>{};
    }

    //=== dsl ===//
    /// Adds a set of reserved identifiers.
    template <typename... R>
    constexpr auto reserve(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        return _id<Leading, Trailing, Reserved..., decltype(_make_reserve(r))...>{};
    }

    /// Reserves everything starting with the given rule.
    template <typename... R>
    constexpr auto reserve_prefix(R... prefix) const
    {
        return reserve((prefix + lexyd::any)...);
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    constexpr auto reserve_containing(R...) const
    {
        return reserve(_contains<R>{}...);
    }

    /// Matches every identifier, ignoring reserved ones.
    constexpr auto pattern() const
    {
        return _idp<Leading, Trailing>{};
    }

    /// Matches the initial char set of an identifier.
    constexpr auto leading_pattern() const
    {
        return Leading{};
    }

    /// Matches the trailing char set of an identifier.
    constexpr auto trailing_pattern() const
    {
        return Trailing{};
    }
};

/// Creates an identifier that consists of one or more of the given tokens.
template <typename Token>
constexpr auto identifier(Token)
{
    return _id<Token, Token>{};
}

/// Creates an identifier that consists of one leading token followed by zero or more trailing
/// tokens.
template <typename LeadingToken, typename TrailingToken>
constexpr auto identifier(LeadingToken, TrailingToken)
{
    return _id<LeadingToken, TrailingToken>{};
}
} // namespace lexyd

namespace lexy
{
template <typename Leading, typename Trailing>
constexpr auto token_kind_of<lexy::dsl::_idp<Leading, Trailing>> = lexy::identifier_token_kind;
}

//=== keyword ===//
namespace lexyd
{
template <typename Id, typename CharT, CharT... C>
struct _kw : token_base<_kw<Id, CharT, C...>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match the literal.
            if (!lexy::try_match_token(_lit<CharT, C...>{}, reader))
                return false;
            end = reader.position();

            // To qualify as a keyword, and not just the prefix of an identifier,
            // we must not have a trailing identifier character.
            return !lexy::try_match_token(Id{}.trailing_pattern(), reader);
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<
                typename Reader::encoding::char_type>;

            // Match the entire identifier.
            auto begin = reader.position();
            lexy::try_match_token(Id{}.pattern(), reader);
            auto end = reader.position();

            auto err = lexy::error<Reader, lexy::expected_keyword>(begin, end, str);
            context.on(_ev::error{}, err);
        }
    };
};

template <typename Id>
struct _keyword;
template <typename L, typename T, typename... R>
struct _keyword<_id<L, T, R...>>
{
    template <typename CharT, CharT... C>
    using get = _kw<_id<L, T>, CharT, C...>;
};

#if LEXY_HAS_NTTP
/// Matches the keyword.
template <lexy::_detail::string_literal Str, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return lexy::_detail::to_type_string<_keyword<_id<L, T>>::template get, Str>{};
}
#endif

#define LEXY_KEYWORD(Str, Id)                                                                      \
    LEXY_NTTP_STRING(::lexyd::_keyword<std::decay_t<decltype(Id)>>::template get, Str) {}
} // namespace lexyd

#endif // LEXY_DSL_IDENTIFIER_HPP_INCLUDED

