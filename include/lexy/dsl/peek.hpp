// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PEEK_HPP_INCLUDED
#define LEXY_DSL_PEEK_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Engine, bool Expected>
struct _peek : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser // propagate parse
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (lexy::engine_peek<Engine>(reader) != Expected)
                return lexy::rule_try_parse_result::backtracked;

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)...));
        }
    };

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
LEXY_CONSTEVAL auto peek(Rule rule)
{
    using token = decltype(token(rule));
    return _peek<typename token::token_engine, true>{};
}

/// Check if at this reader position, the rule would not match, but don't actually consume any
/// characters if it does.
template <typename Rule>
LEXY_CONSTEVAL auto peek_not(Rule rule)
{
    using token = decltype(token(rule));
    return _peek<typename token::token_engine, false>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

