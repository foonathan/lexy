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
    using condition_matcher = typename Condition::matcher;

    template <typename NextParser>
    using then_parser = typename Then::template parser<NextParser>;

    //=== rule ===//
    static constexpr auto has_matcher = Then::has_matcher;

    using _seq    = decltype(Condition{} + Then{});
    using matcher = typename _seq::matcher;
    template <typename NextParser>
    using parser = typename _seq::template parser<NextParser>;

    //=== dsl ===//
    friend LEXY_CONSTEVAL auto branch(_br)
    {
        return _br{};
    }
};

/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
LEXY_CONSTEVAL auto operator>>(Condition, Then)
{
    static_assert(lexy::is_pattern<Condition>, "branch condition must be a pattern");
    return _br<Condition, Then>{};
}

template <typename Pattern>
LEXY_CONSTEVAL auto branch(Pattern pattern)
{
    static_assert(lexy::is_pattern<Pattern>, "non-pattern rule requires a condition");
    return pattern >> success;
}
} // namespace lexyd

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED

