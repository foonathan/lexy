// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRANCH_HPP_INCLUDED
#define LEXY_DSL_BRANCH_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/success.hpp>

namespace lexyd
{
template <typename Condition, typename Then>
struct _br : rule_base
{
    using condition                        = Condition;
    static constexpr auto is_unconditional = std::is_same_v<const Condition, decltype(success)>;
    using condition_matcher                = typename Condition::matcher;

    using then                     = Then;
    static constexpr auto has_then = !std::is_same_v<const Then, decltype(success)>;
    template <typename NextParser>
    using then_parser = typename Then::template parser<NextParser>;

    //=== rule ===//
    static constexpr auto has_matcher = Then::has_matcher;

    using _seq    = decltype(Condition{} + Then{});
    using matcher = typename _seq::matcher;
    template <typename NextParser>
    using parser = typename _seq::template parser<NextParser>;

    //=== dsl ===//
    // A branch is already a branch.
    friend LEXY_CONSTEVAL auto branch(_br)
    {
        return _br{};
    }
};

// If we add something on the left to a branch, we loose the branchy-ness.
template <typename Rule, typename Condition, typename Then>
LEXY_CONSTEVAL auto operator+(Rule rule, _br<Condition, Then>)
{
    return rule + Condition{} + Then{};
}
// If we add something on the right to a branch, we extend the then.
template <typename Condition, typename Then, typename Rule>
LEXY_CONSTEVAL auto operator+(_br<Condition, Then>, Rule rule)
{
    return _br<Condition, decltype(Then{} + rule)>{};
}
// If we add two branches, we extend the then of the first one.
template <typename C1, typename T1, typename C2, typename T2>
LEXY_CONSTEVAL auto operator+(_br<C1, T1>, _br<C2, T2>)
{
    return _br<C1, decltype(T1{} + C2{} + T2{})>{};
}

// A condition on the left extends the condition.
template <typename Pattern, typename Condition, typename Then>
LEXY_CONSTEVAL auto operator>>(Pattern pattern, _br<Condition, Then>)
{
    static_assert(lexy::is_pattern<Pattern>, "branch condition must be a pattern");
    return _br<decltype(pattern + Condition{}), Then>{};
}

/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
LEXY_CONSTEVAL auto operator>>(Condition, Then)
{
    static_assert(lexy::is_pattern<Condition>, "branch condition must be a pattern");
    return _br<Condition, Then>{};
}

/// Turns a pattern into a branch.
template <typename Pattern, typename = std::enable_if_t<lexy::is_pattern<Pattern>>>
LEXY_CONSTEVAL auto branch(Pattern pattern)
{
    return pattern >> success;
}
} // namespace lexyd

namespace lexyd
{
struct _else
{
    template <typename Then>
    friend LEXY_CONSTEVAL auto operator>>(_else, Then then)
    {
        return success >> then;
    }
};

/// Takes the branch unconditionally.
inline constexpr auto else_ = _else{};
} // namespace lexyd

namespace lexy
{
template <typename Rule>
using _detect_branch = decltype(branch(Rule{}));

template <typename Rule>
constexpr auto is_branch_rule = _detail::is_detected<_detect_branch, Rule>;
} // namespace lexy

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED

