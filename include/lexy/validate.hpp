// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED

#include <lexy/_detail/lazy_init.hpp>
#include <lexy/action/base.hpp>
#include <lexy/callback/base.hpp>
#include <lexy/callback/container.hpp>
#include <lexy/callback/noop.hpp>
#include <lexy/error.hpp>

namespace lexy
{
// Convert the callback into an appropriate sink.
template <typename Callback>
constexpr auto _get_error_sink(const Callback& callback)
{
    if constexpr (std::is_same_v<Callback, lexy::_noop>)
    {
        // We collect noop instead, which counts the errors.
        return lexy::collect(callback).sink();
    }
    else if constexpr (lexy::is_sink<Callback>)
    {
        // It already is a sink.
        return callback.sink();
    }
    else
    {
        static_assert(
            std::is_void_v<typename Callback::return_type>,
            "need to use `lexy::collect()` to create an error callback that can handle multiple errors");

        // We need to collect the errors.
        return lexy::collect(callback).sink();
    }
}
template <typename Callback>
using _error_sink_t = decltype(_get_error_sink(LEXY_DECLVAL(Callback)));

template <typename ErrorCallback>
class validate_result
{
    using _sink_t = _error_sink_t<ErrorCallback>;

public:
    using error_callback = ErrorCallback;
    using error_type     = typename _sink_t::return_type;
    static_assert(!std::is_void_v<error_type>, "ErrorCallback must not be a void returning sink");

    constexpr explicit operator bool() const noexcept
    {
        return is_success();
    }

    constexpr bool is_success() const noexcept
    {
        return _status == _status_success;
    }
    constexpr bool is_error() const noexcept
    {
        return !is_success();
    }
    constexpr bool is_recovered_error() const noexcept
    {
        return _status == _status_recovered;
    }
    constexpr bool is_fatal_error() const noexcept
    {
        return _status == _status_fatal;
    }

    constexpr std::size_t error_count() const noexcept
    {
        if constexpr (std::is_same_v<error_type, std::size_t>)
            // void-returning callback yields the count only.
            return _error;
        else
            // We assume it's some sort of container otherwise.
            return _error.size();
    }

    constexpr const auto& errors() const& noexcept
    {
        return _error;
    }
    constexpr auto&& errors() && noexcept
    {
        return LEXY_MOV(_error);
    }

private:
    constexpr explicit validate_result(bool did_recover, error_type&& error)
    : _error(LEXY_MOV(error)), _status()
    {
        if (error_count() == 0u)
            _status = _status_success;
        else if (did_recover)
            _status = _status_recovered;
        else
            _status = _status_fatal;
    }

    error_type _error;
    enum
    {
        _status_success,
        _status_recovered,
        _status_fatal,
    } _status;

    template <typename Input, typename Callback>
    friend class validate_handler;
};
} // namespace lexy

namespace lexy
{
template <typename Input, typename ErrorCallback>
class validate_handler
{
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    constexpr explicit validate_handler(const Input& input, const ErrorCallback& callback)
    : _sink(_get_error_sink(callback)), _input(&input)
    {}

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr auto get_result_value() && noexcept
    {
        return validate_result<ErrorCallback>(true, LEXY_MOV(_sink).finish());
    }
    template <typename Production>
    constexpr auto get_result_empty() && noexcept
    {
        return validate_result<ErrorCallback>(false, LEXY_MOV(_sink).finish());
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {
        iterator position; // beginning of the production
    };

    template <typename Production, typename Iterator>
    constexpr marker<Production> on(parse_events::production_start<Production>, Iterator pos)
    {
        return {pos};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production> m, parse_events::error, Error&& error)
    {
        lexy::error_context err_ctx(Production{}, *_input, m.position);
        _sink(err_ctx, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

private:
    _error_sink_t<ErrorCallback> _sink;
    const Input*                 _input;
};

template <typename Production, typename Input, typename ErrorCallback>
constexpr auto validate(const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = validate_handler(input, callback);
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), reader);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

