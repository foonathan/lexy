// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Rule, typename Whitespace>
struct _ws : decltype(loop(token(Whitespace{}) | break_) + Rule{})
{
    static constexpr bool is_branch = lexy::is_branch<Rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Rule, Reader> _impl;

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            using ws = decltype(token(loop(Whitespace{} | break_)));
            lexy::engine_try_match<typename ws::token_engine>(reader);
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            return _impl.template parse<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Tag>
    LEXY_CONSTEVAL auto error() const
    {
        static_assert(lexy::is_token<Rule>);
        return Rule{}.template error<Tag>();
    }
};

/// Matches whitespace before parsing rule.
template <typename Rule, typename Whitespace>
LEXY_CONSTEVAL auto whitespaced(Rule, Whitespace)
{
    return _ws<Rule, Whitespace>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED

