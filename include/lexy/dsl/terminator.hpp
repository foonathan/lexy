// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TERMINATOR_HPP_INCLUDED
#define LEXY_DSL_TERMINATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexyd
{
template <typename Terminator, typename R>
struct _optt;
template <typename Terminator, typename R>
struct _whlt;
template <typename Terminator, typename R, typename Sep>
struct _lstt;

template <typename Branch, typename Whitespace>
struct _term
{
    /// Sets the whitespace that will be skipped before the terminator.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        return _term<Branch, Ws>{};
    }

    /// Matches rule followed by the terminator.
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        return rule + terminator();
    }

    /// Matches rule as long as terminator isn't matched.
    template <typename Rule>
    LEXY_CONSTEVAL auto while_(Rule) const
    {
        return _whlt<decltype(terminator()), Rule>{};
    }
    /// Matches rule as long as terminator isn't matched, but at least once.
    template <typename Rule>
    LEXY_CONSTEVAL auto while_one(Rule rule) const
    {
        if constexpr (lexy::is_branch<Rule>)
            return rule >> while_(rule);
        else
            return rule + while_(rule);
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt(R r) const
    {
        return _optt<decltype(terminator()), decltype(r + terminator())>{};
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto list(R) const
    {
        return _lstt<decltype(terminator()), R, void>{};
    }
    template <typename R, typename Sep>
    LEXY_CONSTEVAL auto list(R, Sep) const
    {
        return _lstt<decltype(terminator()), R, Sep>{};
    }

    /// Matches `opt(list(r, sep))` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt_list(R r) const
    {
        return _optt<decltype(terminator()), decltype(list(r))>{};
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto opt_list(R r, S sep) const
    {
        return _optt<decltype(terminator()), decltype(list(r, sep))>{};
    }

    /// Matches the terminator alone.
    LEXY_CONSTEVAL auto terminator() const
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Branch{};
        else
            return whitespaced(Branch{}, Whitespace{});
    }
};

/// Creates a terminator using the given branch.
template <typename Branch>
LEXY_CONSTEVAL auto terminator(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _term<Branch, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TERMINATOR_HPP_INCLUDED

