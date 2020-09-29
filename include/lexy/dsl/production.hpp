// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/result.hpp>

namespace lexyd
{
template <typename Production>
using _production_rule = std::remove_const_t<decltype(Production::rule)>;

template <typename Production, typename Rule, typename NextParser>
struct _prd_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
        typename Context::result_type
    {
        auto&& sub_context = context.template sub_context<Production>();

        if (auto result = Rule::template parser<lexy::final_parser>::parse(sub_context, reader))
        {
            if constexpr (result.has_void_value())
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         LEXY_MOV(result).value());
        }
        else
            return typename Context::result_type(LEXY_MOV(result));
    }
};

template <typename Production, typename Rule = void>
struct _prd : rule_base
{
    using _rule = std::conditional_t<std::is_void_v<Rule>, _production_rule<Production>, Rule>;

    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = _prd_parser<Production, _rule, NextParser>;

    // Allow using the production as a branch, if its rule is a branch rule.
    template <typename R = _rule, typename = std::enable_if_t<lexy::is_branch_rule<R>>>
    friend LEXY_CONSTEVAL auto branch(_prd)
    {
        using branch_rule = decltype(branch(_rule()));
        return typename branch_rule::condition{} >> _prd<Production, typename branch_rule::then>{};
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
    struct parser : _prd_parser<Production, _production_rule<Production>, NextParser>
    {};
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

