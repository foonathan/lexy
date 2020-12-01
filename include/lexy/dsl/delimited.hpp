// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/not.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/value.hpp>
#include <lexy/dsl/whitespace.hpp>

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
struct _delb : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // Remember the beginning of the delimited for the error message.
            return NextParser::parse(handler, reader, reader.cur(), LEXY_FWD(args)...);
        }
    };
};

template <typename Content>
struct _delc : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (reader.eof())
            {
                // Find the beginning of the delimited; it was added somwhere in the arguments.
                typename Reader::iterator begin  = {};
                auto                      lambda = [&](const auto& arg) {
                    using arg_type = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<arg_type, typename Reader::iterator>)
                    {
                        begin = arg;
                        return true;
                    }
                    else
                        return false;
                };
                (lambda(args) || ...);

                // If we've reached EOF, it means we're missing the closing delimiter.
                auto e = lexy::make_error<Reader, lexy::missing_delimiter>(begin, reader.cur());
                return LEXY_MOV(handler).error(reader, e);
            }
            else
            {
                return Content::template parser<NextParser>::parse(handler, reader,
                                                                   LEXY_FWD(args)...);
            }
        }
    };
};

struct _dele : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, typename Reader::iterator,
                                 Args&&... args) -> typename Handler::result_type
        {
            // Remove the saved beginning again.
            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Open, typename Close, typename Whitespace>
struct _delim_dsl
{
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        return _delim_dsl<Open, Close, Ws>{};
    }

    template <typename Content>
    LEXY_CONSTEVAL auto _get(Content) const
    {
        auto c = branch(close());

        // We put the content in a list until the closing condition of the list matches.
        auto l = list(!c.condition() >> _delc<Content>{});
        // We surround the list with the logic that handles the positional stuff.
        return open() >> _delb{} + opt(l) + _dele{} + c.then();
    }

    /// Sets the content.
    template <typename Content>
    LEXY_CONSTEVAL auto operator()(Content content) const
    {
        if constexpr (lexy::is_pattern<Content>)
            return _get(capture(content));
        else
            return _get(content);
    }
    template <typename Content, typename Escape>
    LEXY_CONSTEVAL auto operator()(Content content, Escape escape) const
    {
        if constexpr (lexy::is_pattern<Content>)
            return _get(escape | else_ >> capture(content));
        else
            return _get(escape | else_ >> content);
    }

    /// Matches the open delimiter.
    LEXY_CONSTEVAL auto open() const
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{};
        else
            return whitespaced(Open{}, Whitespace{});
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
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
    return _delim_dsl<Open, Close, void>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
LEXY_CONSTEVAL auto delimited(Delim)
{
    static_assert(lexy::is_pattern<Delim>);
    return _delim_dsl<Delim, Delim, void>{};
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
template <typename Pattern, typename... Branches>
LEXY_CONSTEVAL auto _escape_rule(Branches... branches)
{
    if constexpr (sizeof...(Branches) == 0)
        return Pattern{};
    else if constexpr ((decltype(branch(branches))::is_unconditional || ...))
        return Pattern{} >> (branches | ...);
    else
        return Pattern{} >> (branches | ... | (else_ >> error<lexy::invalid_escape_sequence>));
}

template <typename Pattern>
struct _escape_cap
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // We can be sure that the pattern matches here.
            auto begin  = reader.cur();
            auto result = Pattern::matcher::match(reader);
            LEXY_PRECONDITION(result);
            return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                     lexy::lexeme(reader, begin));
        }
    };
};

template <typename EscapePattern, typename... Branches>
struct _escape : decltype(_escape_rule<EscapePattern>(Branches{}...))
{
    /// Adds a generic escape rule.
    template <typename Branch>
    LEXY_CONSTEVAL auto rule(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return _escape<EscapePattern, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the pattern.
    template <typename Pattern>
    LEXY_CONSTEVAL auto capture(Pattern pattern) const
    {
        static_assert(lexy::is_pattern<Pattern>);
        return rule(peek(pattern) >> _escape_cap<Pattern>{});
    }

#if LEXY_HAS_NTTP
    /// Adds an escape rule that replaces the escaped string with the replacement.
    template <lexy::_detail::string_literal Str, typename Value>
    LEXY_CONSTEVAL auto lit(Value value)
    {
        return rule(lexyd::lit<Str> >> value);
    }
    /// Adds an escape rule that replaces the escaped string with itself.
    template <lexy::_detail::string_literal Str>
    LEXY_CONSTEVAL auto lit()
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
/// The pattern is the initial pattern to begin,
/// and then you can add rules that match after it.
template <typename EscapePattern>
LEXY_CONSTEVAL auto escape(EscapePattern)
{
    static_assert(lexy::is_pattern<EscapePattern>);
    return _escape<EscapePattern>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

