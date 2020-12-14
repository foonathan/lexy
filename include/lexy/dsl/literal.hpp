// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/_detail/string_view.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/engine/literal.hpp>

namespace lexyd
{
template <typename String>
struct _lit : token_base<_lit<String>>
{
    template <typename Reader>
    static LEXY_CONSTEVAL bool _string_compatible()
    {
        using encoding = typename Reader::encoding;
        if (lexy::char_type_compatible_with_reader<Reader, typename String::char_type>)
            return true;

        // The string and the input have incompatible character types.
        // We then only allow ASCII characters in the string literal.
        auto str = String::get();
        for (auto c : str)
        {
            auto value = encoding::to_int_type(typename encoding::char_type(c));
            if (value < encoding::to_int_type(0) || value > encoding::to_int_type(0x7F))
                return false;
        }

        return true;
    }

    // TODO: string compatibility check
    static constexpr auto _trie = lexy::linear_trie<String>;
    using token_engine          = lexy::engine_literal<_trie>;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler&                          handler, const Reader&,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        using reader_char_type = typename Reader::encoding::char_type;
        constexpr auto string  = String::template get<reader_char_type>();

        auto err
            = lexy::make_error<Reader, lexy::expected_literal>(pos, string,
                                                               token_engine::index_from_error(ec));
        return LEXY_MOV(handler).error(err);
    }

    //=== dsl ===//
    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
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
