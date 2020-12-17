// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRACKETS_HPP_INCLUDED
#define LEXY_DSL_BRACKETS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/terminator.hpp>

namespace lexyd
{
template <typename Open, typename Close, typename Whitespace>
struct _brackets
{
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        return _brackets<Open, Close, Ws>{};
    }

    /// Matches the rule surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto operator()(R r) const
    {
        return open() >> terminator(close())(r);
    }

    /// Matches rule as often as possible, surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto while_(R r) const
    {
        return open() >> terminator(close()).while_(r);
    }
    /// Matches rule as often as possible but at least once, surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto while_one(R r) const
    {
        return open() >> r + terminator(close()).while_(r);
    }

    /// Matches `opt(r)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt(R r) const
    {
        return open() >> terminator(close()).opt(r);
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto list(R r) const
    {
        return open() >> terminator(close()).list(r);
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto list(R r, S sep) const
    {
        return open() >> terminator(close()).list(r, sep);
    }

    /// Matches `opt(list(r, sep))` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt_list(R r) const
    {
        return open() >> terminator(close()).opt_list(r);
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto opt_list(R r, S sep) const
    {
        return open() >> terminator(close()).opt_list(r, sep);
    }

    /// Matches the open bracket.
    LEXY_CONSTEVAL auto open() const
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{};
        else
            return whitespaced(Open{}, Whitespace{});
    }
    /// Matches the closing bracket.
    LEXY_CONSTEVAL auto close() const
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
    static_assert(lexy::is_branch<Open> && lexy::is_branch<Close>);
    return _brackets<Open, Close, void>{};
}

constexpr auto round_bracketed  = brackets(lit_c<'('>, lit_c<')'>);
constexpr auto square_bracketed = brackets(lit_c<'['>, lit_c<']'>);
constexpr auto curly_bracketed  = brackets(lit_c<'{'>, lit_c<'}'>);
constexpr auto angle_bracketed  = brackets(lit_c<'<'>, lit_c<'>'>);

constexpr auto parenthesized = round_bracketed;
} // namespace lexyd

#endif // LEXY_DSL_BRACKETS_HPP_INCLUDED

