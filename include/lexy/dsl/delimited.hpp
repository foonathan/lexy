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
template <typename Close>
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
                input.bump();
                ++end;
            }

            // Add the lexeme as an argument.
            return NextParser::parse(context, input, LEXY_FWD(args)...,
                                     lexy::lexeme<Input>(begin, end));
        }
    };
};

template <typename Base>
struct _delim : Base
{
    /// Sets the whitespace pattern.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws ws) const
    {
        static_assert(lexy::is_pattern<Ws>, "whitespace must be a pattern");
        // We add the whitespace as another condition.
        // This will extend the original condition.
        return ws >> Base{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto delimited(Open open, Close)
{
    static_assert(lexy::is_pattern<Open> && lexy::is_pattern<Close>);
    auto base = open >> _delim_end<Close>{};
    return _delim<decltype(base)>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
LEXY_CONSTEVAL auto delimited(Delim delim)
{
    static_assert(lexy::is_pattern<Delim>);
    auto base = delim >> _delim_end<Delim>{};
    return _delim<decltype(base)>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

