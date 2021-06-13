// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_BASE_HPP_INCLUDED
#define LEXY_CALLBACK_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/invoke.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_SINK
#    define LEXY_DEPRECATED_SINK
#else
#    define LEXY_DEPRECATED_SINK                                                                   \
        [[deprecated("`dsl::sink<T>(fn)` has been replaced by `lexy::fold_inplace<T>({}, fn)`")]]
#endif

//=== implementation ===//
namespace lexy
{
template <typename Fn>
struct _fn_holder
{
    Fn fn;

    constexpr explicit _fn_holder(Fn fn) : fn(fn) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype(_detail::invoke(fn, LEXY_FWD(args)...))
    {
        return _detail::invoke(fn, LEXY_FWD(args)...);
    }
};

template <typename Fn>
using _fn_as_base = std::conditional_t<std::is_class_v<Fn>, Fn, _fn_holder<Fn>>;
} // namespace lexy

//=== traits ===//
namespace lexy
{
template <typename T>
using _detect_callback = typename T::return_type;
template <typename T>
constexpr bool is_callback = _detail::is_detected<_detect_callback, T>;

template <typename T, typename... Args>
using _detect_callback_for = decltype(LEXY_DECLVAL(T)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_callback_for
    = _detail::is_detected<_detect_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename Context>
using _detect_callback_context = decltype(LEXY_DECLVAL(T)[LEXY_DECLVAL(const Context&)]);
template <typename T, typename Context>
constexpr bool is_callback_context = _detail::is_detected<_detect_callback_context, T, Context>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
} // namespace lexy

//=== adapters ===//
namespace lexy
{
template <typename ReturnType, typename... Fns>
struct _callback : _fn_as_base<Fns>...
{
    using return_type = ReturnType;

    constexpr explicit _callback(Fns... fns) : _fn_as_base<Fns>(fns)... {}

    using _fn_as_base<Fns>::operator()...;
};

/// Creates a callback.
template <typename ReturnType = void, typename... Fns>
constexpr auto callback(Fns&&... fns)
{
    return _callback<ReturnType, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}

template <typename T, typename Callback>
class _sink_cb
{
public:
    using return_type = T;

    constexpr explicit _sink_cb(Callback cb) : _value(), _cb(cb) {}

    template <typename... Args>
    constexpr void operator()(Args&&... args)
    {
        // We pass the value and other arguments to the internal callback.
        _cb(_value, LEXY_FWD(args)...);
    }

    constexpr T&& finish() &&
    {
        return LEXY_MOV(_value);
    }

private:
    T                          _value;
    LEXY_EMPTY_MEMBER Callback _cb;
};

template <typename T, typename... Fns>
class _sink
{
public:
    constexpr explicit _sink(Fns... fns) : _cb(fns...) {}

    constexpr auto sink() const
    {
        return _sink_cb<T, _callback<void, Fns...>>(_cb);
    }

private:
    LEXY_EMPTY_MEMBER _callback<void, Fns...> _cb;
};

/// Creates a sink callback.
template <typename T, typename... Fns>
LEXY_DEPRECATED_SINK constexpr auto sink(Fns&&... fns)
{
    return _sink<T, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}
} // namespace lexy

//=== composition ===//
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

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED

