// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/while.hpp>

namespace lexyd
{
template <typename Rule, typename Whitespace>
struct _ws : decltype(while_(Whitespace{}) + Rule{})
{
    /// Make it a branch rule, if the rule is a branch rule.
    /// If the rule isn't a branch rule, we could make the whitespace the condition, but this is
    /// probably insufficient to identify the rule.
    template <typename R = Rule, typename = std::enable_if_t<lexy::is_branch_rule<R>>>
    friend LEXY_CONSTEVAL auto branch(_ws)
    {
        // We just add another condition to the left of the branch rule.
        return while_(Whitespace{}) >> branch(Rule{});
    }
};

/// Matches whitespace before parsing rule.
template <typename Rule, typename Whitespace>
LEXY_CONSTEVAL auto whitespaced(Rule, Whitespace)
{
    return _ws<Rule, Whitespace>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED

