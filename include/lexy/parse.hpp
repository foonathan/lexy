// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_HPP_INCLUDED
#define LEXY_PARSE_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexy
{
template <typename Production, typename Input, typename Callback>
struct _parse_context
{
    const Input&    _input;
    const Callback& _callback;

    using result_type
        = result<typename decltype(Production::value)::return_type, typename Callback::return_type>;

    constexpr bool is_success(const result_type& result)
    {
        return result.has_value();
    }
    constexpr decltype(auto) forward_value(result_type&& result)
    {
        return LEXY_MOV(result).value();
    }
    template <typename Parent>
    constexpr auto forward_error_result(Parent&, result_type&& result)
    {
        // As all contexts use the same callback, the error type is the same.
        // We just need to turn an errored result<T, E> into an errored result<U, E>.
        return typename Parent::result_type(lexy::result_error, LEXY_MOV(result).error());
    }

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _parse_context<SubProduction, Input, Callback>{_input, _callback};
    }

    template <typename Error>
    constexpr result_type report_error(Error&& error)
    {
        if constexpr (std::is_same_v<typename Callback::return_type, void>)
        {
            _callback(Production{}, _input, LEXY_FWD(error));
            return result_type();
        }
        else
        {
            auto cb_result = _callback(Production{}, _input, LEXY_FWD(error));
            return result_type(lexy::result_error, LEXY_MOV(cb_result));
        }
    }

    template <typename... Args>
    static constexpr result_type parse(_parse_context&, Input&, Args&&... args)
    {
        auto value = Production::value(LEXY_FWD(args)...);
        return result_type(lexy::result_value, LEXY_MOV(value));
    }
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename Callback>
constexpr auto parse(Input&& input, Callback&& callback)
{
    using rule      = std::remove_const_t<decltype(Production::rule)>;
    using context_t = _parse_context<Production, std::decay_t<Input>, std::decay_t<Callback>>;

    context_t context{input, callback};
    return rule::template parser<context_t>::parse(context, input);
}
template <typename Production, typename Input>
constexpr auto parse(Input&& input)
{
    return parse<Production>(LEXY_FWD(input), null_callback);
}
} // namespace lexy

#endif // LEXY_PARSE_HPP_INCLUDED

