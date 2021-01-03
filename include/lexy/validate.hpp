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
template <typename Callback>
struct _validate_handler
{
    LEXY_EMPTY_MEMBER Callback _callback;

    explicit constexpr _validate_handler(Callback callback) : _callback(LEXY_MOV(callback)) {}

    template <typename Production>
    using result_type_for = lexy::result<void, typename Callback::return_type>;

    template <typename Production>
    constexpr auto sink(Production)
    {
        return noop.sink();
    }

    template <typename Production, typename Iterator>
    constexpr void start_production(Production, Iterator)
    {}

    template <typename Production, typename... Args>
    constexpr auto finish_production(Production, Args&&...)
    {
        return result_type_for<Production>(lexy::result_value);
    }

    template <typename Production, typename Input, typename Error>
    constexpr auto error(lexy::error_context<Production, Input>&& err_ctx, Error&& error)
    {
        return lexy::invoke_as_result<result_type_for<Production>>(lexy::result_error, _callback,
                                                                   LEXY_FWD(err_ctx),
                                                                   LEXY_FWD(error));
    }
};

template <typename Production, typename Input, typename Callback>
constexpr auto validate(const Input& input, Callback callback)
{
    using context_t = lexy::parse_context<Input, lexy::_validate_handler<Callback>>;
    context_t context(input, LEXY_MOV(callback));

    auto                     reader = input.reader();
    lexy::production_context prod_ctx(context, Production{}, reader.cur());

    using rule = typename lexy::production_traits<Production>::rule::type;
    return lexy::rule_parser<rule, lexy::context_value_parser>::parse(prod_ctx, reader);
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED

