// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED

#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/literal.hpp>

namespace lexyd
{
template <typename String>
struct _lit : token_base<_lit<String>>
{
    using string = String;

    static constexpr auto _trie = lexy::linear_trie<String>;

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr bool try_parse(Reader reader)
        {
            auto result = lexy::engine_literal<_trie>::match(reader);
            end         = reader.position();
            return result == typename lexy::engine_literal<_trie>::error_code{};
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using reader_char_type = typename Reader::encoding::char_type;
            constexpr auto string  = String::template get<reader_char_type>();

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, this->end);
            auto err   = lexy::error<Reader, lexy::expected_literal>(begin, string.c_str(), index);
            context.on(_ev::error{}, err);
        }
    };
};

template <auto C>
constexpr auto lit_c = _lit<lexy::_detail::type_char<C>>{};

#if LEXY_HAS_NTTP
/// Matches the literal string.
template <lexy::_detail::string_literal Str>
constexpr auto lit = _lit<lexy::_detail::type_string<Str>>{};
#endif

#define LEXY_LIT(Str)                                                                              \
    ::lexyd::_lit<LEXY_NTTP_STRING(Str)> {}
} // namespace lexyd

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED

