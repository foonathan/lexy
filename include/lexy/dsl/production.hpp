// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Production, typename Rule, typename NextParser>
struct _prd_parser
{
    template <typename Context, typename Input, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
        typename Context::result_type
    {
        auto&& sub_context  = context.template sub_context<Production>();
        using sub_context_t = std::decay_t<decltype(sub_context)>;

        if (auto result = Rule::template parser<sub_context_t>::parse(sub_context, input);
            sub_context.is_success(result))
            return NextParser::parse(context, input, LEXY_FWD(args)...,
                                     sub_context.forward_value(LEXY_MOV(result)));
        else
            return sub_context.forward_error_result(context, LEXY_MOV(result));
    }
};

template <typename Production, typename Rule>
struct _prd_then : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = _prd_parser<Production, Rule, NextParser>;
};

template <typename Production>
struct _prd : rule_base
{
    using _rule = decltype(Production().rule());

    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = _prd_parser<Production, _rule, NextParser>;

    // Allow using the production as a branch, if its rule is a branch rule.
    friend LEXY_CONSTEVAL auto branch(_prd)
    {
        static_assert(lexy::is_branch_rule<_rule>,
                      "production cannot be used in a branch without a condition");

        using branch_rule = decltype(branch(_rule()));
        return typename branch_rule::condition{}
               >> _prd_then<Production, typename branch_rule::then>{};
    }
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};

template <typename Production>
struct _rec : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser : _prd_parser<Production, decltype(Production().rule()), NextParser>
    {};
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

