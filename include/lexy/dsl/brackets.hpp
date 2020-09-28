// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRACKETS_HPP_INCLUDED
#define LEXY_DSL_BRACKETS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/condition.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexyd
{
template <typename Open, typename Close, typename Whitespace>
struct _brackets
{
    /// Sets the whitespace pattern.
    template <typename Ws, typename Old = Whitespace,
              typename = std::enable_if_t<std::is_void_v<Old>>>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        static_assert(lexy::is_pattern<Ws>, "whitespace must be a pattern");
        return _brackets<Open, Close, Ws>{};
    }

    /// Matches the rule surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto operator()(R r) const
    {
        return open(*this) >> r + close(*this);
    }

    /// Matches `opt(r)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt(R r) const
    {
        auto o = open(*this);
        auto c = close(*this);
        // We always match an open bracket.
        // If in the condition of the option, we have matched the closing bracket, we have an empty
        // option. Otherwise, if we didn't, we match the rule and then the closing bracket.
        return o >> lexyd::opt(!c >> r + c);
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto list(R r) const
    {
        auto o = open(*this);
        auto c = close(*this);
        // We parse the list item while we're haven't matched the closing bracket.
        // When we're done with the list, we will have consumed the closing bracket.
        return o >> lexyd::list(!c >> r);
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto list(R r, S sep) const
    {
        auto o = open(*this);
        auto c = close(*this);
        // When we have a separator, it can influence whether or not we're trying to have another
        // item. As such, we can't use !c - we can finish parsing the list without having matched
        // the closing bracket. We use unless(c) and then parse c at the end.
        return o >> lexyd::list(unless(c) >> r, sep) + c;
    }

    /// Matches `opt(list(r, sep))` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt_list(R r) const
    {
        auto o = open(*this);
        auto c = close(*this);
        // See above.
        return o >> lexyd::opt(lexyd::list(!c >> r));
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto opt_list(R r, S sep) const
    {
        auto o = open(*this);
        auto c = close(*this);
        // See above.
        return o >> lexyd::opt(lexyd::list(unless(c) >> r, sep)) + c;
    }

    /// Matches the open bracket.
    friend LEXY_CONSTEVAL auto open(_brackets)
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{};
        else
            return whitespaced(Open{}, Whitespace{});
    }
    /// Matches the closing bracket.
    friend LEXY_CONSTEVAL auto close(_brackets)
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Close{};
        else
            return whitespaced(Close{}, Whitespace{});
    }
};

/// Defines open and close brackets.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto brackets(Open, Close)
{
    static_assert(lexy::is_pattern<Open> && lexy::is_pattern<Close>);
    return _brackets<Open, Close, void>{};
}

constexpr auto round_bracketed  = brackets(lit_c<'('>, lit_c<')'>);
constexpr auto square_bracketed = brackets(lit_c<'['>, lit_c<']'>);
constexpr auto curly_bracketed  = brackets(lit_c<'{'>, lit_c<'}'>);
constexpr auto angle_bracketed  = brackets(lit_c<'<'>, lit_c<'>'>);

constexpr auto parenthesized = round_bracketed;
} // namespace lexyd

#endif // LEXY_DSL_BRACKETS_HPP_INCLUDED
