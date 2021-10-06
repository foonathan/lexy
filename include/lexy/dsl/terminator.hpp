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
        if constexpr (lexy::is_branch_rule<Rule>)
            return rule >> terminator();
        else
            return rule + terminator();
    }

    /// Matches rule followed by the terminator, recovering on error.
    template <typename Rule>
    constexpr auto try_(Rule rule) const
    {
        if constexpr (lexy::is_branch_rule<Rule>)
            return lexyd::try_(rule >> terminator(), recovery_rule());
        else
            return lexyd::try_(rule + terminator(), recovery_rule());
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto opt(Rule rule) const
    {
        return _optt<Terminator, decltype(this->try_(rule))>{};
    }

    /// Matches `list(r, sep)` followed by terminator.
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

    /// Matches `opt_list(r, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt_list(R) const
    {
        return _optt<Terminator, _lstt<Terminator, R, void, decltype(recovery_rule())>>{};
    }
    template <typename R, typename S>
    constexpr auto opt_list(R, S) const
    {
        return _optt<Terminator, _lstt<Terminator, R, S, decltype(recovery_rule())>>{};
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

