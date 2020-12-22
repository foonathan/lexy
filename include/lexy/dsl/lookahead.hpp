// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/engine/lookahead.hpp>

namespace lexyd
{
template <typename Needle, typename End>
struct _look : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            using engine = lexy::engine_lookahead<Needle, End>;
            return engine::match(reader) == typename engine::error_code();
        }

        template <typename NextParser, typename Handler, typename... Args>
        constexpr auto parse(Handler& handler, Reader& reader, Args&&... args)
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Looks for the Needle pattern before End pattern.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
LEXY_CONSTEVAL auto lookahead(Needle, End)
{
    static_assert(lexy::is_token<Needle> && lexy::is_token<End>);
    return _look<typename Needle::token_engine, typename End::token_engine>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

