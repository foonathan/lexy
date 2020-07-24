// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_OPTION_HPP_INCLUDED
#define LEXY_PATTERN_OPTION_HPP_INCLUDED

#include <lexy/pattern/base.hpp>

namespace lexyd
{
template <typename P>
struct _popt : pattern_base
{
    static constexpr auto max_capture_count = P::max_capture_count;

    template <typename Context, typename Input>
    LEXY_PATTERN_FUNC bool match(Context& context, Input& input)
    {
        if (auto reset = input; P::match(context, input))
            return true;
        else
        {
            input = LEXY_MOV(reset);
            return true;
        }
    }
};

template <typename P, typename = lexy::_enable_pattern<P>>
LEXY_CONSTEVAL auto opt(P p)
{
    return _popt<decltype(pattern(p))>{};
}
} // namespace lexyd

#endif // LEXY_PATTERN_OPTION_HPP_INCLUDED

