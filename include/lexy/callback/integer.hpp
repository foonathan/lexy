// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_INTEGER_HPP_INCLUDED
#define LEXY_CALLBACK_INTEGER_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/dsl/sign.hpp>

namespace lexy
{
template <typename T>
struct _int
{
    using return_type = T;

    template <typename Integer>
    constexpr T operator()(const Integer& value) const
    {
        return T(value);
    }
    template <typename Integer>
    constexpr T operator()(lexy::plus_sign, const Integer& value) const
    {
        return T(value);
    }
    template <typename Integer>
    constexpr T operator()(lexy::minus_sign, const Integer& value) const
    {
        return T(-value);
    }
};

// A callback that takes an optional sign and an integer and produces the signed integer.
template <typename T>
constexpr auto as_integer = _int<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_INTEGER_HPP_INCLUDED

