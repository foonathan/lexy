// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONDITION_HPP_INCLUDED
#define LEXY_DSL_CONDITION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/match.hpp>

namespace lexyd
{
template <typename Pattern, bool Expected>
struct _if : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        static constexpr auto sets_id           = Pattern::matcher::sets_id;
        static constexpr auto max_capture_count = Pattern::matcher::max_capture_count;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            auto copy = input;
            return Pattern::matcher::match(context, copy) == Expected;
        }
    };

    // TODO: parser
};

template <typename Pattern>
LEXY_CONSTEVAL auto if_(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _if<Pattern, true>{};
}

template <typename Pattern>
LEXY_CONSTEVAL auto unless(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _if<Pattern, false>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONDITION_HPP_INCLUDED

