// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
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

    static constexpr bool is_branch = true;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Condition, Reader> _condition;

        static constexpr auto is_unconditional = decltype(_condition)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            return _condition.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            using continuation = typename _seq_parser<NextParser, R...>::type;
            return _condition.template parse<continuation>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    using parser = typename _seq_parser<NextParser, Condition, R...>::type;
};

//=== operator>> ===//
/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
LEXY_CONSTEVAL auto operator>>(Condition, Then)
{
    static_assert(lexy::is_branch<Condition>, "condition must be a branch");
    return _br<Condition, Then>{};
}
template <typename Condition, typename... R>
LEXY_CONSTEVAL auto operator>>(Condition, _seq<R...>)
{
    static_assert(lexy::is_branch<Condition>, "condition must be a branch");
    return _br<Condition, R...>{};
}
template <typename Condition, typename C, typename... R>
LEXY_CONSTEVAL auto operator>>(Condition, _br<C, R...>)
{
    static_assert(lexy::is_branch<Condition>, "condition must be a branch");
    return _br<Condition, C, R...>{};
}

// Prevent nested branches in `_br`'s condition.
template <typename C, typename... R, typename Then>
LEXY_CONSTEVAL auto operator>>(_br<C, R...>, Then)
{
    return C{} >> _seq<R..., Then>{};
}
template <typename C, typename... R, typename... S>
LEXY_CONSTEVAL auto operator>>(_br<C, R...>, _seq<S...>)
{
    return C{} >> _seq<R..., S...>{};
}

// Disambiguation.
template <typename C1, typename... R, typename C2, typename... S>
LEXY_CONSTEVAL auto operator>>(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}

//=== operator+ ===//
// If we add something on the left to a branch, we loose the branchy-ness.
template <typename Rule, typename Condition, typename... R>
LEXY_CONSTEVAL auto operator+(Rule rule, _br<Condition, R...>)
{
    return rule + _seq<Condition, R...>{};
}
// Disambiguation.
template <typename... R, typename Condition, typename... S>
LEXY_CONSTEVAL auto operator+(_seq<R...>, _br<Condition, S...>)
{
    return _seq<R...>{} + _seq<Condition, S...>{};
}

// If we add something on the right to a branch, we extend the then.
template <typename Condition, typename... R, typename Rule>
LEXY_CONSTEVAL auto operator+(_br<Condition, R...>, Rule)
{
    return _br<Condition, R..., Rule>{};
}
// Disambiguation.
template <typename Condition, typename... R, typename... S>
LEXY_CONSTEVAL auto operator+(_br<Condition, R...>, _seq<S...>)
{
    return _br<Condition, R..., S...>{};
}

// If we add two branches, we use the condition of the first one and treat the second as sequence.
template <typename C1, typename... R, typename C2, typename... S>
LEXY_CONSTEVAL auto operator+(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}
} // namespace lexyd

namespace lexyd
{
struct _else : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = true;

        constexpr bool match(Reader&)
        {
            return true;
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Takes the branch unconditionally.
inline constexpr auto else_ = _else{};
} // namespace lexyd

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED

