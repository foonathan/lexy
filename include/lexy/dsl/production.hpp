// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
/// Parses the rule of the production as if it were part of the current production.
template <typename Production>
constexpr auto inline_ = lexy::production_rule<Production>{};
} // namespace lexyd

namespace lexyd
{
// Not inline: one function per production.
template <typename Rule, typename Context, typename Reader>
constexpr bool _parse(Context& context, Reader& reader)
{
    return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader);
}
template <typename Rule, typename Context, typename Reader>
constexpr auto _try_parse(Context& context, Reader& reader)
{
    return lexy::rule_parser<Rule, lexy::context_value_parser>::try_parse(context, reader);
}

template <typename Production, typename Rule, typename NextParser>
struct _prd_parser
{
    struct _continuation
    {
        template <typename Context, typename Reader, typename ProdContext, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, ProdContext& prod_context,
                                 Args&&... args)
        {
            // If we're about to parse a token production, we need to continue with the parent's
            // (i.e. our current context) whitespace.
            using ws_next
                = std::conditional_t<lexy::is_token_production<Production>,
                                     lexy::whitespace_parser<Context, NextParser>, NextParser>;

            if constexpr (std::is_void_v<typename ProdContext::return_type>)
            {
                LEXY_MOV(prod_context).finish();
                return ws_next::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return ws_next::parse(context, reader, LEXY_FWD(args)...,
                                      LEXY_MOV(prod_context).finish());
            }
        }
    };

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
        -> lexy::rule_try_parse_result
    {
        auto prod_context = context.production_context(Production{}, reader.cur());

        if (auto result = _try_parse<Rule>(prod_context, reader);
            result == lexy::rule_try_parse_result::ok)
        {
            // We succesfully parsed the production.
            // The continuation will call `.finish()` for us.
            return static_cast<lexy::rule_try_parse_result>(
                _continuation::parse(context, reader, prod_context, LEXY_FWD(args)...));
        }
        else // backtracked or canceled
        {
            // Need to backtrack the partial production in either case.
            LEXY_MOV(prod_context).backtrack();
            return result;
        }
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto prod_context = context.production_context(Production{}, reader.cur());

        if (!_parse<Rule>(prod_context, reader))
        {
            // We failed to parse, need to backtrack.
            LEXY_MOV(prod_context).backtrack();
            return false;
        }

        // The continuation will call `.finish()` for us.
        return _continuation::parse(context, reader, prod_context, LEXY_FWD(args)...);
    }
};

template <typename Production>
struct _prd : rule_base
{
    using _rule = lexy::production_rule<Production>;

    static constexpr auto is_branch               = _rule::is_branch;
    static constexpr auto is_unconditional_branch = _rule::is_unconditional_branch;

    template <typename NextParser>
    using parser = _prd_parser<Production, _rule, NextParser>;

    template <typename Whitespace>
    constexpr auto operator[](Whitespace ws) const
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
    constexpr auto operator[](Whitespace ws) const
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

