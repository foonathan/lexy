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
template <typename To, typename... Args>
inline constexpr auto _is_convertible = false;
template <typename To, typename Arg>
inline constexpr auto _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;
template <>
inline constexpr auto _is_convertible<void> = true;

template <typename State, typename Input, typename ErrorCallback>
class parse_handler
{
    using state_t  = _detail::type_or<State, _detail::no_bind_state>;
    using iterator = typename lexy::input_reader<Input>::iterator;

    template <typename Production>
    static auto _value_callback()
    {
        using value = lexy::production_value<Production>;
        if constexpr (lexy::is_callback<typename value::type>)
            return value::get;
        else if constexpr (lexy::is_sink<typename value::type, const state_t&>)
            return value::get.sink(LEXY_DECLVAL(const state_t&));
        else
            return value::get.sink();
    }
    template <typename Production>
    using _production_value_t = typename decltype(_value_callback<Production>())::return_type;

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
    class value_callback
    {
        using prod_value = lexy::production_value<Production>;

    public:
        constexpr explicit value_callback(parse_handler& handler) : _state(handler._state) {}

        using return_type = _production_value_t<Production>;

        constexpr auto sink() const
        {
            if constexpr (lexy::is_sink<typename prod_value::type, const state_t&>)
                return prod_value::get.sink(*_state);
            else
                return prod_value::get.sink();
        }

        template <typename... Args>
        constexpr return_type operator()(Args&&... args) const
        {
            using callback = typename prod_value::type;
            if constexpr (lexy::is_callback_for<callback, Args&&...>)
            {
                if constexpr (lexy::is_callback_state<callback, const state_t&>)
                    return prod_value::get[*_state](LEXY_FWD(args)...);
                else
                    return prod_value::get(LEXY_FWD(args)...);
            }
            else if constexpr (lexy::is_sink<callback> || lexy::is_sink<callback, const state_t&>)
            {
                if constexpr (_is_convertible<return_type, Args&&...>)
                {
                    // We don't have a matching callback, but it is a single argument that has
                    // the correct type already, or we return void and have no arguments.
                    // Assume it came from the list sink and return the value without invoking a
                    // callback.
                    return (LEXY_FWD(args), ...);
                }
                else
                {
                    static_assert(_detail::error<Production, Args...>,
                                  "missing value callback overload for production; only have sink");
                }
            }
            else
            {
                static_assert(_detail::error<Production, Args...>,
                              "missing value callback overload for production");
            }
        }

    private:
        const state_t* _state;
    };

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

