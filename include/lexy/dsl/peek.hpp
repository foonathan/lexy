// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PEEK_HPP_INCLUDED
#define LEXY_DSL_PEEK_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexyd
{
template <typename Pattern>
struct _peek : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            auto copy = reader;
            return Pattern::matcher::match(copy);
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

/// Check if at this reader position, Pattern would match, but don't actually consume any characters
/// if it does.
template <typename Pattern>
LEXY_CONSTEVAL auto peek(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _peek<Pattern>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Pattern>
struct _peekn : branch_base
{
    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader) const
        {
            auto copy = reader;
            return !Pattern::matcher::match(copy);
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

/// Check if at this reader position, Pattern would not match, but don't actually consume any
/// characters if it does.
template <typename Pattern>
LEXY_CONSTEVAL auto peek_not(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _peekn<Pattern>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

