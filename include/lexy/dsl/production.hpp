// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Production>
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
            using rule          = decltype(Production().rule());

            if (auto result = rule::template parser<sub_context_t>::parse(sub_context, input);
                sub_context.success(result))
                return NextParser::parse(context, input, LEXY_FWD(args)...,
                                         sub_context.forward_value(result));
            else
                return context.report_error(sub_context.forward_error(result));
        }
    };
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

