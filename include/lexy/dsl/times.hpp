// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TIMES_HPP_INCLUDED
#define LEXY_DSL_TIMES_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/separator.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
template <std::size_t N, typename Rule>
constexpr auto _gen_times(Rule rule)
{
    if constexpr (N == 1)
        return rule;
    else
        return rule + _gen_times<N - 1>(rule);
}
template <std::size_t N, typename Rule, typename Sep>
constexpr auto _gen_times(Rule rule, Sep)
{
    if constexpr (N == 1)
        return rule + typename Sep::trailing_rule{};
    else
        return rule + typename Sep::rule{} + _gen_times<N - 1>(rule, Sep{});
}

template <std::size_t N, typename Rule, typename Sep>
struct _times : rule_base
{
    static constexpr auto _repeated_rule()
    {
        if constexpr (std::is_same_v<Sep, void>)
            return _gen_times<N>(Rule{});
        else
            return _gen_times<N>(Rule{}, Sep{});
    }

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using rule = decltype(_repeated_rule());
            return lexy::rule_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeats the rule N times in sequence.
template <std::size_t N, typename Rule>
constexpr auto times(Rule)
{
    static_assert(N > 0);
    return _times<N, Rule, void>{};
}

/// Repeates the rule N times in sequence separated by a separator.
template <std::size_t N, typename Rule, typename Sep>
constexpr auto times(Rule, Sep)
{
    static_assert(N > 0);
    return _times<N, Rule, Sep>{};
}

template <typename Rule>
constexpr auto twice(Rule rule)
{
    return times<2>(rule);
}
template <typename Rule, typename Sep>
constexpr auto twice(Rule rule, Sep sep)
{
    return times<2>(rule, sep);
}
} // namespace lexyd

#endif // LEXY_DSL_TIMES_HPP_INCLUDED

