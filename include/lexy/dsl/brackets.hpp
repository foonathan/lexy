// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRACKETS_HPP_INCLUDED
#define LEXY_DSL_BRACKETS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/terminator.hpp>

namespace lexyd
{
template <typename Open, typename Close, typename... RecoveryLimit>
struct _brackets
{
    /// Adds the tokens to the recovery limit.
    template <typename... Tokens>
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));
        return _brackets<Open, Close, RecoveryLimit..., Tokens...>{};
    }

    //=== rules ===//
    /// Matches the rule surrounded by brackets.
    template <typename R>
    constexpr auto operator()(R r) const
    {
        return open() >> as_terminator()(r);
    }

    /// Matches the rule surrounded by brackets, recovering on error.
    template <typename R>
    constexpr auto try_(R r) const
    {
        return open() >> as_terminator().try_(r);
    }

    /// Matches `opt(r)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt(R r) const
    {
        return open() >> as_terminator().opt(r);
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto list(R r) const
    {
        return open() >> as_terminator().list(r);
    }
    template <typename R, typename S>
    constexpr auto list(R r, S sep) const
    {
        return open() >> as_terminator().list(r, sep);
    }

    /// Matches `opt_list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt_list(R r) const
    {
        return open() >> as_terminator().opt_list(r);
    }
    template <typename R, typename S>
    constexpr auto opt_list(R r, S sep) const
    {
        return open() >> as_terminator().opt_list(r, sep);
    }

    //=== access ===//
    /// Matches the open bracket.
    constexpr auto open() const
    {
        return Open{};
    }
    /// Matches the closing bracket.
    constexpr auto close() const
    {
        return Close{};
    }

    /// Returns an equivalent terminator.
    constexpr auto as_terminator() const
    {
        return _term<Close, RecoveryLimit...>{};
    }

    constexpr auto recovery_rule() const
    {
        return as_terminator().recovery_rule();
    }

    //=== deprecated ===//
    /// Sets the whitespace.
    template <typename Ws>
    constexpr auto operator[](Ws ws) const
    {
        auto open  = whitespaced(Open{}, ws);
        auto close = whitespaced(Close{}, ws);
        return _brackets<decltype(open), decltype(close)>{};
    }
};

/// Defines open and close brackets.
template <typename Open, typename Close>
constexpr auto brackets(Open, Close)
{
    static_assert(lexy::is_branch<Open> && lexy::is_branch<Close>);
    return _brackets<Open, Close>{};
}

constexpr auto round_bracketed  = brackets(lit_c<'('>, lit_c<')'>);
constexpr auto square_bracketed = brackets(lit_c<'['>, lit_c<']'>);
constexpr auto curly_bracketed  = brackets(lit_c<'{'>, lit_c<'}'>);
constexpr auto angle_bracketed  = brackets(lit_c<'<'>, lit_c<'>'>);

constexpr auto parenthesized = round_bracketed;
} // namespace lexyd

#endif // LEXY_DSL_BRACKETS_HPP_INCLUDED

