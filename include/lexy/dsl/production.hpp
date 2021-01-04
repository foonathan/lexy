// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexyd
{
// Not inline: one function per production.
template <typename Rule, typename Context, typename Reader>
constexpr auto _parse(Context& context, Reader& reader) -> typename Context::result_type
{
    return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader);
}

template <typename Production, typename Rule, typename NextParser>
struct _prd_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
        typename Context::result_type
    {
        if (lexy::parse_context prod_ctx(context, Production{}, reader.cur());
            auto                result = _parse<Rule>(prod_ctx, reader))
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

template <typename Production>
struct _prd : rule_base
{
    using _rule = typename lexy::production_traits<Production>::rule::type;

    static constexpr bool is_branch = lexy::is_branch<_rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<_rule, Reader> _impl;

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            if (lexy::parse_context prod_ctx(context, Production{}, reader.cur());
                auto result = _impl.template parse<lexy::context_value_parser>(prod_ctx, reader))
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
    struct parser
    : _prd_parser<Production, typename lexy::production_traits<Production>::rule::type, NextParser>
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

