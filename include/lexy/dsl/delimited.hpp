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
template <typename CodePoint, typename Close>
struct _delim_end : rule_base
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

            auto end = input.cur();
            while (!Close::matcher::match(input))
            {
                if (input.peek() == Input::encoding::eof())
                    // We're missing the final delimiter.
                    return LEXY_MOV(context).error(input,
                                                   lexy::missing_delimiter::error<Input>(begin));

                // We didn't match the closing delimiter.
                // Advance to the next code point and increase content length.
                if (!CodePoint::match(input))
                    // We know that end was the previous position.
                    return CodePoint::report_error(context, input, end);

                end = input.cur();
            }

            // Add the lexeme as an argument.
            return NextParser::parse(context, input, LEXY_FWD(args)...,
                                     lexy::lexeme<Input>(begin, end));
        }
    };
};

template <typename Open, typename Close, typename Whitespace>
struct _delim
{
    /// Specifies the atomic for a valid code point.
    template <typename CodePoint>
    LEXY_CONSTEVAL auto operator()(CodePoint) const
    {
        static_assert(lexy::is_atom<CodePoint>);
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{} >> _delim_end<CodePoint, Close>{};
        else
            return Whitespace{} + Open{} >> _delim_end<CodePoint, Close>{};
    }

    /// Sets the whitespace pattern.
    template <typename Ws, typename Old = Whitespace,
              typename = std::enable_if_t<std::is_void_v<Old>>>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        static_assert(lexy::is_pattern<Ws>, "whitespace must be a pattern");
        return _delim<Open, Close, Ws>{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto delimited(Open, Close)
{
    static_assert(lexy::is_pattern<Open> && lexy::is_pattern<Close>);
    return _delim<Open, Close, void>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
LEXY_CONSTEVAL auto delimited(Delim)
{
    static_assert(lexy::is_pattern<Delim>);
    return _delim<Delim, Delim, void>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

