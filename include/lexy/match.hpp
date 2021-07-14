// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/callback/noop.hpp>

namespace lexy
{
class match_handler
{
public:
    constexpr match_handler() : _failed(false) {}

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr bool get_result_value() && noexcept
    {
        // Parsing succeeded or parsing recovered from an error.
        // Return true, if we had an error, false otherwise.
        return !_failed;
    }
    template <typename Production>
    constexpr bool get_result_empty() && noexcept
    {
        // Parsing could not recover from an error, return false.
        LEXY_ASSERT(_failed, "parsing failed without logging an error?!");
        return false;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {};

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
    return lexy::do_action<Production>(match_handler{}, reader);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

