// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_PARSE_HPP_INCLUDED
#define LEXY_ACTION_PARSE_HPP_INCLUDED

#include <lexy/_detail/invoke.hpp>
#include <lexy/action/base.hpp>
#include <lexy/action/validate.hpp>
#include <lexy/callback/base.hpp>
#include <lexy/callback/bind.hpp>

namespace lexy
{
template <typename T, typename ErrorCallback>
class parse_result
{
    using _impl_t = lexy::validate_result<ErrorCallback>;

public:
    using value_type     = T;
    using error_callback = ErrorCallback;
    using error_type     = typename _impl_t::error_type;

    //=== status ===//
    constexpr explicit operator bool() const noexcept
    {
        return _impl.is_success();
    }

    constexpr bool is_success() const noexcept
    {
        return _impl.is_success();
    }
    constexpr bool is_error() const noexcept
    {
        return _impl.is_error();
    }
    constexpr bool is_recovered_error() const noexcept
    {
        return _impl.is_recovered_error();
    }
    constexpr bool is_fatal_error() const noexcept
    {
        return _impl.is_fatal_error();
    }

    //=== value ===//
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

    constexpr const auto& value() const& noexcept
    {
        return *_value;
    }
    constexpr auto&& value() && noexcept
    {
        return LEXY_MOV(*_value);
    }

    //=== error ===//
    constexpr std::size_t error_count() const noexcept
    {
        return _impl.error_count();
    }

    constexpr const auto& errors() const& noexcept
    {
        return _impl.errors();
    }
    constexpr auto&& errors() && noexcept
    {
        return LEXY_MOV(_impl).errors();
    }

private:
    constexpr explicit parse_result(_impl_t&& impl) noexcept : _impl(LEXY_MOV(impl)), _value() {}
    constexpr explicit parse_result(_impl_t&& impl, T&& v) noexcept : _impl(LEXY_MOV(impl))
    {
        LEXY_PRECONDITION(impl.is_success() || impl.is_recovered_error());
        _value.emplace(LEXY_MOV(v));
    }

    // In principle we could do a space optimization, as we can reconstruct the impl's status from
    // the state of _value and error. Feel free to implement it.
    _impl_t                     _impl;
    lexy::_detail::lazy_init<T> _value;

    template <typename State, typename Input, typename Callback>
    friend class parse_handler;
};
} // namespace lexy

namespace lexy
{
template <typename State, typename Input, typename ErrorCallback>
class parse_handler
{
    using state_t  = _detail::type_or<State, _detail::no_bind_state>;
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    constexpr explicit parse_handler(const state_t& state, const Input& input,
                                     const ErrorCallback& callback)
    : _validate(input, callback), _state(&state)
    {}

    template <typename Production>
    using event_handler =
        typename validate_handler<Input, ErrorCallback>::template event_handler<Production>;

    constexpr operator validate_handler<Input, ErrorCallback>&()
    {
        return _validate;
    }

    template <typename Production>
    using value_callback = production_value_callback<Production, state_t>;

    template <typename Production>
    constexpr explicit operator value_callback<Production>()
    {
        return value_callback<Production>(*_state);
    }

    constexpr auto get_result_void(bool rule_parse_result) &&
    {
        return parse_result<void, ErrorCallback>(
            LEXY_MOV(_validate).get_result_void(rule_parse_result));
    }

    template <typename T>
    constexpr auto get_result(bool rule_parse_result, T&& result) &&
    {
        return parse_result<T, ErrorCallback>(LEXY_MOV(_validate).get_result_void(
                                                  rule_parse_result),
                                              LEXY_MOV(result));
    }
    template <typename T>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        return parse_result<T, ErrorCallback>(
            LEXY_MOV(_validate).get_result_void(rule_parse_result));
    }

private:
    validate_handler<Input, ErrorCallback> _validate;
    const state_t*                         _state;
};

template <typename State, typename Input, typename ErrorCallback>
parse_handler(const State&, const Input& input, const ErrorCallback& callback)
    -> parse_handler<State, Input, ErrorCallback>;
template <typename Input, typename ErrorCallback>
parse_handler(const _detail::no_bind_state&, const Input& input, const ErrorCallback& callback)
    -> parse_handler<void, Input, ErrorCallback>;

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename State, typename Callback>
constexpr auto parse(const Input& input, State&& state, Callback callback)
{
    auto handler = lexy::parse_handler(state, input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), reader);
}

template <typename Production, typename Input, typename Callback>
constexpr auto parse(const Input& input, Callback callback)
{
    return parse<Production>(input, _detail::no_bind_state{}, LEXY_MOV(callback));
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_HPP_INCLUDED

