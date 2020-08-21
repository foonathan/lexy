// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_HPP_INCLUDED
#define LEXY_CALLBACK_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy
{
template <typename Fn>
struct _fn_holder
{
    Fn fn;

    constexpr explicit _fn_holder(Fn fn) : fn(fn) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(fn(LEXY_FWD(args)...))
    {
        return fn(LEXY_FWD(args)...);
    }
};

template <typename Fn>
using _fn_as_base = std::conditional_t<std::is_class_v<Fn>, Fn, _fn_holder<Fn>>;

template <typename ReturnType, typename... Fns>
struct _callback : _fn_as_base<Fns>...
{
    using return_type = ReturnType;

    LEXY_CONSTEVAL explicit _callback(Fns... fns) : _fn_as_base<Fns>(fns)... {}

    using _fn_as_base<Fns>::operator()...;

    // This is a fallback overload to create a nice error message if the callback isn't handling a
    // case. The const volatile qualification ensures that it is worse than any other option, unless
    // another callback is const volatile qualified (but who does that).
    template <typename... Args>
    constexpr return_type operator()(const Args&...) const volatile
    {
        static_assert(_detail::error<Args...>, "missing callback overload for Args...");
        return LEXY_DECLVAL(return_type);
    }
};

/// Creates a callback
template <typename ReturnType = void, typename... Fns>
LEXY_CONSTEVAL auto callback(Fns&&... fns)
{
    static_assert(((std::is_pointer_v<
                        std::decay_t<Fns>> || std::is_empty_v<std::decay_t<Fns>>)&&...),
                  "only capture-less lambdas are allowed in a callback");
    return _callback<ReturnType, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}
} // namespace lexy

namespace lexy
{
struct _null_callback
{
    using return_type = void;

    template <typename... Args>
    constexpr void operator()(const Args&...) const
    {}
};

/// A callback that does nothing.
inline constexpr auto null_callback = _null_callback{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _construct_callback
{
    using return_type = T;

    constexpr T operator()(T&& t) const
    {
        return LEXY_MOV(t);
    }
    constexpr T operator()(const T& t) const
    {
        return t;
    }

    template <typename... Args>
    constexpr T operator()(Args&&... args) const
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
            return T(LEXY_FWD(args)...);
        else
            return T{LEXY_FWD(args)...};
    }
};

/// A callback that constructs an object of type T.
template <typename T>
inline constexpr auto construct = _construct_callback<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_HPP_INCLUDED

