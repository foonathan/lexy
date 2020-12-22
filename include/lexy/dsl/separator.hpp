// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Branch>
struct _sep
{};

/// Defines a separator for a list.
template <typename Branch>
LEXY_CONSTEVAL auto sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _sep<Branch>{};
}

template <typename Branch>
struct _tsep
{};

/// Defines a separator for a list that can be trailing.
template <typename Branch>
LEXY_CONSTEVAL auto trailing_sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _tsep<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED

