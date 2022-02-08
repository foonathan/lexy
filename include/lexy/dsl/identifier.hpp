// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_IDENTIFIER_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/char_class.hpp>
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
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match Leading character.
            if (!lexy::try_match_token(Leading{}, reader))
                return false;

            // Match zero or more trailing characters.
            while (lexy::try_match_token(Trailing{}, reader))
            {}

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            Leading::template char_class_report_error<Reader>(context, reader.position());
        }
    };
};

template <typename Token>
struct _prefix : token_base<_prefix<Token>>
{
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Token, Reader> token;
        typename Reader::iterator             end;

        constexpr explicit tp(const Reader& reader) : token(reader), end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the token.
            if (!token.try_parse(reader))
                return false;
            reader.set_position(token.end);

            // Consume the rest of the input.
            lexy::try_match_token(lexy::dsl::any, reader);
            end = reader.position();

            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            // Only the token part can fail.
            token.report_error(context, reader);
        }
    };
};

template <typename Token>
struct _contains : token_base<_contains<Token>>
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
                if (lexy::try_match_token(Token{}, reader))
                {
                    // We've found it.
                    break;
                }
                else if (reader.peek() == Reader::encoding::eof())
                {
                    // Haven't found it.
                    end = reader.position();
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            // Consume everything else.
            lexy::try_match_token(lexy::dsl::any, reader);

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            // Trigger an error by parsing the token at the end of the input.
            reader.set_position(end);

            LEXY_ASSERT(reader.peek() == Reader::encoding::eof(),
                        "forgot to set end in try_parse()");

            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            LEXY_ASSERT(!result, "token shouldn't have matched?!");
            parser.report_error(context, reader);
        }
    };
};

template <typename Id, typename CharT, CharT... C>
struct _kw;

template <typename Leading, typename Trailing, typename... Reserved>
struct _id : branch_base
{
    template <typename Reader>
    constexpr static auto _is_reserved(const Reader& reader, typename Reader::iterator begin,
                                       typename Reader::iterator end)
    {
        if constexpr (sizeof...(Reserved) == 0)
        {
            (void)reader;
            (void)begin;
            (void)end;

            // No reserved patterns, never reserved.
            return std::false_type{};
        }
        else
        {
            auto id_reader = lexy::partial_reader(reader, begin, end);
            // Need to match any of the reserved tokens.
            return lexy::try_match_token((Reserved{} / ...), id_reader)
                   // And fully match it.
                   && id_reader.position() == end;
        }
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the pattern; this does not consume whitespace, so the range is accurate.
            auto begin = reader.position();
            if (!pattern().token_parse(context, reader))
                return false;
            auto end = reader.position();

            // Check for a reserved identifier.
            if (_is_reserved(reader, begin, end))
            {
                // It is reserved, report an error but trivially recover.
                auto err = lexy::error<Reader, lexy::reserved_identifier>(begin, end);
                context.on(_ev::error{}, err);
            }

            // Skip whitespace and continue with the value.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };

    template <typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock*, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<decltype(pattern()), Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // We only succeed if it's not a reserved identifier.
            return !_is_reserved(reader, reader.position(), end);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end);
            reader.set_position(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };

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
    constexpr auto reserve_prefix(R... r) const
    {
        return reserve(_prefix<decltype(_make_reserve(r))>{}...);
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    constexpr auto reserve_containing(R... r) const
    {
        return reserve(_contains<decltype(_make_reserve(r))>{}...);
    }

    /// Matches every identifier, ignoring reserved ones.
    static constexpr auto pattern()
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

/// Creates an identifier that consists of one or more of the given characters.
template <typename CharClass>
constexpr auto identifier(CharClass)
{
    static_assert(lexy::is_char_class_rule<CharClass>);
    return _id<CharClass, CharClass>{};
}

/// Creates an identifier that consists of one leading token followed by zero or more trailing
/// tokens.
template <typename LeadingClass, typename TrailingClass>
constexpr auto identifier(LeadingClass, TrailingClass)
{
    static_assert(
        lexy::is_char_class_rule<LeadingClass> && lexy::is_char_class_rule<TrailingClass>);
    return _id<LeadingClass, TrailingClass>{};
}
} // namespace lexyd

namespace lexy
{
template <typename Leading, typename Trailing>
constexpr auto token_kind_of<lexy::dsl::_idp<Leading, Trailing>> = lexy::identifier_token_kind;
} // namespace lexy

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

            auto err = lexy::error<Reader, lexy::expected_keyword>(begin, end, str, sizeof...(C));
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
template <lexy::_detail::string_literal Str, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return lexy::_detail::to_type_string<_keyword<_id<L, T>>::template get, Str>{};
}
#else
template <auto C, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return _kw<_id<L, T>, LEXY_DECAY_DECLTYPE(C), C>{};
}
#endif

#define LEXY_KEYWORD(Str, Id)                                                                      \
    LEXY_NTTP_STRING(::lexyd::_keyword<LEXY_DECAY_DECLTYPE(Id)>::template get, Str) {}
} // namespace lexyd

namespace lexy
{
template <typename Id, typename CharT, CharT... C>
constexpr auto token_kind_of<lexy::dsl::_kw<Id, CharT, C...>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_IDENTIFIER_HPP_INCLUDED

