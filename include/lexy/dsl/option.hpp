// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/match.hpp>

namespace lexyd
{
template <typename Branch>
struct _opt : rule_base
{
    static constexpr auto has_matcher = Branch::has_matcher;

    struct matcher
    {
        static constexpr auto sets_id           = Branch::matcher::sets_id;
        static constexpr auto max_capture_count = Branch::matcher::max_capture_count;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            if (auto reset = input; Branch::matcher::match(context, input))
                return true;
            else
            {
                input = LEXY_MOV(reset);
                return true;
            }
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            if (auto result = lexy::pattern_match(input, Branch::condition()))
                return Branch::template then_parser<NextParser>::parse(context, input,
                                                                       LEXY_FWD(args)...);
            else
                return NextParser::parse(context, input, LEXY_FWD(args)...);
        }
    };

    LEXY_CONSTEVAL _opt() = default;
    LEXY_CONSTEVAL _opt(Branch) {}
};

/// Matches the rule or nothing.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule rule)
{
    return _opt(branch(rule));
}
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED
