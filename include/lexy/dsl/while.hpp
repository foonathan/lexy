// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/match.hpp>

namespace lexyd
{
template <typename Pattern>
struct _while : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        static constexpr auto sets_id           = false;
        static constexpr auto max_capture_count = 0;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            while (Pattern::matcher::match(context, input))
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
            lexy::pattern_match(input, _while{});
            // Then we continue.
            return NextParser::parse(context, input, LEXY_FWD(args)...);
        }
    };
};

template <typename Pattern>
LEXY_CONSTEVAL auto while_(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    static_assert(!Pattern::matcher::sets_id, "repeated pattern must not set an id");
    static_assert(Pattern::matcher::max_capture_count == 0, "cannot repeat captures");
    return _while<Pattern>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED
