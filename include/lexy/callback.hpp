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
} // namespace lexy

namespace lexy
{
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

/// Creates a callback.
template <typename ReturnType = void, typename... Fns>
LEXY_CONSTEVAL auto callback(Fns&&... fns)
{
    static_assert(((std::is_pointer_v<
                        std::decay_t<Fns>> || std::is_empty_v<std::decay_t<Fns>>)&&...),
                  "only capture-less lambdas are allowed in a callback");
    return _callback<ReturnType, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}

/// Invokes a callback into a result.
template <typename Result, typename ErrorOrValue, typename Callback, typename... Args>
constexpr Result invoke_as_result(ErrorOrValue tag, Callback&& callback, Args&&... args)
{
    using callback_t  = std::decay_t<Callback>;
    using return_type = typename callback_t::return_type;

    if constexpr (std::is_same_v<return_type, void>)
    {
        LEXY_FWD(callback)(LEXY_FWD(args)...);
        return Result(tag);
    }
    else
    {
        return Result(tag, LEXY_FWD(callback)(LEXY_FWD(args)...));
    }
}
} // namespace lexy

namespace lexy
{
template <typename T, typename Callback>
class _sink
{
public:
    using return_type = T;

    constexpr explicit _sink(Callback cb) : _value(), _cb(cb) {}

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
class _sink_callback
{
public:
    LEXY_CONSTEVAL explicit _sink_callback(Fns... fns) : _cb(fns...) {}

    constexpr auto sink() const
    {
        return _sink<T, _callback<void, Fns...>>(_cb);
    }

private:
    LEXY_EMPTY_MEMBER _callback<void, Fns...> _cb;
};

/// Creates a sink callback.
template <typename T, typename... Fns>
LEXY_CONSTEVAL auto sink(Fns&&... fns)
{
    static_assert(((std::is_pointer_v<
                        std::decay_t<Fns>> || std::is_empty_v<std::decay_t<Fns>>)&&...),
                  "only capture-less lambdas are allowed in a callback");
    return _sink_callback<T, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}
} // namespace lexy

namespace lexy
{
struct _noop
{
    using return_type = void;

    constexpr auto sink() const
    {
        // We don't need a separate type, noop itself can have the required functions.
        return *this;
    }

    template <typename... Args>
    constexpr void operator()(const Args&...) const
    {}

    constexpr void finish() && {}
};

/// A callback that does nothing.
inline constexpr auto noop = _noop{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _construct
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
inline constexpr auto construct = _construct<T>{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _list
{
    using return_type = T;

    template <typename... Args>
    constexpr T operator()(Args&&... args) const
    {
        // Use the initializer_list constructor.
        return T{LEXY_FWD(args)...};
    }

    struct _sink
    {
        T _result;

        using return_type = T;

        void operator()(const typename T::value_type& obj)
        {
            _result.push_back(obj);
        }
        void operator()(typename T::value_type&& obj)
        {
            _result.push_back(LEXY_MOV(obj));
        }
        template <typename... Args>
        auto operator()(Args&&... args) -> std::enable_if_t<(sizeof...(Args) > 1)>
        {
            _result.emplace_back(LEXY_FWD(args)...);
        }

        T&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback that builds a list of things.
template <typename T>
inline constexpr auto list = _list<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_HPP_INCLUDED
