// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_HPP_INCLUDED
#define LEXY_CALLBACK_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/invoke.hpp>
#include <lexy/dsl/member.hpp>
#include <lexy/encoding.hpp>
#include <lexy/lexeme.hpp>

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

    LEXY_CONSTEVAL explicit _callback(Fns... fns) : _fn_as_base<Fns>(fns)... {}

    using _fn_as_base<Fns>::operator()...;
};

/// Creates a callback.
template <typename ReturnType = void, typename... Fns>
LEXY_CONSTEVAL auto callback(Fns&&... fns)
{
    static_assert(((std::is_pointer_v<std::decay_t<Fns>>           //
                    || std::is_member_pointer_v<std::decay_t<Fns>> //
                    || std::is_empty_v<std::decay_t<Fns>>)&&...),
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
template <typename First, typename Second>
struct _compose
{
    LEXY_EMPTY_MEMBER First  first;
    LEXY_EMPTY_MEMBER Second second;

    using return_type = typename Second::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::decay_t<decltype(first(LEXY_FWD(args)...), LEXY_DECLVAL(return_type))>
    {
        return second(first(LEXY_FWD(args)...));
    }
};

/// Composes two callbacks.
template <typename First, typename Second, typename = typename First::return_type,
          typename = typename Second::return_type>
LEXY_CONSTEVAL auto operator|(First first, Second second)
{
    return _compose<First, Second>{LEXY_MOV(first), LEXY_MOV(second)};
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

/// A callback with sink that does nothing.
inline constexpr auto noop = _noop{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _fwd
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
};

/// A callback that just forwards an existing object.
template <typename T>
constexpr auto forward = _fwd<T>{};

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

/// A callback that constructs an object of type T by forwarding the arguments.
template <typename T>
constexpr auto construct = _construct<T>{};

template <typename T, typename PtrT>
struct _new
{
    using return_type = PtrT;

    constexpr PtrT operator()(T&& t) const
    {
        auto ptr = new T(LEXY_MOV(t));
        return PtrT(ptr);
    }
    constexpr PtrT operator()(const T& t) const
    {
        auto ptr = new T(t);
        return PtrT(ptr);
    }

    template <typename... Args>
    constexpr PtrT operator()(Args&&... args) const
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
        {
            auto ptr = new T(LEXY_FWD(args)...);
            return PtrT(ptr);
        }
        else
        {
            auto ptr = new T{LEXY_FWD(args)...};
            return PtrT(ptr);
        }
    }
};

/// A callback that constructs an object of type T on the heap by forwarding the arguments.
template <typename T, typename PtrT = T*>
constexpr auto new_ = _new<T, PtrT>{};
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

/// A callback with sink that creates a list of things (e.g. a `std::vector`, `std::list`, etc.).
/// As a callback, it forwards the arguments to the initializer list constructor.
/// As a sink, it repeatedly calls `push_back()` and `emplace_back()`.
template <typename T>
constexpr auto as_list = _list<T>{};

template <typename T>
struct _collection
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
            _result.insert(obj);
        }
        void operator()(typename T::value_type&& obj)
        {
            _result.insert(LEXY_MOV(obj));
        }
        template <typename... Args>
        auto operator()(Args&&... args) -> std::enable_if_t<(sizeof...(Args) > 1)>
        {
            _result.emplace(LEXY_FWD(args)...);
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

/// A callback with sink that creates an unordered collection of things (e.g. a `std::set`,
/// `std::unordered_map`, etc.). As a callback, it forwards the arguments to the initializer list
/// constructor. As a sink, it repeatedly calls `insert()` and `emplace()`.
template <typename T>
constexpr auto as_collection = _collection<T>{};
} // namespace lexy

namespace lexy
{
template <typename MemPtr>
struct _mem_ptr_type_impl;
template <typename T, typename ClassT>
struct _mem_ptr_type_impl<T ClassT::*>
{
    using class_type  = ClassT;
    using member_type = T;
};

template <auto MemPtr>
using _mem_ptr_class_type = typename _mem_ptr_type_impl<decltype(MemPtr)>::class_type;
template <auto MemPtr>
using _mem_ptr_member_type = typename _mem_ptr_type_impl<decltype(MemPtr)>::member_type;

template <typename T>
struct _as_aggregate
{
    using return_type = T;
    static_assert(std::is_aggregate_v<return_type>);

    template <typename Fn, typename H, typename... Tail>
    constexpr void _set(T& result, lexy::member<Fn>, H&& value, Tail&&... tail) const
    {
        Fn()(result, LEXY_FWD(value));
        if constexpr (sizeof...(Tail) > 0)
            _set(result, LEXY_FWD(tail)...);
    }

    template <typename Fn, typename... Args>
    constexpr auto operator()(lexy::member<Fn> member, Args&&... args) const
    {
        static_assert(sizeof...(Args) % 2 == 1, "missing dsl::member rules");

        T result{};
        _set(result, member, LEXY_FWD(args)...);
        return result;
    }
    template <typename... Args>
    constexpr auto operator()(return_type&& result, Args&&... args) const
    {
        static_assert(sizeof...(Args) % 2 == 0, "missing dsl::member rules");

        _set(result, LEXY_FWD(args)...);
        return LEXY_MOV(result);
    }

    struct _sink
    {
        T _result{};

        using return_type = T;

        template <typename Fn, typename Value>
        constexpr void operator()(lexy::member<Fn>, Value&& value)
        {
            Fn()(_result, LEXY_FWD(value));
        }

        constexpr auto&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates an aggregate.
template <typename T>
constexpr auto as_aggregate = _as_aggregate<T>{};
} // namespace lexy

namespace lexy
{
template <typename String>
using _string_char_type = std::decay_t<decltype(LEXY_DECLVAL(String)[0])>;

template <typename String, typename Encoding>
struct _as_string
{
    using return_type = String;
    using _char_type  = _string_char_type<String>;

    constexpr String operator()(String&& str) const
    {
        return LEXY_MOV(str);
    }
    constexpr String operator()(const String& str) const
    {
        return str;
    }

    template <typename CharT>
    constexpr auto operator()(const CharT* str, std::size_t length) const
        -> decltype(String(str, length))
    {
        return String(str, length);
    }

    template <typename Reader>
    constexpr String operator()(lexeme<Reader> lex) const
    {
        using iterator = typename lexeme<Reader>::iterator;
        if constexpr (std::is_pointer_v<iterator>)
        {
            static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                          "cannot convert lexeme to this string type");

            if constexpr (std::is_same_v<_char_type, typename Reader::encoding::char_type>)
                return String(lex.data(), lex.size());
            else
                return String(reinterpret_cast<const _char_type*>(lex.data()), lex.size());
        }
        else
            return String(lex.begin(), lex.end());
    }

    constexpr String operator()(code_point cp) const
    {
        typename Encoding::char_type buffer[4] = {};
        auto                         size      = Encoding::encode_code_point(cp, buffer, 4);

        if constexpr (std::is_same_v<_char_type, typename Encoding::char_type>)
            return (*this)(buffer, size);
        else
            return (*this)(reinterpret_cast<const _char_type*>(buffer), size);
    }

    struct _sink
    {
        String _result;

        using return_type = String;

        template <typename CharT>
        auto operator()(CharT c) -> decltype(_result.push_back(c))
        {
            return _result.push_back(c);
        }

        void operator()(const String& str)
        {
            _result.append(str);
        }
        void operator()(String&& str)
        {
            _result.append(LEXY_MOV(str));
        }

        template <typename CharT>
        auto operator()(const CharT* str, std::size_t length)
            -> decltype(_result.append(str, length))
        {
            return _result.append(str, length);
        }

        template <typename Reader>
        void operator()(lexeme<Reader> lex)
        {
            using iterator = typename lexeme<Reader>::iterator;
            if constexpr (std::is_pointer_v<iterator>)
            {
                static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                              "cannot convert lexeme to this string type");
                _result.append(reinterpret_cast<const _char_type*>(lex.data()), lex.size());
            }
            else
            {
                _result.append(lex.begin(), lex.end());
            }
        }

        void operator()(code_point cp)
        {
            typename Encoding::char_type buffer[4] = {};
            auto                         size      = Encoding::encode_code_point(cp, buffer, 4);
            (*this)(reinterpret_cast<const _char_type*>(buffer), size);
        }

        String&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates a string (e.g. `std::string`).
/// As a callback, it converts a lexeme into the string.
/// As a sink, it repeatedly calls `.push_back()` for individual characters,
/// or `.append()` for lexemes or other strings.
template <typename String, typename Encoding = deduce_encoding<_string_char_type<String>>>
constexpr auto as_string = _as_string<String, Encoding>{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _int
{
    using return_type = T;

    template <typename Integer>
    constexpr T operator()(const Integer& value) const
    {
        return T(value);
    }
    template <typename Integer>
    constexpr T operator()(int sign, const Integer& value) const
    {
        return T(sign * value);
    }
};

// A callback that takes an optional sign and an integer and produces the signed integer.
template <typename T>
constexpr auto as_integer = _int<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_HPP_INCLUDED
