// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
// Not inline: one function per production.
template <typename Rule, typename Context, typename Reader>
constexpr bool _parse(Context& context, Reader& reader)
{
    return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader);
}

template <typename Production, typename Rule, typename NextParser>
struct _prd_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        // If we're about to parse a token production, we need to continue with the parent's (i.e.
        // our current context) whitespace.
        using continuation
            = std::conditional_t<lexy::is_token_production<Production>,
                                 lexy::whitespace_parser<Context, NextParser>, NextParser>;

        auto prod_ctx = context.production_context(Production{}, reader.cur());
        if (_parse<Rule>(prod_ctx, reader))
        {
            auto result = LEXY_MOV(prod_ctx).finish();
            if constexpr (std::is_same_v<decltype(result), lexy::result_value_t>)
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            else
                return continuation::parse(context, reader, LEXY_FWD(args)..., LEXY_MOV(result));
        }
        else
            return false;
    }
};

template <typename Production>
struct _prd : rule_base
{
    using _rule = lexy::production_rule<Production>;

    static constexpr bool is_branch = lexy::is_branch<_rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<_rule, Reader> _impl;
        typename Reader::iterator           _begin = {};

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            // The production really begins here, not when we start parsing the branch.
            _begin = reader.cur();
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // If we're about to parse a token production, we need to continue with the parent's
            // (i.e. our current context) whitespace.
            using continuation
                = std::conditional_t<lexy::is_token_production<Production>,
                                     lexy::whitespace_parser<Context, NextParser>, NextParser>;

            auto prod_ctx = context.production_context(Production{}, _begin);
            if (_impl.template parse<lexy::context_value_parser>(prod_ctx, reader))
            {
                auto result = LEXY_MOV(prod_ctx).finish();
                if constexpr (std::is_same_v<decltype(result), lexy::result_value_t>)
                    return continuation::parse(context, reader, LEXY_FWD(args)...);
                else
                    return continuation::parse(context, reader, LEXY_FWD(args)...,
                                               LEXY_MOV(result));
            }
            else
                return false;
        }
    };

    template <typename NextParser>
    using parser = _prd_parser<Production, _rule, NextParser>;

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};

template <typename Production>
struct _rec : rule_base
{
    template <typename NextParser>
    struct parser : _prd_parser<Production, lexy::production_rule<Production>, NextParser>
    {};

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

