// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
/// The input ends before the closing delimiter was found.
struct missing_delimiter
{
    template <typename Input>
    class error
    {
    public:
        constexpr explicit error(typename Input::iterator pos) noexcept : _pos(pos) {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

    private:
        typename Input::iterator _pos;
    };
};
} // namespace lexy

namespace lexyd
{
template <typename Escape, typename CodePoint, typename Close>
struct _delim : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            auto sink = context.list_sink();

            const auto begin = input.cur();
            while (true)
            {
                if (input.peek() == Input::encoding::eof())
                    // We're missing the final delimiter.
                    return LEXY_MOV(context).error(input,
                                                   lexy::missing_delimiter::error<Input>(begin));
                else if (auto pos = input.cur(); Escape::escape_matcher::match(input))
                {
                    // We have an escape character.
                    if (!Escape::match_arg(sink, input))
                        // Invalid escape sequence.
                        return Escape::report_error(context, input, pos);
                }
                else if (Close::matcher::match(input))
                    // Done with the string.
                    break;
                else
                {
                    // Match a code point.
                    if (auto pos = input.cur(); CodePoint::match(input))
                    {
                        for (auto end = input.cur(); pos != end; ++pos)
                            sink(*pos);
                    }
                    else
                        return CodePoint::report_error(context, input, pos);
                }
            }

            // Add the final string as an argument.
            return NextParser::parse(context, input, LEXY_FWD(args)..., LEXY_MOV(sink).finish());
        }
    };
};

template <typename CodePoint, typename Close>
struct _delim<void, CodePoint, Close> : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            const auto begin = input.cur();

            auto pos = input.cur();
            while (true)
            {
                if (input.peek() == Input::encoding::eof())
                    // We're missing the final delimiter.
                    return LEXY_MOV(context).error(input,
                                                   lexy::missing_delimiter::error<Input>(begin));
                else if (Close::matcher::match(input))
                    // Done with the string.
                    break;
                else
                {
                    // Match a code point.
                    if (!CodePoint::match(input))
                        return CodePoint::report_error(context, input, pos);

                    pos = input.cur();
                }
            }

            // Add the lexeme as an argument.
            return NextParser::parse(context, input, LEXY_FWD(args)...,
                                     lexy::lexeme<Input>(begin, pos));
        }
    };
};

template <typename Open, typename Close, typename Whitespace>
struct _delim_dsl
{
    /// Specifies the atomic for a valid code point.
    template <typename CodePoint>
    LEXY_CONSTEVAL auto operator()(CodePoint) const
    {
        static_assert(lexy::is_atom<CodePoint>);
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{} >> _delim<void, CodePoint, Close>{};
        else
            return Whitespace{} + Open{} >> _delim<void, CodePoint, Close>{};
    }

    /// Specifies the atomic for a valid code point and an escape sequence.
    template <typename CodePoint, typename Escape>
    LEXY_CONSTEVAL auto operator()(CodePoint, Escape) const
    {
        static_assert(lexy::is_atom<CodePoint>);
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{} >> _delim<Escape, CodePoint, Close>{};
        else
            return Whitespace{} + Open{} >> _delim<Escape, CodePoint, Close>{};
    }

    /// Sets the whitespace pattern.
    template <typename Ws, typename Old = Whitespace,
              typename = std::enable_if_t<std::is_void_v<Old>>>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        static_assert(lexy::is_pattern<Ws>, "whitespace must be a pattern");
        return _delim_dsl<Open, Close, Ws>{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto delimited(Open, Close)
{
    static_assert(lexy::is_pattern<Open> && lexy::is_pattern<Close>);
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
    template <typename Input>
    class error
    {
    public:
        constexpr explicit error(typename Input::iterator pos) noexcept : _pos(pos) {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

    private:
        typename Input::iterator _pos;
    };
};
} // namespace lexy

namespace lexyd
{
template <typename Pattern, typename Replacement>
struct _escape_lit
{
    template <typename Sink, typename Input>
    LEXY_DSL_FUNC bool try_match(Sink& sink, Input& input)
    {
        if (!Pattern::matcher::match(input))
            return false;

        for (auto c : Replacement::get())
            sink(c);
        return true;
    }
};

template <typename EscapePattern, typename... EscapeArguments>
struct _escape
{
    using escape_matcher = typename EscapePattern::matcher;

    template <typename Sink, typename Input>
    LEXY_DSL_FUNC bool match_arg(Sink& sink, Input& input)
    {
        return (EscapeArguments::try_match(sink, input) || ...);
    }

    template <typename Context, typename Input>
    LEXY_DSL_FUNC auto report_error(Context& context, Input& input, typename Input::iterator pos) ->
        typename Context::result_type
    {
        return LEXY_MOV(context).error(input, lexy::invalid_escape_sequence::error<Input>(pos));
    }

    //=== dsl ===//
    /// Replace the escaped pattern with the literal string value.
    template <typename Pattern, typename Replacement>
    LEXY_CONSTEVAL auto literal(Pattern, Replacement) const
    {
        static_assert(lexy::is_pattern<Pattern>);
        return _escape<EscapePattern, EscapeArguments..., _escape_lit<Pattern, Replacement>>{};
    }
    /// Replace the escaped literal with itself.
    template <typename Replacement>
    LEXY_CONSTEVAL auto literal(Replacement rep) const
    {
        return literal(_lit<Replacement>{}, rep);
    }
};

template <typename EscapePattern>
LEXY_CONSTEVAL auto escape(EscapePattern)
{
    static_assert(lexy::is_pattern<EscapePattern>);
    return _escape<EscapePattern>{};
}

#if LEXY_HAS_NTTP
/// Defines the replacement string of a literal replacement.
template <lexy::_detail::string_literal Str>
constexpr auto escape_value = lexy::_detail::type_string<Str>;
#endif

#define LEXY_ESCAPE_VALUE(Str)                                                                     \
    LEXY_NTTP_STRING(Str) {}

constexpr auto backslash_escape = escape(LEXY_LIT("\\"));
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

