// Copyright (C) 2020 Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_DETECT_HPP_INCLUDED
#define LEXY_DETAIL_DETECT_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy::_detail
{
template <typename... Args>
using void_t = void;

template <template <typename...> typename Op, typename Void, typename... Args>
struct _detector : std::false_type
{};
template <template <typename...> typename Op, typename... Args>
struct _detector<Op, void_t<Op<Args...>>, Args...> : std::true_type
{};

template <template <typename...> typename Op, typename... Args>
constexpr bool is_detected = _detector<Op, void, Args...>::value;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_DETECT_HPP_INCLUDED

