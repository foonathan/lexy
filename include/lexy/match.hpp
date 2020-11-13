// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/callback.hpp>
#include <lexy/result.hpp>

namespace lexy
{
struct _match_handler
{
    using result_type = lexy::result<void, void>;

    template <typename SubProduction, typename Reader>
    constexpr auto sub_handler(const Reader&)
    {
        return _match_handler();
    }

    constexpr auto list_sink()
    {
        return noop.sink();
    }

    template <typename Reader, typename Error>
    constexpr auto error(const Reader&, Error&&) &&
    {
        return result_type();
    }

    template <typename... Args>
    constexpr auto value(Args&&...) &&
    {
        return result_type(result_value);
    }
};

template <typename Input, typename Rule, typename = std::enable_if_t<is_rule<Rule>>>
LEXY_FORCE_INLINE constexpr bool match(const Input& input, Rule)
{
    auto reader = input.reader();

    _match_handler handler;
    return !!Rule::template parser<final_parser>::parse(handler, reader);
}

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    return match(input, Production::rule);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

