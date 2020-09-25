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
template <typename Production, typename Callback>
struct _validate_context
{
    const Callback& _callback;

    using result_type = optional_error<typename Callback::return_type>;

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _validate_context<SubProduction, Callback>{_callback};
    }

    constexpr auto list_sink()
    {
        return noop.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader& reader, Error&& error) &&
    {
        return lexy::invoke_as_result<result_type>(lexy::result_error, _callback, Production{},
                                                   reader, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr auto value(Args&&...) &&
    {
        return result_type(lexy::result_value);
    }
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(const Input& input, Callback&& callback)
{
    using rule      = std::remove_const_t<decltype(Production::rule)>;
    using context_t = _validate_context<Production, std::decay_t<Callback>>;

    auto reader = input.reader();

    context_t context{callback};
    return rule::template parser<final_parser>::parse(context, reader);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

