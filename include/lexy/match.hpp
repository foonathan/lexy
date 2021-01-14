// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/production.hpp>
#include <lexy/result.hpp>

namespace lexy
{
struct _match_handler
{
    template <typename Production>
    using result_type_for = lexy::result<void, void>;

    template <typename Production>
    constexpr auto get_sink(Production)
    {
        return noop.sink();
    }

    struct state
    {};

    template <typename Production, typename Iterator>
    constexpr state start_production(Production, Iterator)
    {
        return {};
    }

    template <typename Kind, typename Iterator>
    constexpr void token(Kind, Iterator, Iterator)
    {}

    template <typename Production, typename... Args>
    constexpr auto finish_production(Production, state, Args&&...)
    {
        return result_type_for<Production>(lexy::result_value);
    }

    template <typename Production, typename Error>
    constexpr auto error(Production, state, Error&&)
    {
        return result_type_for<Production>(lexy::result_error);
    }
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto                handler = _match_handler{};
    auto                reader  = input.reader();
    lexy::parse_context context(Production{}, handler, reader.cur());

    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, lexy::context_value_parser>::parse(context, reader).has_value();
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

