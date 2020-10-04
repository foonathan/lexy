// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_HPP_INCLUDED
#define LEXY_PARSE_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/invoke.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexyd
{
template <auto Fn>
struct _state;
} // namespace lexyd

namespace lexy
{
template <typename Production>
using _production_value = decltype(Production::value);

struct _no_parse_state
{};

template <typename Production, typename State, typename Input, typename Callback>
class _parse_handler
{
    static_assert(std::is_lvalue_reference_v<State>);

public:
    constexpr explicit _parse_handler(State state, const Input& input,
                                      const input_reader<Input>& reader, Callback callback) noexcept
    : _err_ctx(input, reader.cur()), _state(state), _callback(callback)
    {}

    _parse_handler(const _parse_handler&) = delete;
    _parse_handler& operator=(const _parse_handler&) = delete;

    static auto _result_value_cb()
    {
        if constexpr (_detail::is_detected<_production_value, Production>)
            return Production::value;
        else
            // If we don't have a Production::value callback, we must have only the list.
            // Then the list return type determines value.
            return Production::list.sink();
    }
    using result_type = result<typename decltype(_result_value_cb())::return_type,
                               typename Callback::return_type>;

    template <typename SubProduction>
    constexpr auto sub_handler(const input_reader<Input>& reader)
    {
        return _parse_handler<SubProduction, State, Input, Callback>(_state, _err_ctx.input(),
                                                                     reader, _callback);
    }

    constexpr auto list_sink()
    {
        return Production::list.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader&, Error&& error) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_error, _callback, _err_ctx,
                                                   LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr auto value(Args&&... args) &&
    {
        if constexpr (!_detail::is_detected<_production_value, Production>)
        {
            // We don't have a value callback, which means we must have a list callback.
            // Use it to handle the arguments.

            if constexpr (sizeof...(Args) == 0)
                // No arguments, build an empty list.
                return result_type(lexy::result_value, Production::list.sink().finish());
            else if constexpr (sizeof...(Args) == 1)
                // Single argument, return that one.
                return result_type(lexy::result_value, LEXY_FWD(args)...);
            else
                static_assert(_detail::error<Production, Args...>,
                              "missing value callback for Production");
        }
        else
            // Pass the arguments to the value callback.
            return lexy::invoke_as_result<result_type>(lexy::result_value, Production::value,
                                                       LEXY_FWD(args)...);
    }

private:
    // If we don't have a state, don't store a reference.
    using _state_storage_type
        = std::conditional_t<std::is_same_v<State, _no_parse_state&>, _no_parse_state, State>;

    error_context<Production, Input>      _err_ctx;
    LEXY_EMPTY_MEMBER _state_storage_type _state;
    LEXY_EMPTY_MEMBER Callback            _callback;

    template <auto Fn>
    friend struct lexyd::_state;
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename State, typename Callback>
constexpr auto parse(const Input& input, State&& state, Callback callback)
{
    using rule = std::remove_const_t<decltype(Production::rule)>;
    // We make state an lvalue reference.
    using handler_t = _parse_handler<Production, State&, Input, Callback>;

    auto reader = input.reader();

    handler_t handler(state, input, reader, callback);
    return rule::template parser<final_parser>::parse(handler, reader);
}

template <typename Production, typename Input, typename Callback>
constexpr auto parse(const Input& input, Callback callback)
{
    return parse<Production>(input, _no_parse_state{}, callback);
}
} // namespace lexy

namespace lexyd
{
template <auto Fn>
struct _state : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Production, typename State, typename Input, typename Callback,
                  typename... Args>
        LEXY_DSL_FUNC auto parse(lexy::_parse_handler<Production, State, Input, Callback>& handler,
                                 lexy::input_reader<Input>& reader, Args&&... args) ->
            typename lexy::_parse_handler<Production, State, Input, Callback>::result_type
        {
            static_assert(!std::is_same_v<State, lexy::_no_parse_state&>,
                          "lexy::dsl::state requires passing a state to lexy::parse()");

            if constexpr (std::is_same_v<decltype(Fn), decltype(nullptr)>)
                return NextParser::parse(handler, reader, LEXY_FWD(args)..., handler._state);
            else
                return NextParser::parse(handler, reader, LEXY_FWD(args)...,
                                         lexy::_detail::invoke(Fn, handler._state));
        }

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // Not used with parse, ignore.
            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
};

/// Produces the parsing state passed to `parse()` as a value.
constexpr auto parse_state = _state<nullptr>{};

/// Produces a member of the parsing state as a value.
template <auto Fn>
constexpr auto parse_state_member = _state<Fn>{};
} // namespace lexyd

#endif // LEXY_PARSE_HPP_INCLUDED
