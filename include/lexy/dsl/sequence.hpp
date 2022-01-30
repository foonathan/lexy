// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SEQUENCE_HPP_INCLUDED
#define LEXY_DSL_SEQUENCE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename... R>
struct _seq_impl;
template <>
struct _seq_impl<>
{
    template <typename NextParser>
    using p = NextParser;
};
template <typename H, typename... T>
struct _seq_impl<H, T...>
{
    // We parse Head and then seq<Tail...>.
    template <typename NextParser>
    using p = lexy::parser_for<H, lexy::parser_for<_seq_impl<T...>, NextParser>>;
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);

    template <typename NextParser>
    using p = lexy::parser_for<_seq_impl<R...>, NextParser>;
};

template <typename R, typename S>
constexpr auto operator+(R, S)
{
    return _seq<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator+(_seq<R...>, S)
{
    return _seq<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator+(R, _seq<S...>)
{
    return _seq<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator+(_seq<R...>, _seq<S...>)
{
    return _seq<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEQUENCE_HPP_INCLUDED

