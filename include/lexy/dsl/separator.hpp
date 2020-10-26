// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Pattern, bool Capture>
struct _sep
{
    LEXY_CONSTEVAL auto capture() const
    {
        return _sep<Pattern, true>{};
    }
};

/// Defines a separator pattern for a list.
template <typename Pattern>
LEXY_CONSTEVAL auto sep(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _sep<Pattern, false>{};
}

template <typename Pattern, bool Capture>
struct _tsep
{
    LEXY_CONSTEVAL auto capture() const
    {
        return _tsep<Pattern, true>{};
    }
};

/// Defines a separator pattern for a list that can be trailing.
template <typename Pattern>
LEXY_CONSTEVAL auto trailing_sep(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _tsep<Pattern, false>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED

