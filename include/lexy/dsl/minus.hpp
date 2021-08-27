// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MINUS_HPP_INCLUDED
#define LEXY_DSL_MINUS_HPP_INCLUDED

#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/minus.hpp>

namespace lexy
{
/// We've matched the Except of a minus.
struct minus_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "minus failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Token, typename Except>
struct _minus : token_base<_minus<Token, Except>>
{
    using token_engine
        = lexy::engine_minus<typename Token::token_engine, typename Except::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::minus_failure)
        {
            auto err = lexy::error<Reader, lexy::minus_failure>(pos, reader.position());
            context.on(_ev::error{}, err);
        }
        else
        {
            Token::token_error(context, reader, token_engine::error_to_matcher(ec), pos);
        }
    }
};

/// Matches Token unless Except matches on the input Token matched.
template <typename Token, typename Except>
constexpr auto operator-(Token, Except)
{
    static_assert(lexy::is_token_rule<Token>);
    static_assert(lexy::is_token_rule<Except>);
    return _minus<Token, Except>{};
}
template <typename Token, typename E, typename Except>
constexpr auto operator-(_minus<Token, E>, Except except)
{
    static_assert(lexy::is_token_rule<Except>);
    return _minus<Token, decltype(E{} / except)>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED

