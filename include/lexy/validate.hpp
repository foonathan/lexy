// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexy
{
template <typename Production, typename Input, typename Callback>
struct _validate_context
{
    const Input&               _input;
    LEXY_EMPTY_MEMBER Callback _callback;

    using result_type = optional_error<typename Callback::return_type>;

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _validate_context<SubProduction, Input, Callback>{_input, _callback};
    }

    constexpr auto list_sink()
    {
        return noop.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader&, Error&& error) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_error, _callback, Production{},
                                                   _input, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr auto value(Args&&...) &&
    {
        return result_type(lexy::result_value);
    }
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(const Input& input, Callback callback)
{
    using rule      = std::remove_const_t<decltype(Production::rule)>;
    using context_t = _validate_context<Production, Input, Callback>;

    auto reader = input.reader();

    context_t context{input, callback};
    return rule::template parser<final_parser>::parse(context, reader);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

