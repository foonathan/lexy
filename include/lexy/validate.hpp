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
    const Input*               _input;
    LEXY_EMPTY_MEMBER Callback _callback;

    template <typename Production>
    using result_type_for = lexy::result<void, typename Callback::return_type>;

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
    constexpr auto finish_production(Production, Iterator, Args&&...)
    {
        return result_type_for<Production>(lexy::result_value);
    }

    template <typename Production, typename Iterator, typename Error>
    constexpr auto error(Production p, Iterator pos, Error&& error)
    {
        lexy::error_context err_ctx(p, *_input, pos);
        return lexy::invoke_as_result<result_type_for<Production>>(lexy::result_error, _callback,
                                                                   err_ctx, LEXY_FWD(error));
    }
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(const Input& input, Callback callback)
{
    auto                handler = _validate_handler<Input, Callback>{&input, LEXY_MOV(callback)};
    auto                reader  = input.reader();
    lexy::parse_context context(Production{}, handler, reader.cur());

    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, lexy::context_value_parser>::parse(context, reader);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

