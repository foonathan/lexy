// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRANCH_HPP_INCLUDED
#define LEXY_DSL_BRANCH_HPP_INCLUDED

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

    using then = Then;
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

    // If we add something on the left to a branch, we loose our branchy-ness.
    template <typename Rule>
    friend LEXY_CONSTEVAL auto operator+(Rule rule, _br)
    {
        return rule + Condition{} + Then{};
    }
    // If we add something on the right to a branch, we extend our then.
    template <typename Rule>
    friend LEXY_CONSTEVAL auto operator+(_br, Rule rule)
    {
        return _br<Condition, decltype(Then{} + rule)>{};
    }

    // A condition on the left extends our condition.
    template <typename Pattern>
    friend LEXY_CONSTEVAL auto operator>>(Pattern pattern, _br)
    {
        static_assert(lexy::is_pattern<Pattern>, "branch condition must be a pattern");
        return _br<decltype(pattern + Condition{}), Then>{};
    }
    // A rule on the right extends our then.
    template <typename Rule>
    friend LEXY_CONSTEVAL auto operator>>(_br, Rule rule)
    {
        static_assert(lexy::is_pattern<Then>,
                      "branch cannot be used as condition of another branch as it isn't a pattern");
        return _br{} + rule;
    }
};

/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
LEXY_CONSTEVAL auto operator>>(Condition, Then)
{
    static_assert(lexy::is_pattern<Condition>, "branch condition must be a pattern");
    return _br<Condition, Then>{};
}

/// Turns a pattern into a branch.
template <typename Pattern>
LEXY_CONSTEVAL auto branch(Pattern pattern)
{
    static_assert(lexy::is_pattern<Pattern>, "non-pattern rule requires a condition");
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
constexpr auto is_branch_rule = is_pattern<Rule>;
template <typename Condition, typename Then>
constexpr auto is_branch_rule<lexyd::_br<Condition, Then>> = true;
} // namespace lexy

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED

