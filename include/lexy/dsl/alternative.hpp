// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
struct exhausted_alternatives
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted alternatives";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if ((lexy::engine_try_match<typename Tokens::token_engine>(reader) || ...))
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
        auto err = lexy::make_error<Reader, lexy::exhausted_alternatives>(pos);
        return LEXY_MOV(handler).error(err);
    }
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator/(R, S)
{
    static_assert(lexy::is_token<R>);
    static_assert(lexy::is_token<S>);
    return _alt<R, S>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token<S>);
    return _alt<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token<R>);
    return _alt<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
