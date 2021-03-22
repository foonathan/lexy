// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/engine/find.hpp>

namespace lexyd
{
template <typename Needle, typename End>
struct _look : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser // delegate parse
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using engine = lexy::engine_find_before<Needle, End>;
            if (!lexy::engine_peek<engine>(reader))
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

/// Looks for the Needle before End.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
LEXY_CONSTEVAL auto lookahead(Needle, End)
{
    static_assert(lexy::is_token<Needle> && lexy::is_token<End>);
    return _look<typename Needle::token_engine, typename End::token_engine>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

