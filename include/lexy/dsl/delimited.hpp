// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/value.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/lexeme.hpp>

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
template <typename Close, typename Char, typename Escape>
struct _del : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            auto sink          = context.sink();
            auto del_begin     = reader.cur();
            auto content_begin = del_begin;
            auto content_end   = del_begin;

            using engine = typename Char::token_engine;
            lexy::branch_matcher<Close, Reader>  close{};
            lexy::branch_matcher<Escape, Reader> escape{};
            while (!close.match(reader))
            {
                if (reader.eof())
                {
                    // If we've reached EOF, it means we're missing the closing delimiter.
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    return LEXY_MOV(context).error(err);
                }
                else if (escape.match(reader))
                {
                    // Before we add the escape character to the sink, we need to add the content.
                    context.token(Char::token_kind(), content_begin, content_end);
                    sink(lexy::lexeme<Reader>(content_begin, content_end));

                    auto result = escape.template parse<_list_sink>(context, reader, sink);
                    if (result.has_error())
                        return result;

                    // We begin again after the escape sequence has been parsed.
                    content_begin = reader.cur();
                }
                // Parse the next character.
                else if constexpr (lexy::engine_can_fail<engine, Reader>)
                {
                    auto position = reader.cur();
                    if (auto ec = engine::match(reader); ec != typename engine::error_code())
                        return Char::token_error(context, reader, ec, position);
                }
                else
                {
                    engine::match(reader);
                }

                // Extend the content by what we've just parsed.
                content_end = reader.cur();
            }

            // Finish up the list.
            context.token(Char::token_kind(), content_begin, content_end);
            sink(lexy::lexeme<Reader>(content_begin, content_end));
            return _list_finish<NextParser, Args...>::parse_branch(close, context, reader,
                                                                   LEXY_FWD(args)..., sink);
        }
    };
};
template <typename Close, typename Char>
struct _del<Close, Char, void>
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            auto sink          = context.sink();
            auto del_begin     = reader.cur();
            auto content_begin = del_begin;
            auto content_end   = del_begin;

            using engine = typename Char::token_engine;
            lexy::branch_matcher<Close, Reader> close{};
            while (!close.match(reader))
            {
                if (reader.eof())
                {
                    // If we've reached EOF, it means we're missing the closing delimiter.
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    return LEXY_MOV(context).error(err);
                }
                else if constexpr (lexy::engine_can_fail<engine, Reader>)
                {
                    auto position = reader.cur();
                    if (auto ec = engine::match(reader); ec != typename engine::error_code())
                        return Char::token_error(context, reader, ec, position);
                }
                else
                {
                    engine::match(reader);
                }

                // Extend the content by what we've just parsed.
                content_end = reader.cur();
            }

            // Finish up the list.
            context.token(Char::token_kind(), content_begin, content_end);
            sink(lexy::lexeme<Reader>(content_begin, content_end));
            return _list_finish<NextParser, Args...>::parse_branch(close, context, reader,
                                                                   LEXY_FWD(args)..., sink);
        }
    };
};

template <typename Open, typename Close>
struct _delim_dsl
{
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws ws) const
    {
        auto open = whitespaced(Open{}, ws);
        return _delim_dsl<decltype(open), Close>{};
    }

    /// Sets the content.
    template <typename Char>
    LEXY_CONSTEVAL auto operator()(Char) const
    {
        static_assert(lexy::is_token<Char>);
        return no_whitespace(open() >> _del<Close, Char, void>{});
    }
    template <typename Char, typename Escape>
    LEXY_CONSTEVAL auto operator()(Char, Escape) const
    {
        static_assert(lexy::is_token<Char>);
        static_assert(lexy::is_branch<Escape>);
        return no_whitespace(open() >> _del<Close, Char, Escape>{});
    }

    /// Matches the open delimiter.
    LEXY_CONSTEVAL auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    LEXY_CONSTEVAL auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto delimited(Open, Close)
{
    static_assert(lexy::is_branch<Open> && lexy::is_branch<Close>);
    return _delim_dsl<Open, Close>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
LEXY_CONSTEVAL auto delimited(Delim)
{
    static_assert(lexy::is_branch<Delim>);
    return _delim_dsl<Delim, Delim>{};
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
LEXY_CONSTEVAL auto _escape_rule(Branches... branches)
{
    if constexpr (sizeof...(Branches) == 0)
        return Escape{};
    else
        return Escape{} >> (branches | ... | error<lexy::invalid_escape_sequence>);
}

template <typename Engine>
struct _escape_cap : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        typename Reader::iterator _begin{};

        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            _begin = reader.cur();
            return lexy::engine_try_match<Engine>(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                     lexy::lexeme(reader, _begin));
        }
    };
};

template <typename Escape, typename... Branches>
struct _escape : decltype(_escape_rule<Escape>(Branches{}...))
{
    /// Adds a generic escape rule.
    template <typename Branch>
    LEXY_CONSTEVAL auto rule(Branch) const
    {
        static_assert(lexy::is_branch<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the token.
    template <typename Token>
    LEXY_CONSTEVAL auto capture(Token) const
    {
        static_assert(lexy::is_token<Token>);
        return rule(_escape_cap<typename Token::token_engine>{});
    }

#if LEXY_HAS_NTTP
    /// Adds an escape rule that replaces the escaped string with the replacement.
    template <lexy::_detail::string_literal Str, typename Value>
    LEXY_CONSTEVAL auto lit(Value value) const
    {
        return rule(lexyd::lit<Str> >> value);
    }
    /// Adds an escape rule that replaces the escaped string with itself.
    template <lexy::_detail::string_literal Str>
    LEXY_CONSTEVAL auto lit() const
    {
        return lit<Str>(value_str<Str>);
    }
#endif

    /// Adds an escape rule that replaces the escaped character with the replacement.
    template <auto C, typename Value>
    LEXY_CONSTEVAL auto lit_c(Value value) const
    {
        return rule(lexyd::lit_c<C> >> value);
    }
    /// Adds an escape rule that replaces the escape character with itself.
    template <auto C>
    LEXY_CONSTEVAL auto lit_c() const
    {
        return lit_c<C>(value_c<C>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
LEXY_CONSTEVAL auto escape(EscapeToken)
{
    static_assert(lexy::is_token<EscapeToken>);
    return _escape<EscapeToken>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

