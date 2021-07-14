// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_HPP_INCLUDED
#define LEXY_PARSE_HPP_INCLUDED

#include <lexy/_detail/invoke.hpp>
#include <lexy/action/base.hpp>
#include <lexy/callback/base.hpp>
#include <lexy/callback/bind.hpp>
#include <lexy/validate.hpp>

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
        _value.emplace(LEXY_MOV(v));
    }

    // In principle we could do a space optimization, as we can reconstruct the impl's status from
    // the state of _value and error. Feel free to implement it.
    _impl_t                     _impl;
    lexy::_detail::lazy_init<T> _value;

    template <typename State, typename Input, typename Callback>
    friend class _parse_handler;
};
} // namespace lexy

namespace lexy
{
template <typename To, typename... Args>
constexpr bool _is_convertible = false;
template <typename To, typename Arg>
constexpr bool _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;

template <typename State, typename Input, typename ErrorCallback>
class _parse_handler
{
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    constexpr explicit _parse_handler(const State& state, const Input& input,
                                      const ErrorCallback& callback)
    : _validate(input, callback), _state(state)
    {}

    template <typename T>
    constexpr auto get_result(lexy::_detail::lazy_init<T>&& value) &&
    {
        auto did_recover = static_cast<bool>(value);
        return parse_result<T, ErrorCallback>(LEXY_MOV(_validate).get_result(did_recover),
                                              LEXY_MOV(value));
    }

    //=== result ===//
    template <typename Production>
    static auto _value_callback()
    {
        using value = lexy::production_value<Production>;
        if constexpr (lexy::is_callback<typename value::type>)
            return value::get;
        else if constexpr (lexy::is_sink<typename value::type, const State&>)
            return value::get.sink(LEXY_DECLVAL(const State&));
        else
            return value::get.sink();
    }
    template <typename Production>
    using production_result = typename decltype(_value_callback<Production>())::return_type;

    template <typename Production>
    constexpr auto get_result_value(production_result<Production>&& value) && noexcept
    {
        using result_t = parse_result<production_result<Production>, ErrorCallback>;
        return result_t(LEXY_MOV(_validate).template get_result_value<Production>(),
                        LEXY_MOV(value));
    }
    template <typename Production>
    constexpr auto get_result_empty() && noexcept
    {
        using result_t = parse_result<production_result<Production>, ErrorCallback>;
        return result_t(LEXY_MOV(_validate).template get_result_empty<Production>());
    }

    //=== events ===//
    template <typename Production>
    using marker =
        typename lexy::validate_handler<Input, ErrorCallback>::template marker<Production>;

    template <typename Production, typename Iterator>
    constexpr marker<Production> on(parse_events::production_start<Production>, Iterator pos)
    {
        return {pos};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        using value = lexy::production_value<Production>;
        if constexpr (lexy::is_sink<typename value::type, const State&>)
            return value::get.sink(_state);
        else
            return value::get.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production> m, parse_events::error, Error&& error)
    {
        _validate.on(m, parse_events::error{}, LEXY_FWD(error));
    }

    template <typename Production, typename Iterator, typename... Args>
    constexpr auto on(marker<Production>&&, parse_events::production_finish<Production>, Iterator,
                      Args&&... args)
    {
        using value = typename lexy::production_value<Production>;

        if constexpr (lexy::is_callback_for<typename value::type, Args&&...>)
        {
            // We have a callback for those arguments; invoke it.
            if constexpr (lexy::is_callback_context<typename value::type, State>)
                return value::get[_state](LEXY_FWD(args)...);
            else
                return value::get(LEXY_FWD(args)...);
        }
        else if constexpr (((lexy::is_sink<typename value::type>)
                            || (lexy::is_sink<typename value::type, const State&>))
                           && _is_convertible<production_result<Production>, Args&&...>)
        {
            // We don't have a matching callback, but it is a single argument that has the
            // correct type already. Assume it came from the list sink and
            // construct the result without invoking a callback.
            return production_result<Production>(LEXY_FWD(args)...);
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
    const State&                                 _state;
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename State, typename Callback>
constexpr auto parse(const Input& input, State&& state, Callback callback)
{
    auto handler = lexy::_parse_handler(state, input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), reader);
}

template <typename Production, typename Input, typename Callback>
constexpr auto parse(const Input& input, Callback callback)
{
    return parse<Production>(input, _detail::no_bind_context{}, callback);
}
} // namespace lexy

#endif // LEXY_PARSE_HPP_INCLUDED

