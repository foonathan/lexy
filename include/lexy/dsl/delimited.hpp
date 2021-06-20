// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/symbol.hpp>
#include <lexy/dsl/value.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/lexeme.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_ESCAPE
#    define LEXY_DEPRECATED_ESCAPE
#else
#    define LEXY_DEPRECATED_ESCAPE [[deprecated("`.lit[_c]()` has been replaced by `.symbol()`")]]
#endif

namespace lexy
{
/// The reader ends before the closing delimiter was found.
struct missing_delimiter
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing delimiter";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Char, typename Context, typename Reader, typename Sink>
constexpr auto _del_parse_char(Context& context, Reader& reader, Sink& sink)
{
    using engine = typename Char::token_engine;
    if constexpr (lexy::engine_can_fail<engine, Reader>)
    {
        auto content_begin = reader.cur();
        if (auto ec = engine::match(reader); ec != typename engine::error_code())
        {
            Char::token_error(context, reader, ec, content_begin);
            if (!engine::recover(reader, ec))
                return false;
            else
                // We've recovered, repeat loop.
                return true;
        }
        auto content_end = reader.cur();

        context.token(Char::token_kind(), content_begin, content_end);
        sink(lexy::lexeme<Reader>(content_begin, content_end));
    }
    else
    {
        auto content_begin = reader.cur();
        engine::match(reader);
        auto content_end = reader.cur();

        context.token(Char::token_kind(), content_begin, content_end);
        sink(lexy::lexeme<Reader>(content_begin, content_end));
    }

    return true;
}

template <typename Close, typename Char, typename Limit, typename... Escapes>
struct _del : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink      = context.sink();
            auto del_begin = reader.cur();

            using close = lexy::rule_parser<Close, _list_finish<NextParser, Args...>>;
            while (true)
            {
                // Try to finish parsing the production.
                if (auto result = close::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had a closing delimiter, return that result.
                    return static_cast<bool>(result);
                }
                // Check for missing closing delimiter.
                else if (lexy::engine_peek<typename Limit::token_engine>(reader))
                {
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    context.error(err);
                    return false;
                }
                // Try to parse the escape sequences.
                else if (auto result = lexy::rule_try_parse_result::backtracked;
                         // This tries to parse each escape in order until one doesn't backtrack.
                         // Then enters the if.
                         ((result = lexy::rule_parser<Escapes, _list_sink>::try_parse(context,
                                                                                      reader, sink),
                           result != lexy::rule_try_parse_result::backtracked)
                          || ...))
                {
                    // If we just parsed an escape sequence, we just continue with the next
                    // character.
                    //
                    // If we had an invalid escape sequence, we also just continue as if
                    // nothing happened.
                    // The leading escape character will be skipped, as well as any valid prefixes.
                    // We could try and add them to the list, but it should be fine as-is.
                }
                // Parse the next character.
                else
                {
                    if (!_del_parse_char<Char>(context, reader, sink))
                        return false;
                }
            }

            return false; // unreachable
        }
    };
};

struct _escape_base
{};

template <typename Open, typename Close, typename Limit>
struct _delim_dsl
{
    /// Add tokens that will limit the delimited to detect a missing terminator.
    template <typename... Tokens>
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));
        return _delim_dsl<Open, Close, decltype((Limit{} / ... / Tokens{}))>{};
    }

    //=== rules ===//
    /// Sets the content.
    template <typename Char, typename... Escapes>
    constexpr auto operator()(Char, Escapes...) const
    {
        static_assert(lexy::is_token_rule<Char>);
        static_assert((std::is_base_of_v<_escape_base, Escapes> && ...));
        return no_whitespace(open() >> _del<Close, Char, Limit, Escapes...>{});
    }

    //=== access ===//
    /// Matches the open delimiter.
    constexpr auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    constexpr auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }

    //=== deprecated ===//
    /// Sets the whitespace.
    template <typename Ws>
    constexpr auto operator[](Ws ws) const
    {
        auto open = whitespaced(Open{}, ws);
        return _delim_dsl<decltype(open), Close, Limit>{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
constexpr auto delimited(Open, Close)
{
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
    return _delim_dsl<Open, Close, lexyd::_eof>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
constexpr auto delimited(Delim)
{
    static_assert(lexy::is_branch_rule<Delim>);
    return _delim_dsl<Delim, Delim, lexyd::_eof>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

namespace lexy
{
struct invalid_escape_sequence
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid escape sequence";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Escape, typename... Branches>
constexpr auto _escape_rule(Branches... branches)
{
    if constexpr (sizeof...(Branches) == 0)
        return Escape{};
    else
        return Escape{} >> (branches | ... | error<lexy::invalid_escape_sequence>);
}

template <typename Engine>
struct _escape_cap : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto begin = reader.cur();
            if (!lexy::engine_try_match<Engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::lexeme(reader, begin)));
        }
    };
};

struct _escape_char : token_base<_escape_char>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (reader.eof())
                return error_code::error;
            reader.bump();
            return error_code();
        }
    };

    // Don't need error, it won't be called.
};

template <typename Escape, typename... Branches>
struct _escape : decltype(_escape_rule<Escape>(Branches{}...)), _escape_base
{
    /// Adds a generic escape rule.
    template <typename Branch>
    constexpr auto rule(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the token.
    template <typename Token>
    constexpr auto capture(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return this->rule(_escape_cap<typename Token::token_engine>{});
    }

    /// Adds an escape rule that parses the symbol.
    template <const auto& Table, typename Rule>
    constexpr auto symbol(Rule rule) const
    {
        return this->rule(lexyd::symbol<Table>(rule));
    }
    /// Adds an escape rule that parses the symbol from the next code unit.
    template <const auto& Table>
    constexpr auto symbol() const
    {
        return this->symbol<Table>(_escape_char{});
    }

#if LEXY_HAS_NTTP
    /// Adds an escape rule that replaces the escaped string with the replacement.
    template <lexy::_detail::string_literal Str, typename Value>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit(Value value) const
    {
        return rule(lexyd::lit<Str> >> value);
    }
    /// Adds an escape rule that replaces the escaped string with itself.
    template <lexy::_detail::string_literal Str>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit() const
    {
        return lit<Str>(value_str<Str>);
    }
#endif

    /// Adds an escape rule that replaces the escaped character with the replacement.
    template <auto C, typename Value>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit_c(Value value) const
    {
        return rule(lexyd::lit_c<C> >> value);
    }
    /// Adds an escape rule that replaces the escape character with itself.
    template <auto C>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit_c() const
    {
        return lit_c<C>(value_c<C>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
constexpr auto escape(EscapeToken)
{
    static_assert(lexy::is_token_rule<EscapeToken>);
    return _escape<EscapeToken>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

