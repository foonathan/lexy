// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_TUPLE_HPP_INCLUDED
#define LEXY_DETAIL_TUPLE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/integer_sequence.hpp>

namespace lexy::_detail
{
template <std::size_t Idx, typename T>
struct _tuple_holder
{
    LEXY_EMPTY_MEMBER T value;
};

template <std::size_t Idx, typename... T>
struct _nth_type;
template <std::size_t Idx, typename H, typename... T>
struct _nth_type<Idx, H, T...>
{
    using type = typename _nth_type<Idx - 1, T...>::type;
};
template <typename H, typename... T>
struct _nth_type<0, H, T...>
{
    using type = H;
};

template <typename Indices, typename... T>
class _tuple;
template <std::size_t... Idx, typename... T>
class _tuple<index_sequence<Idx...>, T...> : public _tuple_holder<Idx, T>...
{
public:
    template <typename... Args>
    constexpr _tuple(int, Args&&... args) : _tuple_holder<Idx, T>{LEXY_FWD(args)}...
    {}
};

template <typename... T>
struct tuple : _tuple<index_sequence_for<T...>, T...>
{
    template <typename... Args>
    constexpr tuple(int, Args&&... args)
    : _tuple<index_sequence_for<T...>, T...>(0, LEXY_FWD(args)...)
    {}

    template <std::size_t N>
    using element_type = typename _nth_type<N, T...>::type;

    template <std::size_t N>
    constexpr decltype(auto) get() noexcept
    {
        // NOLINTNEXTLINE: this is perfectly fine.
        return LEXY_FWD(static_cast<_tuple_holder<N, element_type<N>>&>(*this).value);
    }
    template <std::size_t N>
    constexpr decltype(auto) get() const noexcept
    {
        // NOLINTNEXTLINE: this is perfectly fine.
        return LEXY_FWD(static_cast<const _tuple_holder<N, element_type<N>>&>(*this).value);
    }

    constexpr auto index_sequence() const
    {
        return index_sequence_for<T...>{};
    }
};
template <>
struct tuple<>
{
    constexpr tuple(int) {}

    static constexpr auto index_sequence()
    {
        return index_sequence_for<>{};
    }
};

template <typename... Args>
constexpr auto make_tuple(Args&&... args)
{
    return tuple<std::decay_t<Args>...>(0, LEXY_FWD(args)...);
}

template <typename... Args>
constexpr auto forward_as_tuple(Args&&... args)
{
    return tuple<Args&&...>(0, LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TUPLE_HPP_INCLUDED

