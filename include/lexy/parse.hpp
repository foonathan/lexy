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
template <typename Production, typename Callback>
struct _parse_context
{
    const Callback& _callback;

    using result_type
        = result<typename decltype(Production::value)::return_type, typename Callback::return_type>;

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _parse_context<SubProduction, Callback>{_callback};
    }

    template <typename Input, typename Error>
    constexpr auto error(const Input& input, Error&& error) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_error, _callback, Production{},
                                                   input, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr auto value(Args&&... args) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_value, Production::value,
                                                   LEXY_FWD(args)...);
    }
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename Callback>
constexpr auto parse(Input&& input, Callback&& callback)
{
    using rule      = std::remove_const_t<decltype(Production::rule)>;
    using context_t = _parse_context<Production, std::decay_t<Callback>>;

    context_t context{callback};
    return rule::template parser<final_parser>::parse(context, input);
}
template <typename Production, typename Input>
constexpr auto parse(Input&& input)
{
    return parse<Production>(LEXY_FWD(input), noop);
}
} // namespace lexy

#endif // LEXY_PARSE_HPP_INCLUDED
