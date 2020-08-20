// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
struct _match_context
{
    using result_type = bool;

    constexpr bool is_success(bool v)
    {
        return v;
    }
    constexpr bool forward_value(bool v)
    {
        return v;
    }
    constexpr bool forward_result(_match_context&, bool v)
    {
        return v;
    }

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return _match_context();
    }

    template <typename Error>
    constexpr bool report_error(Error&&)
    {
        return false;
    }

    template <typename Input, typename... Args>
    static constexpr bool parse(_match_context&, Input&, Args&&...)
    {
        return true;
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
        return Rule::template parser<_match_context>::parse(context, input);
    }
}

template <typename Production, typename Input>
constexpr bool match(Input&& input)
{
    return match(input, Production().rule());
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED

