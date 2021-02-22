// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MINUS_HPP_INCLUDED
#define LEXY_DSL_MINUS_HPP_INCLUDED

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
template <typename Token, typename... Excepts>
struct _minus : token_base<_minus<Token, Excepts...>>
{
    using token_engine
        = lexy::engine_minus<typename Token::token_engine, typename Excepts::token_engine...>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::minus_failure)
        {
            auto err = lexy::make_error<Reader, lexy::minus_failure>(pos, reader.cur());
            context.error(err);
        }
        else
        {
            Token::token_error(context, reader, token_engine::error_to_matcher(ec), pos);
        }
    }
};

/// Matches Token unless Except matches on the input Token matched.
template <typename Token, typename Except>
LEXY_CONSTEVAL auto operator-(Token, Except)
{
    static_assert(lexy::is_token<Token>);
    static_assert(lexy::is_token<Except>);
    return _minus<Token, Except>{};
}
template <typename Token, typename... E, typename Except>
LEXY_CONSTEVAL auto operator-(_minus<Token, E...>, Except)
{
    static_assert(lexy::is_token<Except>);
    return _minus<Token, E..., Except>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED

