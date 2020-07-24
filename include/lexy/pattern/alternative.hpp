// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_ALTERNATIVE_HPP_INCLUDED
#define LEXY_PATTERN_ALTERNATIVE_HPP_INCLUDED

#include <lexy/pattern/base.hpp>

namespace lexyd
{
template <typename... P>
struct _palt : pattern_base
{
    static constexpr auto max_capture_count = [] {
        std::size_t max      = 0;
        std::size_t counts[] = {P::max_capture_count...};
        for (auto count : counts)
            if (count > max)
                max = count;
        return max;
    }();

    template <typename Context, typename Input>
    LEXY_PATTERN_FUNC bool match(Context& context, Input& input)
    {
        return (P::match(context, input) || ...);
    }

    LEXY_CONSTEVAL _palt() = default;
    LEXY_CONSTEVAL _palt(P...) {}
};

template <typename P1, typename P2, typename = lexy::_enable_pattern<P1, P2>>
LEXY_CONSTEVAL auto operator/(P1 p1, P2 p2)
{
    return _palt(pattern(p1), pattern(p2));
}
template <typename... P, typename Other, typename = lexy::_enable_pattern<Other>>
LEXY_CONSTEVAL auto operator/(_palt<P...>, Other other)
{
    return _palt(P{}..., pattern(other));
}
template <typename Other, typename... P, typename = lexy::_enable_pattern<Other>>
LEXY_CONSTEVAL auto operator/(Other other, _palt<P...>)
{
    return _palt(pattern(other), P{}...);
}
template <typename... P, typename... U>
LEXY_CONSTEVAL auto operator/(_palt<P...>, _palt<U...>)
{
    return _palt(P{}..., U{}...);
}
} // namespace lexyd

#endif // LEXY_PATTERN_ALTERNATIVE_HPP_INCLUDED
