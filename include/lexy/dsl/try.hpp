// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TRY_HPP_INCLUDED
#define LEXY_DSL_TRY_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/failure.hpp>

namespace lexyd
{
template <typename Tag, typename Pattern>
struct _try : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            if (auto pos = input.cur(); Pattern::matcher::match(input))
                return NextParser::parse(context, input, LEXY_FWD(args)...);
            else
                return context.report_error(input,
                                            typename lexy::failure<Tag>::template error<Input>(
                                                pos));
        }
    };
};

/// Tries to match the pattern, report a tagged failure if it doesn't match.
template <typename Tag, typename Pattern>
LEXY_CONSTEVAL auto try_(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _try<Tag, Pattern>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TRY_HPP_INCLUDED

