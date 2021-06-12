// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TERMINATOR_HPP_INCLUDED
#define LEXY_DSL_TERMINATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/recover.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexyd
{
template <typename Terminator, typename R, typename Recover>
struct _optt;
template <typename Terminator, typename R, typename Recover>
struct _whlt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _lstt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _olstt;

template <typename Terminator, typename... RecoveryLimit>
struct _term
{
    /// Adds the tokens to the recovery limit.
    template <typename... Tokens>
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));
        return _term<Terminator, RecoveryLimit..., Tokens...>{};
    }

    //=== rules ===//
    /// Matches rule followed by the terminator.
    template <typename Rule>
    constexpr auto operator()(Rule rule) const
    {
        return rule + terminator();
    }

    /// Matches rule followed by the terminator, recovering on error.
    template <typename Rule>
    constexpr auto try_(Rule rule) const
    {
        return lexyd::try_(rule + terminator(), recovery_rule());
    }

    /// Matches rule as long as terminator isn't matched.
    template <typename Rule>
    constexpr auto while_(Rule) const
    {
        return _whlt<Terminator, Rule, decltype(recovery_rule())>{};
    }
    /// Matches rule as long as terminator isn't matched, but at least once.
    template <typename Rule>
    constexpr auto while_one(Rule rule) const
    {
        if constexpr (lexy::is_branch<Rule>)
            return rule >> while_(rule);
        else
            return rule + while_(rule);
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt(R) const
    {
        return _optt<Terminator, R, decltype(recovery_rule())>{};
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto list(R) const
    {
        return _lstt<Terminator, R, void, decltype(recovery_rule())>{};
    }
    template <typename R, typename Sep>
    constexpr auto list(R, Sep) const
    {
        return _lstt<Terminator, R, Sep, decltype(recovery_rule())>{};
    }

    /// Matches `opt_list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt_list(R) const
    {
        return _olstt<Terminator, R, void, decltype(recovery_rule())>{};
    }
    template <typename R, typename S>
    constexpr auto opt_list(R, S) const
    {
        return _olstt<Terminator, R, S, decltype(recovery_rule())>{};
    }

    //=== access ===//
    /// Matches the terminator alone.
    constexpr auto terminator() const
    {
        return Terminator{};
    }

    /// Matches the recovery rule alone.
    constexpr auto recovery_rule() const
    {
        if constexpr (sizeof...(RecoveryLimit) == 0)
            return recover(terminator());
        else
            return recover(terminator()).limit(RecoveryLimit{}...);
    }

    //=== deprecated ===//
    /// Sets the whitespace that will be skipped before the terminator.
    template <typename Ws>
    constexpr auto operator[](Ws ws) const
    {
        auto branch = whitespaced(Terminator{}, ws);
        return _term<decltype(branch)>{};
    }
};

/// Creates a terminator using the given branch.
template <typename Branch>
constexpr auto terminator(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _term<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TERMINATOR_HPP_INCLUDED

