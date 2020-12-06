// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_RESULT_HPP_INCLUDED
#define LEXY_RESULT_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <new>
#include <variant>

namespace lexy
{
struct result_value_t
{};

/// Tag value to indicate result success.
constexpr auto result_value = result_value_t{};

struct result_error_t
{};

/// Tag value to indiciate result error.
constexpr auto result_error = result_error_t{};
} // namespace lexy

namespace lexy
{
template<typename T, typename U>
using _result_t = std::conditional_t<std::is_void_v<T>, U, T>;

template <typename T, typename E>
class _result_storage: protected std::variant<_result_t<T, result_value_t>,
                                              _result_t<E, result_error_t>> {
protected:
    using value_type = _result_t<T, result_value_t>;
    using error_type = _result_t<E, result_error_t>;
    bool _has_value;

    using parent = std::variant<value_type, error_type>;

    template<typename ...Args>
    constexpr _result_storage(result_value_t, Args&&... args)
    : parent{std::in_place_index_t<0>{}, std::forward<Args>(args)...}, _has_value(true) {}

    template<typename ...Args>
    constexpr _result_storage(result_error_t, Args&&... args)
    : parent{std::in_place_index_t<1>{}, std::forward<Args>(args)...}, _has_value(false) {}
};

} // namespace lexy

namespace lexy
{
/// Stores a T or an E.
/// Supports `void` for either one of them meaning "none".
template <typename T, typename E>
class result : _result_storage<T, E>
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
    constexpr result() : _result_storage<T, E>(result_error) {}

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
        return this->_has_value;
    }
    constexpr bool has_value() const noexcept
    {
        return this->_has_value;
    }
    constexpr bool has_error() const noexcept
    {
        return !this->_has_value;
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
        return std::get<0>(*this);
    }
    constexpr const value_type& value() const& noexcept
    {
        LEXY_PRECONDITION(has_value());
        return std::get<0>(*this);
    }
    constexpr value_type&& value() && noexcept
    {
        LEXY_PRECONDITION(has_value());
        return LEXY_MOV(std::get<0>(*this));
    }
    constexpr const value_type&& value() const&& noexcept
    {
        LEXY_PRECONDITION(has_value());
        return LEXY_MOV(std::get<0>(*this));
    }

    constexpr error_type& error() & noexcept
    {
        LEXY_PRECONDITION(has_error());
        return std::get<1>(*this);
    }
    constexpr const error_type& error() const& noexcept
    {
        LEXY_PRECONDITION(has_error());
        return std::get<1>(*this);
    }
    constexpr error_type&& error() && noexcept
    {
        LEXY_PRECONDITION(has_error());
        return LEXY_MOV(std::get<1>(*this));
    }
    constexpr const error_type&& error() const&& noexcept
    {
        LEXY_PRECONDITION(has_error());
        return LEXY_MOV(std::get<1>(*this));
    }
};
} // namespace lexy

#endif // LEXY_RESULT_HPP_INCLUDED
