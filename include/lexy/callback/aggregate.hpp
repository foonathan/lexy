// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_AGGREGATE_HPP_INCLUDED
#define LEXY_CALLBACK_AGGREGATE_HPP_INCLUDED

#include <lexy/callback/base.hpp>
#include <lexy/dsl/member.hpp>

namespace lexy
{
template <typename MemPtr>
struct _mem_ptr_type_impl;
template <typename T, typename ClassT>
struct _mem_ptr_type_impl<T ClassT::*>
{
    using class_type  = ClassT;
    using member_type = T;
};

template <auto MemPtr>
using _mem_ptr_class_type = typename _mem_ptr_type_impl<decltype(MemPtr)>::class_type;
template <auto MemPtr>
using _mem_ptr_member_type = typename _mem_ptr_type_impl<decltype(MemPtr)>::member_type;

template <typename T>
struct _as_aggregate
{
    using return_type = T;
    static_assert(std::is_aggregate_v<return_type>);

    constexpr T operator()(T&& result) const
    {
        return LEXY_MOV(result);
    }

    template <typename Fn, typename Value, typename... Tail>
    constexpr T operator()(lexy::member<Fn>, Value&& value, Tail&&... tail) const
    {
        T result{};
        Fn{}(result, LEXY_FWD(value));
        return (*this)(LEXY_MOV(result), LEXY_FWD(tail)...);
    }
    template <typename Fn, typename Value, typename... Tail>
    constexpr T operator()(T&& result, lexy::member<Fn>, Value&& value, Tail&&... tail) const
    {
        Fn{}(result, LEXY_FWD(value));
        return (*this)(LEXY_MOV(result), LEXY_FWD(tail)...);
    }

    struct _sink
    {
        T _result{};

        using return_type = T;

        template <typename Fn, typename Value>
        constexpr void operator()(lexy::member<Fn>, Value&& value)
        {
            Fn()(_result, LEXY_FWD(value));
        }

        constexpr auto&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates an aggregate.
template <typename T>
constexpr auto as_aggregate = _as_aggregate<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_AGGREGATE_HPP_INCLUDED

