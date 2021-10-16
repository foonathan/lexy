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
    constexpr explicit parse_result(_impl_t&& impl) noexcept : _impl(LEXY_MOV(impl)), _value()
    {
        LEXY_PRECONDITION(impl.is_fatal_error());
    }
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
inline constexpr bool _is_convertible = false;
template <typename To, typename Arg>
inline constexpr bool _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;

template <typename State, typename Input, typename ErrorCallback>
class parse_handler
{
    using state_t  = _detail::type_or<State, _detail::no_bind_context>;
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
    : _validate(input, callback), _state(state)
    {}

    //=== events ===//
    template <typename Production>
    struct marker
    {
        using impl =
            typename lexy::validate_handler<Input, ErrorCallback>::template marker<Production>;

        impl                                                _impl;
        _detail::lazy_init<_production_value_t<Production>> _value;

        constexpr auto get_value() &&
        {
            LEXY_PRECONDITION(_value);
            if constexpr (std::is_void_v<_production_value_t<Production>>)
                return;
            else
                return LEXY_MOV(*_value);
        }
    };

    template <typename Production>
    constexpr auto get_action_result(bool parse_result, marker<Production>&& m) &&
    {
        using result_t = lexy::parse_result<_production_value_t<Production>, ErrorCallback>;

        auto validate_result
            = LEXY_MOV(_validate).get_action_result(parse_result, LEXY_MOV(m._impl));
        if (parse_result)
            return result_t(LEXY_MOV(validate_result), LEXY_MOV(m).get_value());
        else
            return result_t(LEXY_MOV(validate_result));
    }

    template <typename Production, typename Iterator>
    constexpr marker<Production> on(parse_events::production_start<Production>, Iterator pos)
    {
        return {{pos}, {}};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(const marker<Production>&, parse_events::list, Iterator)
    {
        using value = lexy::production_value<Production>;
        if constexpr (lexy::is_sink<typename value::type, const state_t&>)
            return value::get.sink(_state);
        else
            return value::get.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(const marker<Production>& m, parse_events::error, Error&& error)
    {
        _validate.on(m._impl, parse_events::error{}, LEXY_FWD(error));
    }

    template <typename Production, typename Iterator, typename... Args>
    constexpr void on(marker<Production>& m, parse_events::production_finish<Production>, Iterator,
                      Args&&... args)
    {
        using value   = typename lexy::production_value<Production>;
        using value_t = _production_value_t<Production>;

        if constexpr (lexy::is_callback_for<typename value::type, Args&&...>)
        {
            // We have a callback for those arguments; invoke it.
            if constexpr (lexy::is_callback_context<typename value::type, state_t>)
                m._value.emplace_result(value::get[_state], LEXY_FWD(args)...);
            else
                m._value.emplace_result(value::get, LEXY_FWD(args)...);
        }
        else if constexpr ((lexy::is_sink<typename value::type>)
                           || (lexy::is_sink<typename value::type, const state_t&>))
        {
            if constexpr (std::is_void_v<value_t>)
            {
                // We don't have a matching callback, but the sink returns void, which is our
                // value.
                m._value.emplace();
            }
            else if constexpr (_is_convertible<value_t, Args&&...>)
            {
                // We don't have a matching callback, but it is a single argument that has the
                // correct type already. Assume it came from the list sink and
                // construct the value without invoking a callback.
                m._value.emplace(LEXY_FWD(args)...);
            }
            else
            {
                // We're missing a callback overload.
                static_assert(_detail::error<Production, Args...>,
                              "missing value callback overload for production; only have sink");
            }
        }
        else
        {
            // We're missing a callback overload.
            static_assert(_detail::error<Production, Args...>,
                          "missing value callback overload for production");
        }
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

private:
    lexy::validate_handler<Input, ErrorCallback> _validate;
    const state_t&                               _state;
};

template <typename State, typename Input, typename ErrorCallback>
parse_handler(const State&, const Input& input, const ErrorCallback& callback)
    -> parse_handler<State, Input, ErrorCallback>;
template <typename Input, typename ErrorCallback>
parse_handler(const _detail::no_bind_context&, const Input& input, const ErrorCallback& callback)
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
    return parse<Production>(input, _detail::no_bind_context{}, LEXY_MOV(callback));
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_HPP_INCLUDED

