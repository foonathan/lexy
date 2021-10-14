// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_MATCH_HPP_INCLUDED
#define LEXY_ACTION_MATCH_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/callback/noop.hpp>

namespace lexy
{
class match_handler
{
public:
    constexpr match_handler() : _failed(false) {}

    //=== events ===//
    template <typename Production>
    struct marker
    {
        constexpr void get_value() && {}
    };

    template <typename Production>
    constexpr bool get_action_result(bool parse_result, marker<Production>&&) &&
    {
        return parse_result && !_failed;
    }

    template <typename Production, typename Iterator>
    constexpr marker<Production> on(parse_events::production_start<Production>, Iterator)
    {
        return {};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production>, parse_events::error, Error&&)
    {
        _failed = true;
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

private:
    bool _failed;
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto reader = input.reader();
    return lexy::do_action<Production>(match_handler(), reader);
}
} // namespace lexy

#endif // LEXY_ACTION_MATCH_HPP_INCLUDED

