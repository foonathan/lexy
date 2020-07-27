// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_WHILE_HPP_INCLUDED
#define LEXY_PATTERN_WHILE_HPP_INCLUDED

#include <lexy/pattern/base.hpp>

namespace lexyd
{
template <typename P>
struct _while : pattern_base<_while<P>>
{
    static constexpr auto max_capture_count = 0;

    template <typename Context, typename Input>
    LEXY_DSL_FUNC bool match(Context& context, Input& input)
    {
        while (P::pattern::match(context, input))
        {
        }

        return true;
    }
};

template <typename P>
LEXY_CONSTEVAL auto while_(P)
{
    static_assert(P::pattern::max_capture_count == 0, "cannot repeat captures");
    return _while<P>{};
}
} // namespace lexyd

#endif // LEXY_PATTERN_WHILE_HPP_INCLUDED

