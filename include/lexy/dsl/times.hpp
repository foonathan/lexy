// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TIMES_HPP_INCLUDED
#define LEXY_DSL_TIMES_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/separator.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexy
{
template <std::size_t N, typename T>
using times = T (&)[N];

template <typename T>
using twice = times<2, T>;
} // namespace lexy

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

    // We only use this template if our rule does not have a matcher.

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Context, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args,
                                     RuleArgs&&... rule_args)
            {
                // Create an array containing the rule arguments.
                static_assert(N == sizeof...(RuleArgs), "rule must create exactly one value");
                using array_type    = std::common_type_t<std::decay_t<RuleArgs>...>;
                array_type array[N] = {LEXY_FWD(rule_args)...};
                return NextParser::parse(context, reader, LEXY_FWD(args)..., array);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the rule with the special continuation that converts the value into an array
            // afterwards.
            using rule         = decltype(_repeated_rule());
            using continuation = _continuation<Args...>;
            return lexy::rule_parser<rule, continuation>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeats the rule N times and collects the values into an array.
template <std::size_t N, typename Rule>
constexpr auto times(Rule)
{
    static_assert(N > 0);
    return _times<N, Rule, void>{};
}

/// Repeates the rule N times separated by the separator and collects the values into an array.
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

