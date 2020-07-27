// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_SEQUENCE_HPP_INCLUDED
#define LEXY_PATTERN_SEQUENCE_HPP_INCLUDED

#include <lexy/pattern/base.hpp>

namespace lexyd
{
template <typename... P>
struct _pseq : pattern_base<_pseq<P...>>
{
    static constexpr auto max_capture_count = (P::pattern::max_capture_count + ...);

    template <typename Context, typename Input>
    LEXY_DSL_FUNC bool match(Context& context, Input& input)
    {
        auto reset = input;
        if ((P::pattern::match(context, input) && ...))
            return true;

        input = LEXY_MOV(reset);
        return false;
    }

    LEXY_CONSTEVAL _pseq() = default;
    LEXY_CONSTEVAL _pseq(P...) {}
};

template <typename P1, typename P2, typename = lexy::_enable_pattern<P1, P2>>
LEXY_CONSTEVAL auto operator+(P1 p1, P2 p2)
{
    return _pseq(p1, p2);
}
template <typename... P, typename Other, typename = lexy::_enable_pattern<Other>>
LEXY_CONSTEVAL auto operator+(_pseq<P...>, Other other)
{
    return _pseq(P{}..., other);
}
template <typename Other, typename... P, typename = lexy::_enable_pattern<Other>>
LEXY_CONSTEVAL auto operator+(Other other, _pseq<P...>)
{
    return _pseq(other, P{}...);
}
template <typename... P, typename... U>
LEXY_CONSTEVAL auto operator+(_pseq<P...>, _pseq<U...>)
{
    return _pseq(P{}..., U{}...);
}
} // namespace lexyd

#endif // LEXY_PATTERN_SEQUENCE_HPP_INCLUDED
