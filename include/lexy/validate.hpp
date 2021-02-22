// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/production.hpp>
#include <lexy/result.hpp>

namespace lexy
{
template <typename Input, typename Callback>
struct _validate_handler
{
    lexy::result<void, typename Callback::return_type> _error;
    const Input*                                       _input;
    LEXY_EMPTY_MEMBER Callback                         _callback;

    template <typename Production>
    using return_type_for = void;

    template <typename Production>
    constexpr auto get_sink(Production)
    {
        return noop.sink();
    }

    template <typename Production, typename Iterator>
    constexpr auto start_production(Production, Iterator pos)
    {
        return pos;
    }

    template <typename Kind, typename Iterator>
    constexpr void token(Kind, Iterator, Iterator)
    {}

    template <typename Production, typename Iterator, typename... Args>
    constexpr void finish_production(Production, Iterator, Args&&...)
    {}

    template <typename Production, typename Iterator, typename Error>
    constexpr void error(Production p, Iterator pos, Error&& error)
    {
        lexy::error_context err_ctx(p, *_input, pos);
        _error = lexy::invoke_as_result<decltype(_error)>(lexy::result_error, _callback, err_ctx,
                                                          LEXY_FWD(error));
    }
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(const Input& input, Callback callback)
    -> lexy::result<void, typename Callback::return_type>
{
    auto handler
        = _validate_handler<Input, Callback>{lexy::result_empty, &input, LEXY_MOV(callback)};
    auto                reader = input.reader();
    lexy::parse_context context(Production{}, handler, reader.cur());

    using rule = lexy::production_rule<Production>;
    if (lexy::rule_parser<rule, lexy::context_value_parser>::parse(context, reader))
        return lexy::result_value;
    else
        return LEXY_MOV(handler._error);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

