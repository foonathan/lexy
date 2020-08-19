// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Production, typename Rule>
struct _prd : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            auto&& sub_context  = context.template sub_context<Production>();
            using sub_context_t = std::decay_t<decltype(sub_context)>;

            if (auto result = Rule::template parser<sub_context_t>::parse(sub_context, input);
                sub_context.success(result))
                return NextParser::parse(context, input, LEXY_FWD(args)...,
                                         sub_context.forward_value(LEXY_MOV(result)));
            else
                return sub_context.forward_error(LEXY_MOV(result));
        }
    };
};

/// Parses the production.
template <typename Production>
constexpr auto p = [] {
    using rule = decltype(Production().rule());
    if constexpr (lexy::is_branch_rule<rule>)
    {
        using branch_rule = decltype(branch(rule()));
        return typename branch_rule::condition{} >> _prd<Production, typename branch_rule::then>{};
    }
    else
        return _prd<Production, rule>{};
}();

template <typename Production>
struct _rec : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser : _prd<Production, decltype(Production().rule())>::template parser<NextParser>
    {};
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

