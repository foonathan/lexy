// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename... R>
struct _alt : rule_base
{
    static constexpr auto has_matcher = (R::has_matcher && ...);

    struct matcher
    {
        template <typename Input>
        LEXY_DSL_FUNC bool match(Input& input)
        {
            return (R::matcher::match(input) || ...);
        }
    };

    //=== dsl ===//
    template <typename Other>
    friend LEXY_CONSTEVAL auto operator/(_alt<R...>, Other)
    {
        return _alt<R..., Other>{};
    }
    template <typename Other>
    friend LEXY_CONSTEVAL auto operator/(Other, _alt<R...>)
    {
        return _alt<Other, R...>{};
    }
};

template <typename R1, typename R2>
LEXY_CONSTEVAL auto operator/(R1, R2)
{
    return _alt<R1, R2>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
