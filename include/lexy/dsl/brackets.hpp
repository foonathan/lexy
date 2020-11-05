// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRACKETS_HPP_INCLUDED
#define LEXY_DSL_BRACKETS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/not.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexyd
{
template <typename Open, typename Close, typename Whitespace>
struct _brackets
{
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws) const
    {
        return _brackets<Open, Close, Ws>{};
    }

    /// Matches the rule surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto operator()(R r) const
    {
        auto o = open();
        auto c = close();
        return o >> r + c;
    }

    /// Matches `opt(r)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt(R r) const
    {
        auto o = open();
        auto c = close();

        if constexpr (lexy::is_pattern<decltype(c)>)
            return o >> lexyd::opt(!c >> r + c);
        else
            return o >> (!c.condition() >> r + c | else_ >> nullopt + c.then());
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto list(R r) const
    {
        auto o = open();
        auto c = branch(close());

        // !c.condition() matches until we've consumed the branch condition.
        // Then the list exits and we still need c.then().
        return o >> lexyd::list(!c.condition() >> r) + c.then();
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto list(R r, S sep) const
    {
        auto o = open();
        auto c = branch(close());

        // We can't use ! alone, as we might decide the list ends based on separator alone.
        // As such we reset after the ! anyway and parse the entire condition again.
        return o >> lexyd::list(peek(!c.condition()) >> r, sep) + c;
    }

    /// Matches `opt(list(r, sep))` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt_list(R r) const
    {
        auto o = open();
        auto c = branch(close());

        // Same as above, we're just allowing the list to exit before doing one item.
        // As !c.condition() is also the condition for taking the optional, we still need c.then().
        return o >> lexyd::opt(lexyd::list(!c.condition() >> r)) + c.then();
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto opt_list(R r, S sep) const
    {
        auto o = open();
        auto c = branch(close());

        // As above, we can't use !c and reuse it as the condition for opt().
        return o >> lexyd::opt(lexyd::list(peek(!c.condition()) >> r, sep)) + c;
    }

    /// Matches the open bracket.
    LEXY_CONSTEVAL auto open() const
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Open{};
        else
            return whitespaced(Open{}, Whitespace{});
    }
    /// Matches the closing bracket.
    LEXY_CONSTEVAL auto close() const
    {
        if constexpr (std::is_same_v<Whitespace, void>)
            return Close{};
        else
            return whitespaced(Close{}, Whitespace{});
    }
};

/// Defines open and close brackets.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto brackets(Open, Close)
{
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
    return _brackets<Open, Close, void>{};
}

constexpr auto round_bracketed  = brackets(lit_c<'('>, lit_c<')'>);
constexpr auto square_bracketed = brackets(lit_c<'['>, lit_c<']'>);
constexpr auto curly_bracketed  = brackets(lit_c<'{'>, lit_c<'}'>);
constexpr auto angle_bracketed  = brackets(lit_c<'<'>, lit_c<'>'>);

constexpr auto parenthesized = round_bracketed;
} // namespace lexyd

#endif // LEXY_DSL_BRACKETS_HPP_INCLUDED
