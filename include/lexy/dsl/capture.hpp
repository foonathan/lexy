// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Rule>
struct _cap : dsl_base
{
    struct matcher
    {
        static constexpr auto max_capture_count = 1 + Rule::matcher::max_capture_count;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            auto idx = context._capture_count++;

            auto begin = input.cur();
            if (!Rule::matcher::match(context, input))
            {
                context._capture_count--; // Undo the increment.
                return false;
            }
            auto end = input.cur();

            context._captures[idx] = {begin, end};
            return true;
        }
    };
};

/// Captures whatever the rule matches as a lexeme.
template <typename Rule>
LEXY_CONSTEVAL auto capture(Rule)
{
    return _cap<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED
