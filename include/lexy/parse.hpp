// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_HPP_INCLUDED
#define LEXY_PARSE_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexy
{
template <typename Production>
using _production_value = decltype(Production::value);

template <typename Production, typename Callback>
struct _parse_context
{
    LEXY_EMPTY_MEMBER Callback _callback;

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
        return _parse_context<SubProduction, Callback>{_callback};
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
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename Callback>
constexpr auto parse(const Input& input, Callback callback)
{
    using rule      = std::remove_const_t<decltype(Production::rule)>;
    using context_t = _parse_context<Production, Callback>;

    auto reader = input.reader();

    context_t context{callback};
    return rule::template parser<final_parser>::parse(context, reader);
}
template <typename Production, typename Input>
constexpr auto parse(const Input& input)
{
    return parse<Production>(LEXY_FWD(input), noop);
}
} // namespace lexy

#endif // LEXY_PARSE_HPP_INCLUDED
