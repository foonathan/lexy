// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_CONSTANT_HPP_INCLUDED
#define LEXY_CALLBACK_CONSTANT_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy
{
template <typename T>
struct _constant
{
    T _value;

    using return_type = T;

    constexpr const T& operator()() const
    {
        return _value;
    }
};

/// Creates a callback that produces the given value without accepting arguments.
template <typename Arg>
LEXY_CONSTEVAL auto constant(Arg&& value)
{
    return _constant<std::decay_t<Arg>>{LEXY_FWD(value)};
}
} // namespace lexy

#endif // LEXY_CALLBACK_CONSTANT_HPP_INCLUDED

