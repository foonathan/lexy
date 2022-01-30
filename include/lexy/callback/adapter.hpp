// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_ADAPTER_HPP_INCLUDED
#define LEXY_CALLBACK_ADAPTER_HPP_INCLUDED

#include <lexy/callback/base.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_SINK
#    define LEXY_DEPRECATED_SINK
#else
// `dsl::sink<T>(fn)` has been replaced by `lexy::fold_inplace<T>({}, fn)`.
// I'd put it into the deprecated message, but then GCC 7 doesn't like a std::enable_if in
// _detail::tuple's constructor. Really: removing the deprecated message from here, which is
// COMPLETELY UNRELATED CODE, fixes an SFINAE bug. I swear I'm not making this up.
#    define LEXY_DEPRECATED_SINK [[deprecated]]
#endif

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

template <typename Sink>
struct _cb_from_sink
{
    Sink _sink;

    using _cb         = lexy::sink_callback<Sink>;
    using return_type = typename _cb::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype((LEXY_DECLVAL(_cb&)(LEXY_FWD(args)), ..., LEXY_DECLVAL(_cb&&).finish()))
    {
        auto cb = _sink.sink();
        (cb(LEXY_FWD(args)), ...);
        return LEXY_MOV(cb).finish();
    }
};

/// Creates a callback that forwards all arguments to the sink.
template <typename Sink, typename = lexy::sink_callback<Sink>>
constexpr auto callback(Sink&& sink)
{
    return _cb_from_sink<std::decay_t<Sink>>{LEXY_FWD(sink)};
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

namespace lexy
{
template <typename MemFn>
struct _mem_fn_traits // MemFn is member data
{
    using return_type = MemFn;
};

#define LEXY_MAKE_MEM_FN_TRAITS(...)                                                               \
    template <typename ReturnType, typename... Args>                                               \
    struct _mem_fn_traits<ReturnType(Args...) __VA_ARGS__>                                         \
    {                                                                                              \
        using return_type = ReturnType;                                                            \
    };                                                                                             \
    template <typename ReturnType, typename... Args>                                               \
    struct _mem_fn_traits<ReturnType(Args..., ...) __VA_ARGS__>                                    \
    {                                                                                              \
        using return_type = ReturnType;                                                            \
    };

#define LEXY_MAKE_MEM_FN_TRAITS_CV(...)                                                            \
    LEXY_MAKE_MEM_FN_TRAITS(__VA_ARGS__)                                                           \
    LEXY_MAKE_MEM_FN_TRAITS(const __VA_ARGS__)                                                     \
    LEXY_MAKE_MEM_FN_TRAITS(volatile __VA_ARGS__)                                                  \
    LEXY_MAKE_MEM_FN_TRAITS(const volatile __VA_ARGS__)

#define LEXY_MAKE_MEM_FN_TRAITS_CV_REF(...)                                                        \
    LEXY_MAKE_MEM_FN_TRAITS_CV(__VA_ARGS__)                                                        \
    LEXY_MAKE_MEM_FN_TRAITS_CV(&__VA_ARGS__)                                                       \
    LEXY_MAKE_MEM_FN_TRAITS_CV(&&__VA_ARGS__)

LEXY_MAKE_MEM_FN_TRAITS_CV_REF()
LEXY_MAKE_MEM_FN_TRAITS_CV_REF(noexcept)

#undef LEXY_MAKE_MEM_FN_TRAITS_CV_REF
#undef LEXY_MAKE_MEM_FN_TRAITS_CV
#undef LEXY_MAKE_MEM_FN_TRAITS

template <typename Fn>
struct _mem_fn;
template <typename MemFn, typename T>
struct _mem_fn<MemFn T::*>
{
    MemFn T::*_fn;

    using return_type = typename _mem_fn_traits<MemFn>::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype(_detail::_mem_invoker<MemFn T::*>::invoke(_fn, LEXY_FWD(args)...))
    {
        return _detail::_mem_invoker<MemFn T::*>::invoke(_fn, LEXY_FWD(args)...);
    }
};

/// Creates a callback from a member function.
template <typename MemFn, typename T>
constexpr auto mem_fn(MemFn T::*fn)
{
    return _mem_fn<MemFn T::*>{fn};
}
} // namespace lexy

#endif // LEXY_CALLBACK_ADAPTER_HPP_INCLUDED

