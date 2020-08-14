// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Pattern>
struct _while : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Input>
        LEXY_DSL_FUNC bool match(Input& input)
        {
            while (Pattern::matcher::match(input))
            {
            }

            return true;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            // We match ourselves as pattern.
            matcher::match(input);
            // Then we continue.
            return NextParser::parse(context, input, LEXY_FWD(args)...);
        }
    };
};

template <typename Pattern>
LEXY_CONSTEVAL auto while_(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _while<Pattern>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED
