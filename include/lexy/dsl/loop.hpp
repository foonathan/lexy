// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOP_HPP_INCLUDED
#define LEXY_DSL_LOOP_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _break : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context&, Reader&, Args&&... args)
        {
// GCC doesn't like how we use the comma operator.
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-value"
#endif

            // The last argument must be the state as looped rules must not push values.
            auto& state      = (LEXY_FWD(args), ...);
            state.loop_break = true;
            return typename Context::result_type(lexy::result_empty);

#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
        }
    };
};

/// Exits a loop().
constexpr auto break_ = _break{};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _loop : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            for (struct { bool loop_break = false; } state; !state.loop_break;)
            {
                using parser = lexy::rule_parser<Rule, lexy::context_discard_parser>;
                auto result  = parser::parse(context, reader, LEXY_FWD(args)..., state);
                if (result.has_error())
                    return LEXY_MOV(result);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeatedly matches the rule until a break rule matches.
template <typename Rule>
LEXY_CONSTEVAL auto loop(Rule)
{
    return _loop<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOP_HPP_INCLUDED

