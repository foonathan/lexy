// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_RESULT_HPP_INCLUDED
#define LEXY_RESULT_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <new>

#ifdef LEXY_IGNORE_DEPRECATED_RESULT
#    define LEXY_DEPRECATED_RESULT
#else
#    define LEXY_DEPRECATED_RESULT [[deprecated("lexy::result has been deprecated")]]
#endif

namespace lexy
{
enum class _result_state : unsigned char
{
    value,
    error,
};

struct result_value_t
{
    LEXY_CONSTEVAL operator _result_state() const
    {
        return _result_state::value;
    }
};

/// Tag value to indicate result success.
constexpr auto result_value = result_value_t{};

struct result_error_t
{
    LEXY_CONSTEVAL operator _result_state() const
    {
        return _result_state::error;
    }
};

/// Tag value to indiciate result error.
constexpr auto result_error = result_error_t{};
} // namespace lexy

namespace lexy
{
template <typename T, typename E>
struct _result_storage_trivial
{
    using value_type = T;
    using error_type = E;

    _result_state _state;
    union
    {
        T _value;
        E _error;
    };

    template <typename... Args>
    constexpr _result_storage_trivial(result_value_t tag, Args&&... args)
    : _state(tag), _value(LEXY_FWD(args)...)
    {}

    template <typename... Args>
    constexpr _result_storage_trivial(result_error_t tag, Args&&... args)
    : _state(tag), _error(LEXY_FWD(args)...)
    {}
};

template <typename T, typename E>
struct _result_storage_non_trivial
{
    using value_type = T;
    using error_type = E;

    _result_state _state;
    union
    {
        char _empty;
        T    _value;
        E    _error;
    };

    template <typename... Args>
    _result_storage_non_trivial(result_value_t tag, Args&&... args)
    : _state(tag), _value(LEXY_FWD(args)...)
    {}
    template <typename... Args>
    _result_storage_non_trivial(result_error_t tag, Args&&... args)
    : _state(tag), _error(LEXY_FWD(args)...)
    {}

    _result_storage_non_trivial(_result_storage_non_trivial&& other) noexcept
    : _state(other._state), _empty()
    {
        if (_state == _result_state::value)
            ::new (static_cast<void*>(&_value)) T(LEXY_MOV(other._value));
        else if (_state == _result_state::error)
            ::new (static_cast<void*>(&_error)) E(LEXY_MOV(other._error));
    }

    ~_result_storage_non_trivial() noexcept
    {
        if (_state == _result_state::value)
            _value.~T();
        else if (_state == _result_state::error)
            _error.~E();
    }

    _result_storage_non_trivial& operator=(_result_storage_non_trivial&& other) noexcept
    {
        // This can be made more efficient by leveraging assignment and what-not.
        // However, the vast majority of users should just get a result and unpack it immediately.
        // So let's keep this one simple and do a destroy + create.
        // (If you're reading this, feel free to add a PR).

        // Destroy.
        if (_state == _result_state::value)
            _value.~T();
        else if (_state == _result_state::error)
            _error.~E();

        _state = other._state;

        // Create.
        if (_state == _result_state::value)
            ::new (static_cast<void*>(&_value)) T(LEXY_MOV(other._value));
        else if (_state == _result_state::error)
            ::new (static_cast<void*>(&_error)) E(LEXY_MOV(other._error));

        return *this;
    }
};

// https://github.com/foonathan/lexy/pull/17
template <typename T, typename E>
using _result_storage_impl = std::conditional_t<
    std::is_trivially_move_constructible_v<
        T> && std::is_trivially_move_constructible_v<E> && std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E> && std::is_trivially_move_assignable_v<T> && std::is_trivially_move_assignable_v<E>,
    _result_storage_trivial<T, E>, _result_storage_non_trivial<T, E>>;
template <typename T, typename E>
using _result_storage
    = _result_storage_impl<std::conditional_t<std::is_void_v<T>, result_value_t, T>,
                           std::conditional_t<std::is_void_v<E>, result_error_t, E>>;
} // namespace lexy

namespace lexy
{
/// Stores a T or an E (or nothing).
/// Supports `void` for either one of them meaning "none".
template <typename T, typename E>
class LEXY_DEPRECATED_RESULT result : _result_storage<T, E>
{
    static constexpr auto optional_tag = [] {
        if constexpr (std::is_void_v<T>)
            return result_error;
        else
            return result_value;
    }();

public:
    using value_type = typename _result_storage<T, E>::value_type;
    using error_type = typename _result_storage<T, E>::error_type;

    //=== constructor ===//
    template <typename... Args>
    constexpr result(result_value_t, Args&&... args)
    : _result_storage<T, E>(result_value, LEXY_FWD(args)...)
    {}
    template <typename... Args>
    constexpr result(result_error_t, Args&&... args)
    : _result_storage<T, E>(result_error, LEXY_FWD(args)...)
    {}

    /// Conversion from an errored result with a different value type.
    template <typename U>
    constexpr explicit result(const result<U, E>& other) : result(result_error, other.error())
    {}
    template <typename U>
    constexpr explicit result(result<U, E>&& other) : result(result_error, LEXY_MOV(other).error())
    {}

    /// Construct a value without tag if we don't have an error.
    template <typename Arg, typename = std::enable_if_t<
                                (std::is_constructible_v<T, Arg> || std::is_constructible_v<E, Arg>)
                                || (std::is_void_v<T> || std::is_void_v<E>)>>
    constexpr explicit result(Arg&& arg) : result(optional_tag, LEXY_FWD(arg))
    {}

    //=== access ===//
    constexpr explicit operator bool() const noexcept
    {
        return this->_state == _result_state::value;
    }
    constexpr bool has_value() const noexcept
    {
        return this->_state == _result_state::value;
    }
    constexpr bool has_error() const noexcept
    {
        return this->_state == _result_state::error;
    }

    static constexpr bool has_void_value() noexcept
    {
        return std::is_same_v<T, void>;
    }
    static constexpr bool has_void_error() noexcept
    {
        return std::is_same_v<E, void>;
    }

    constexpr value_type& value() & noexcept
    {
        LEXY_PRECONDITION(has_value());
        return this->_value;
    }
    constexpr const value_type& value() const& noexcept
    {
        LEXY_PRECONDITION(has_value());
        return this->_value;
    }
    constexpr value_type&& value() && noexcept
    {
        LEXY_PRECONDITION(has_value());
        return LEXY_MOV(this->_value);
    }
    constexpr const value_type&& value() const&& noexcept
    {
        LEXY_PRECONDITION(has_value());
        return LEXY_MOV(this->_value);
    }

    constexpr error_type& error() & noexcept
    {
        LEXY_PRECONDITION(has_error());
        return this->_error;
    }
    constexpr const error_type& error() const& noexcept
    {
        LEXY_PRECONDITION(has_error());
        return this->_error;
    }
    constexpr error_type&& error() && noexcept
    {
        LEXY_PRECONDITION(has_error());
        return LEXY_MOV(this->_error);
    }
    constexpr const error_type&& error() const&& noexcept
    {
        LEXY_PRECONDITION(has_error());
        return LEXY_MOV(this->_error);
    }
};
} // namespace lexy

#endif // LEXY_RESULT_HPP_INCLUDED
