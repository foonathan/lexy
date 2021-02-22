// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/trie.hpp>

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
// TODO: _trie does not really require the same order; unlike _alt.
// This means that using _trie over _alt is a behavior change, as it might accept more input.
// However, all cases are probably bugs?
template <typename... Strings>
struct _trie : token_base<_trie<Strings...>>
{
    using _char_type            = std::common_type_t<typename Strings::char_type...>;
    static constexpr auto _impl = lexy::trie<_char_type, Strings...>;
    using token_engine          = lexy::engine_trie<_impl>;

    template <typename Context, typename Reader>
    static constexpr auto token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::exhausted_alternatives>(pos);
        return LEXY_MOV(context).error(err);
    }
};

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

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::exhausted_alternatives>(pos);
        context.error(err);
    }
};

template <typename Token>
using _detect_string = typename Token::string;
template <typename Token>
constexpr bool _can_use_trie = lexy::_detail::is_detected<_detect_string, Token>;

template <typename R, typename S>
LEXY_CONSTEVAL auto operator/(R, S)
{
    static_assert(lexy::is_token<R> && lexy::is_token<S>);
    if constexpr (_can_use_trie<R> && _can_use_trie<S>)
        return _trie<typename R::string, typename S::string>{};
    else
        return _alt<R, S>{};
}

template <typename... R, typename S>
LEXY_CONSTEVAL auto operator/(_trie<R...>, S)
{
    static_assert(lexy::is_token<S>);
    if constexpr (_can_use_trie<S>)
        return _trie<R..., typename S::string>{};
    else
        return _alt<_trie<R...>, S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator/(R, _trie<S...>)
{
    static_assert(lexy::is_token<R>);
    if constexpr (_can_use_trie<R>)
        return _trie<typename R::string, S...>{};
    else
        return _alt<R, _trie<S...>>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator/(_trie<R...>, _trie<S...>)
{
    return _trie<R..., S...>{};
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

