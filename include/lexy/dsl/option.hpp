// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Rule>
struct _opt : rule_base
{
    static constexpr auto has_matcher = Rule::has_matcher;

    struct matcher
    {
        static constexpr auto sets_id           = Rule::matcher::sets_id;
        static constexpr auto max_capture_count = Rule::matcher::max_capture_count;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            if (auto reset = input; Rule::matcher::match(context, input))
                return true;
            else
            {
                input = LEXY_MOV(reset);
                return true;
            }
        }
    };
};

/// Matches the rule or nothing.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule)
{
    return _opt<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED
