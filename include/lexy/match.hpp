// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/callback/noop.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/production.hpp>

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

    //=== handler functions ===//
    template <typename Production>
    using return_type_for = void;

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
    constexpr void finish_production(Production, state, Args&&...)
    {}
    template <typename Production>
    constexpr void backtrack_production(Production, state)
    {}

    template <typename Production, typename Error>
    constexpr void error(Production, state, Error&&)
    {
        _failed = true;
    }

private:
    bool _failed;
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto handler = match_handler{};
    auto reader  = input.reader();

    lexy::_detail::parse_impl<Production>(handler, reader);

    // We only match the production if no error was logged.
    return static_cast<bool>(handler);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

