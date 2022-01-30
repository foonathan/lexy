// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

template <typename T, typename State>
using _detect_callback_state = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(const State&)]);
template <typename T, typename State>
constexpr bool is_callback_state
    = _detail::is_detected<_detect_callback_state, T, std::decay_t<State>>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(const T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
} // namespace lexy

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED

