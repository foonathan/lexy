// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
template <typename Production, typename Input, typename Callback>
struct _validate_context
{
    const Input& _input;
    Callback&    _callback;

    using result_type = bool;

    constexpr bool success(bool v)
    {
        return v;
    }
    constexpr bool forward_value(bool v)
    {
        return v;
    }
    constexpr bool forward_error(bool v)
    {
        return v;
    }

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _validate_context<SubProduction, Input, Callback>{_input, _callback};
    }

    template <typename Error>
    constexpr bool report_error(Error&& error)
    {
        _callback(Production{}, _input, LEXY_FWD(error));
        return false;
    }

    template <typename... Args>
    static constexpr bool parse(_validate_context&, Input&, Args&&...)
    {
        return true;
    }
};

template <typename Production, typename Input, typename Callback>
constexpr bool validate(Input&& input, Callback&& callback)
{
    using rule      = decltype(Production().rule());
    using context_t = _validate_context<Production, std::decay_t<Input>, std::decay_t<Callback>>;

    context_t context{input, callback};
    return rule::template parser<context_t>::parse(context, input);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

