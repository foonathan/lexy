// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED
#define LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
template <typename First, typename Second>
struct _compose_cb
{
    LEXY_EMPTY_MEMBER First  _first;
    LEXY_EMPTY_MEMBER Second _second;

    using return_type = typename Second::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::decay_t<decltype(_first(LEXY_FWD(args)...), LEXY_DECLVAL(return_type))>
    {
        return _second(_first(LEXY_FWD(args)...));
    }
};

template <typename Sink, typename Callback>
struct _compose_s
{
    LEXY_EMPTY_MEMBER Sink     _sink;
    LEXY_EMPTY_MEMBER Callback _callback;

    using return_type = typename Callback::return_type;

    template <typename... Args>
    constexpr auto sink(Args&&... args) const -> decltype(_sink.sink(LEXY_FWD(args)...))
    {
        return _sink.sink(LEXY_FWD(args)...);
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_callback(LEXY_FWD(args)...))
    {
        return _callback(LEXY_FWD(args)...);
    }
};

/// Composes two callbacks.
template <typename First, typename Second, typename = _detect_callback<First>,
          typename = _detect_callback<Second>>
constexpr auto operator|(First first, Second second)
{
    return _compose_cb<First, Second>{LEXY_MOV(first), LEXY_MOV(second)};
}
template <typename S, typename Cb, typename Second>
constexpr auto operator|(_compose_s<S, Cb> composed, Second second)
{
    auto cb = LEXY_MOV(composed._callback) | LEXY_MOV(second);
    return _compose_s<S, decltype(cb)>{LEXY_MOV(composed._sink), LEXY_MOV(cb)};
}

/// Composes a sink with a callback.
template <typename Sink, typename Callback, typename = _detect_callback<Callback>>
constexpr auto operator>>(Sink sink, Callback cb)
{
    return _compose_s<Sink, Callback>{LEXY_MOV(sink), LEXY_MOV(cb)};
}
} // namespace lexy

#endif // LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED

