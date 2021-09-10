// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

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

template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    template <typename Reader,
              typename Indices = lexy::_detail::make_index_sequence<sizeof...(Tokens)>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr bool try_parse(const Reader& reader)
        {
            auto result = false;
            auto impl   = [&](auto token, Reader local_reader) {
                // Try to match the current token.
                if (!lexy::try_match_token(token, local_reader))
                    return;

                // Update end to longest match.
                end    = lexy::_detail::max_range_end(reader.position(), this->end,
                                                   local_reader.position());
                result = true;
            };

            // Need to try everything.
            // TODO: optimize for literals
            end = reader.position();
            (impl(Tokens{}, reader), ...);
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::exhausted_alternatives>(reader.position());
            context.on(_ev::error{}, err);
        }
    };
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

