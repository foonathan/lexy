// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRANCH_HPP_INCLUDED
#define LEXY_DSL_BRANCH_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
template <typename Condition, typename... R>
struct _br : rule_base
{
    static_assert(sizeof...(R) >= 0);

    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = Condition::is_unconditional_branch;

    // We simple connect Condition with R... and then NextParser.
    // Condition has a try_parse() that will try to match Condition and then continue on with the
    // continuation.
    template <typename NextParser>
    using parser = lexy::rule_parser<Condition, lexy::rule_parser<_seq_impl<R...>, NextParser>>;
};

//=== operator>> ===//
/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
constexpr auto operator>>(Condition, Then)
{
    static_assert(lexy::is_branch_rule<Condition>, "condition must be a branch");
    return _br<Condition, Then>{};
}
template <typename Condition, typename... R>
constexpr auto operator>>(Condition, _seq<R...>)
{
    static_assert(lexy::is_branch_rule<Condition>, "condition must be a branch");
    return _br<Condition, R...>{};
}
template <typename Condition, typename C, typename... R>
constexpr auto operator>>(Condition, _br<C, R...>)
{
    static_assert(lexy::is_branch_rule<Condition>, "condition must be a branch");
    return _br<Condition, C, R...>{};
}

// Prevent nested branches in `_br`'s condition.
template <typename C, typename... R, typename Then>
constexpr auto operator>>(_br<C, R...>, Then)
{
    return C{} >> _seq<R..., Then>{};
}
template <typename C, typename... R, typename... S>
constexpr auto operator>>(_br<C, R...>, _seq<S...>)
{
    return C{} >> _seq<R..., S...>{};
}

// Disambiguation.
template <typename C1, typename... R, typename C2, typename... S>
constexpr auto operator>>(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}

//=== operator+ ===//
// If we add something on the left to a branch, we loose the branchy-ness.
template <typename Rule, typename Condition, typename... R>
constexpr auto operator+(Rule rule, _br<Condition, R...>)
{
    return rule + _seq<Condition, R...>{};
}
// Disambiguation.
template <typename... R, typename Condition, typename... S>
constexpr auto operator+(_seq<R...>, _br<Condition, S...>)
{
    return _seq<R...>{} + _seq<Condition, S...>{};
}

// If we add something on the right to a branch, we extend the then.
template <typename Condition, typename... R, typename Rule>
constexpr auto operator+(_br<Condition, R...>, Rule)
{
    return _br<Condition, R..., Rule>{};
}
// Disambiguation.
template <typename Condition, typename... R, typename... S>
constexpr auto operator+(_br<Condition, R...>, _seq<S...>)
{
    return _br<Condition, R..., S...>{};
}

// If we add two branches, we use the condition of the first one and treat the second as sequence.
template <typename C1, typename... R, typename C2, typename... S>
constexpr auto operator+(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}
} // namespace lexyd

namespace lexyd
{
struct _else : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    using parser = NextParser;
};
struct _else_dsl
{
    template <typename R>
    friend constexpr auto operator>>(_else_dsl, R rule)
    {
        return _else{} >> rule;
    }
    template <typename... R>
    friend constexpr auto operator>>(_else_dsl, _seq<R...> rule)
    {
        return _else{} >> rule;
    }
    template <typename C, typename... R>
    friend constexpr auto operator>>(_else_dsl, _br<C, R...> rule)
    {
        return _else{} >> rule;
    }
};

/// Takes the branch unconditionally.
inline constexpr auto else_ = _else_dsl{};
} // namespace lexyd

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED

