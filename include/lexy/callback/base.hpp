// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_BASE_HPP_INCLUDED
#define LEXY_CALLBACK_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/invoke.hpp>

namespace lexy
{
template <typename T>
using _detect_callback = typename T::return_type;
template <typename T>
constexpr bool is_callback = _detail::is_detected<_detect_callback, T>;

template <typename T, typename... Args>
using _detect_callback_for = decltype(LEXY_DECLVAL(const T)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_callback_for
    = _detail::is_detected<_detect_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename Context>
using _detect_callback_context = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(const Context&)]);
template <typename T, typename Context>
constexpr bool is_callback_context
    = _detail::is_detected<_detect_callback_context, T, std::decay_t<Context>>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(const T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
} // namespace lexy

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED

