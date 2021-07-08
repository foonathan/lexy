// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/failure.hpp>
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
template <typename Token>
using _detect_string = typename Token::string;
template <typename Token>
constexpr bool _can_use_trie = lexy::_detail::is_detected<_detect_string, Token>;

// Just a type_list type.
template <typename... Tokens>
struct _alt_impl
{};

template <typename... Tokens>
struct _alt_trie
{
    using _char_type           = std::common_type_t<typename Tokens::string::char_type...>;
    static constexpr auto trie = lexy::trie<_char_type, typename Tokens::string...>;
};

template <typename Trie, typename Manual, typename... Tokens>
struct _alt_engine;
template <typename... Lits, typename... Tokens>
struct _alt_engine<_alt_impl<Lits...>, _alt_impl<Tokens...>>
{
    struct engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            auto success        = false;
            auto longest_reader = reader;
            auto longest_match  = std::size_t(0);
            auto try_engine     = [&](auto engine) {
                // Match each engine on a fresh reader and determine the length of the match.
                auto copy = reader;
                if (!lexy::engine_try_match<decltype(engine)>(copy))
                    return false;
                auto length = lexy::_detail::range_size(reader.cur(), copy.cur());

                // Update previous maximum.
                if (length > longest_match)
                {
                    longest_match  = length;
                    longest_reader = LEXY_MOV(copy);
                }
                // We've succeeded in either case.
                success = true;

                // We can exit early if we've reached EOF -- there can't be a longer match.
                return copy.eof();
            };

            // Match each rule in some order.
            // We trie the trie first as it is more optimized and gives a longer initial maximum.
            [[maybe_unused]] auto done = false;
            if constexpr (sizeof...(Lits) > 0)
                done = try_engine(lexy::engine_trie<_alt_trie<Lits...>::trie>{});
            if constexpr (sizeof...(Tokens) > 0)
                (done || ... || try_engine(typename Tokens::token_engine{}));

            if (!success)
                return error_code::error;

            reader = LEXY_MOV(longest_reader);
            return error_code();
        }
    };
};
template <typename... Lits, typename... Tokens, typename H, typename... T>
struct _alt_engine<_alt_impl<Lits...>, _alt_impl<Tokens...>, H, T...>
{
    static auto _engine()
    {
        // Insert H into either the trie or the manual version.
        if constexpr (_can_use_trie<H>)
            return
                typename _alt_engine<_alt_impl<Lits..., H>, _alt_impl<Tokens...>, T...>::engine{};
        else
            return
                typename _alt_engine<_alt_impl<Lits...>, _alt_impl<Tokens..., H>, T...>::engine{};
    }
    using engine = decltype(_engine());
};

template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    struct token_engine : _alt_engine<_alt_impl<>, _alt_impl<>, Tokens...>::engine
    {};

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::exhausted_alternatives>(pos);
        context.on(_ev::error{}, err);
    }
};

template <typename R, typename S>
constexpr auto operator/(R, S)
{
    static_assert(lexy::is_token_rule<R> && lexy::is_token_rule<S>);
    return _alt<R, S>{};
}

template <typename... R, typename S>
constexpr auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token_rule<S>);
    return _alt<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token_rule<R>);
    return _alt<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

