// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexy
{
struct nullopt
{
    template <typename T, typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    constexpr operator T() const
    {
        return T();
    }
};
} // namespace lexy

namespace lexyd
{
struct _nullopt : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

constexpr auto nullopt = _nullopt{};
} // namespace lexyd

namespace lexyd
{
template <typename Condition, typename Then>
struct _opt : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (auto result = Condition::matcher::match(reader))
                return Then::template parser<NextParser>::parse(handler, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(handler, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "opt() requires a branch condition");

    auto as_branch = branch(rule);
    return _opt<decltype(as_branch.condition()), decltype(as_branch.then())>{};
}
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED
