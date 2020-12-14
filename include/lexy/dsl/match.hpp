// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MATCH_HPP_INCLUDED
#define LEXY_DSL_MATCH_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/match.hpp>

namespace lexy
{
struct no_match
{
    static LEXY_CONSTEVAL auto name()
    {
        return "no match";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Rule>
struct _match : token_base<_match<Rule>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            lexy::_match_handler handler{};
            if (Rule::template parser<lexy::final_parser>::parse(handler, reader).has_value())
                return error_code();
            else
                return error_code::error;
        }
    };

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::no_match>(pos);
        return LEXY_MOV(handler).error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Turns the arbitrary rule into a pattern by matching it without producing any values.
template <typename Rule>
LEXY_CONSTEVAL auto match(Rule)
{
    return _match<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MATCH_HPP_INCLUDED

