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
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

constexpr auto nullopt = _nullopt{};
} // namespace lexyd

namespace lexyd
{
template <typename Branch>
struct _opt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            lexy::branch_matcher<Branch, Reader> branch{};
            if (branch.match(reader))
                return branch.template parse<NextParser>(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule)
{
    static_assert(lexy::is_branch<Rule>, "opt() requires a branch condition");
    return _opt<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Terminator, typename R>
struct _optt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            lexy::branch_matcher<Terminator, Reader> term{};
            if (term.match(reader))
                return term.template parse<NextParser>(context, reader, LEXY_FWD(args)...,
                                                       lexy::nullopt{});
            else
                // Note: we don't add the terminator.
                // This has to be done by the parent rule, if necessary.
                return lexy::rule_parser<R, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED
