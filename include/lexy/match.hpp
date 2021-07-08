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

    constexpr explicit operator bool() const noexcept
    {
        return !_failed;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {};

    template <typename Production>
    using production_result = void;

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
    auto handler = match_handler{};
    auto reader  = input.reader();

    lexy::_detail::action_impl<Production>(handler, reader);

    // We only match the production if no error was logged.
    return static_cast<bool>(handler);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

