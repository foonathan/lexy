// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename R>
struct _while : dsl_base
{
    struct matcher
    {
        static constexpr auto max_capture_count = 0;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            while (R::matcher::match(context, input))
            {
            }

            return true;
        }
    };
};

template <typename R>
LEXY_CONSTEVAL auto while_(R)
{
    static_assert(R::matcher::max_capture_count == 0, "cannot repeat captures");
    return _while<R>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED
