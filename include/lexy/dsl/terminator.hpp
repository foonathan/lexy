// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
template <typename Terminator, typename Rule>
struct _optt;
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
        static_assert((lexy::is_token_rule<Tokens> && ...));
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
    constexpr auto try_(Rule) const
    {
        return _tryt<Terminator, Rule, decltype(recovery_rule())>{};
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto opt(Rule) const
    {
        return _optt<Terminator, _tryt<Terminator, Rule, decltype(recovery_rule())>>{};
    }

    /// Matches `list(rule, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto list(Rule) const
    {
        return _lstt<Terminator, Rule, void, decltype(recovery_rule())>{};
    }
    template <typename Rule, typename Sep>
    constexpr auto list(Rule, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _lstt<Terminator, Rule, Sep, decltype(recovery_rule())>{};
    }

    /// Matches `opt_list(rule, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto opt_list(Rule) const
    {
        return _optt<Terminator, _lstt<Terminator, Rule, void, decltype(recovery_rule())>>{};
    }
    template <typename Rule, typename Sep>
    constexpr auto opt_list(Rule, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _optt<Terminator, _lstt<Terminator, Rule, Sep, decltype(recovery_rule())>>{};
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
};

/// Creates a terminator using the given branch.
template <typename Branch>
constexpr auto terminator(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _term<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TERMINATOR_HPP_INCLUDED

