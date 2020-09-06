// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/result.hpp>

namespace lexy
{
struct _match_context
{
    using result_type = lexy::result<void, void>;

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _match_context();
    }

    template <typename Input, typename Error>
    constexpr auto report_error(const Input&, Error&&)
    {
        return result_type();
    }

    template <typename Input, typename... Args>
    static constexpr auto parse(_match_context&, Input&, Args&&...)
    {
        return result_type(result_value);
    }
};

template <typename Input, typename Rule, typename = std::enable_if_t<is_rule<Rule>>>
LEXY_FORCE_INLINE constexpr bool match(Input&& input, Rule)
{
    if constexpr (is_pattern<Rule>)
        return Rule::matcher::match(input);
    else
    {
        _match_context context;
        return !!Rule::template parser<_match_context>::parse(context, input);
    }
}

template <typename Production, typename Input>
constexpr bool match(Input&& input)
{
    return match(input, Production::rule);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

