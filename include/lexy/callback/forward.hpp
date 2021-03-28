// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_FORWARD_HPP_INCLUDED
#define LEXY_CALLBACK_FORWARD_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy
{
template <typename T>
struct _fwd
{
    using return_type = T;

    constexpr T operator()(T&& t) const
    {
        return LEXY_MOV(t);
    }
    constexpr T operator()(const T& t) const
    {
        return t;
    }
};

/// A callback that just forwards an existing object.
template <typename T>
constexpr auto forward = _fwd<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_FORWARD_HPP_INCLUDED

