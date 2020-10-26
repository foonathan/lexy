// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED
#define LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy::_detail
{
template <typename T, T... Indices>
struct integer_sequence
{
    using type = integer_sequence<T, Indices...>;
};
template <std::size_t... Indices>
using index_sequence = integer_sequence<std::size_t, Indices...>;

#if defined(__clang__)
template <std::size_t Size>
using make_index_sequence = __make_integer_seq<integer_sequence, std::size_t, Size>;
#elif defined(__GNUC__) && __GNUC__ >= 8
template <std::size_t Size>
using make_index_sequence = index_sequence<__integer_pack(Size)...>;
#else
#    error TODO
#endif

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED

