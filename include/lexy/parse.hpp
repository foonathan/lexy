// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_HPP_INCLUDED
#define LEXY_PARSE_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexyd
{
struct _state;
} // namespace lexyd

namespace lexy
{
template <typename Production>
using _production_value = decltype(Production::value);

struct _no_parse_state
{};

template <typename Production, typename State, typename Callback>
class _parse_context
{
    static_assert(std::is_lvalue_reference_v<State>);

public:
    constexpr explicit _parse_context(State state, Callback callback) noexcept
    : _state(state), _callback(callback)
    {}

    _parse_context(const _parse_context&) = delete;
    _parse_context& operator=(const _parse_context&) = delete;

    static auto _result_value_cb()
    {
        if constexpr (_detail::is_detected<_production_value, Production>)
            return Production::value;
        else
            // If we don't have a Production::value callback, we must have only the list.
            // Then the list return type determines value.
            return Production::list;
    }
    using result_type = result<typename decltype(_result_value_cb())::return_type,
                               typename Callback::return_type>;

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _parse_context<SubProduction, State, Callback>(_state, _callback);
    }

    constexpr auto list_sink()
    {
        return Production::list.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader& reader, Error&& error) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_error, _callback, Production{},
                                                   reader, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr auto value(Args&&... args) &&
    {
        if constexpr (!_detail::is_detected<_production_value, Production> && sizeof...(Args) == 1)
            // We don't have a value callback and only a single argument.
            // This means the result of the list builder (which we must have), is our result.
            return result_type(lexy::result_value, LEXY_FWD(args)...);
        else
            return lexy::invoke_as_result<result_type>(lexy::result_value, Production::value,
                                                       LEXY_FWD(args)...);
    }

private:
    // If we don't have a state, don't store a reference.
    using _state_storage_type
        = std::conditional_t<std::is_same_v<State, _no_parse_state&>, _no_parse_state, State>;

    LEXY_EMPTY_MEMBER _state_storage_type _state;
    LEXY_EMPTY_MEMBER Callback            _callback;

    friend lexyd::_state;
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename State, typename Callback>
constexpr auto parse(const Input& input, State&& state, Callback callback)
{
    using rule = std::remove_const_t<decltype(Production::rule)>;
    // We make state an lvalue reference.
    using context_t = _parse_context<Production, State&, Callback>;

    auto reader = input.reader();

    context_t context(state, callback);
    return rule::template parser<final_parser>::parse(context, reader);
}

template <typename Production, typename Input, typename Callback>
constexpr auto parse(const Input& input, Callback callback)
{
    return parse<Production>(input, _no_parse_state{}, callback);
}
} // namespace lexy

namespace lexyd
{
struct _state : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Production, typename State, typename Callback, typename Reader,
                  typename... Args>
        LEXY_DSL_FUNC auto parse(lexy::_parse_context<Production, State, Callback>& context,
                                 Reader& reader, Args&&... args) ->
            typename lexy::_parse_context<Production, State, Callback>::result_type
        {
            static_assert(!std::is_same_v<State, lexy::_no_parse_state&>,
                          "lexy::dsl::state requires passing a state to lexy::parse()");
            return NextParser::parse(context, reader, LEXY_FWD(args)..., context._state);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            // Not used with parse, ignore.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Produces the parsing state passed to `parse()` as a value.
constexpr auto parse_state = _state{};
} // namespace lexyd

#endif // LEXY_PARSE_HPP_INCLUDED
