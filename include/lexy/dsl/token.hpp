// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/match.hpp>

namespace lexy
{
struct missing_token
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing token";
    }
};
} // namespace lexy

namespace lexyd
{
struct _token_dummy_production
{};

template <typename Rule>
struct _token : token_base<_token<Rule>>
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
            auto                handler = lexy::_match_handler{};
            lexy::parse_context context(_token_dummy_production{}, handler, reader.cur());

            return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader)
                       ? error_code()
                       : error_code::error;
        }
    };

    template <typename Context, typename Reader>
    static constexpr auto token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::missing_token>(pos);
        return LEXY_MOV(context).error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Turns the arbitrary rule into a token by matching it without producing any values.
template <typename Rule>
LEXY_CONSTEVAL auto token(Rule)
{
    if constexpr (lexy::is_token<Rule>)
        return Rule{};
    else
        return _token<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TOKEN_HPP_INCLUDED

