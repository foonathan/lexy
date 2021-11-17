// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_MATCH_HPP_INCLUDED
#define LEXY_ACTION_MATCH_HPP_INCLUDED

#include <lexy/action/base.hpp>

namespace lexy
{
class match_handler
{
public:
    constexpr match_handler() : _failed(false) {}

    template <typename Production>
    class event_handler
    {
    public:
        template <typename Error>
        constexpr void on(match_handler& handler, parse_events::error, Error&&)
        {
            handler._failed = true;
        }

        template <typename Event, typename... Args>
        constexpr void on(match_handler&, Event, const Args&...)
        {}
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr bool get_result_void(bool rule_parse_result) &&
    {
        return rule_parse_result && !_failed;
    }

private:
    bool _failed;
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto reader = input.reader();
    return lexy::do_action<Production>(match_handler(), no_parse_state, reader);
}
} // namespace lexy

#endif // LEXY_ACTION_MATCH_HPP_INCLUDED

