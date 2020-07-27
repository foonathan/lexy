// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename... R>
struct _alt : dsl_base
{
    struct matcher
    {
        static constexpr auto max_capture_count = [] {
            std::size_t max      = 0;
            std::size_t counts[] = {R::matcher::max_capture_count...};
            for (auto count : counts)
                if (count > max)
                    max = count;
            return max;
        }();

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context& context, Input& input)
        {
            return (R::matcher::match(context, input) || ...);
        }
    };

    LEXY_CONSTEVAL _alt() = default;
    LEXY_CONSTEVAL _alt(R...) {}
};

template <typename R1, typename R2>
LEXY_CONSTEVAL auto operator/(R1 r1, R2 r2)
{
    return _alt(r1, r2);
}
template <typename... R, typename Other>
LEXY_CONSTEVAL auto operator/(_alt<R...>, Other other)
{
    return _alt(R{}..., other);
}
template <typename Other, typename... R>
LEXY_CONSTEVAL auto operator/(Other other, _alt<R...>)
{
    return _alt(other, R{}...);
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt(R{}..., S{}...);
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
