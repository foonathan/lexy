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
    const Input&    _input;
    const Callback& _callback;

    using result_type = optional_error<typename Callback::return_type>;

    constexpr bool is_success(const result_type& result)
    {
        return result.has_value();
    }
    constexpr lexy::result_value_t forward_value(result_type&& result)
    {
        return result.value();
    }
    template <typename Parent>
    constexpr result_type&& forward_result(Parent&, result_type&& result)
    {
        return LEXY_MOV(result);
    }

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _validate_context<SubProduction, Input, Callback>{_input, _callback};
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
    static constexpr result_type parse(_validate_context&, Input&, Args&&...)
    {
        return result_type(lexy::result_value);
    }
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(Input&& input, Callback&& callback)
{
    using rule      = decltype(Production().rule());
    using context_t = _validate_context<Production, std::decay_t<Input>, std::decay_t<Callback>>;

    context_t context{input, callback};
    return rule::template parser<context_t>::parse(context, input);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

