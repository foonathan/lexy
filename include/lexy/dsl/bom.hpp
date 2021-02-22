// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/engine/literal.hpp>

namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom_impl
{
    static_assert(Endianness != lexy::encoding_endianness::bom,
                  "bom with BOM-endianness doesn't make sense");

    static constexpr auto name = "";

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view<unsigned char>();
    }
};
template <lexy::encoding_endianness DontCare>
struct _bom_impl<lexy::utf8_encoding, DontCare>
{
    static constexpr auto name = "BOM.UTF-8";

    static constexpr unsigned char value[] = {0xEF, 0xBB, 0xBF};
    static constexpr auto          length  = 3u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-16-LE";

    static constexpr unsigned char value[] = {0xFF, 0xFE};
    static constexpr auto          length  = 2u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-16-BE";

    static constexpr unsigned char value[] = {0xFE, 0xFF};
    static constexpr auto          length  = 2u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-32-LE";

    static constexpr unsigned char value[] = {0xFF, 0xFE, 0x00, 0x00};
    static constexpr auto          length  = 4u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-32-BE";

    static constexpr unsigned char value[] = {0x00, 0x00, 0xFE, 0xFF};
    static constexpr auto          length  = 4u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom : token_base<_bom<Encoding, Endianness>>
{
    using string                = _bom_impl<Encoding, Endianness>;
    static constexpr auto _trie = lexy::linear_trie<string>;

    using token_engine = lexy::engine_literal<_trie>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, string::name);
        context.error(err);
    }
};

/// The BOM for that particular encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = _bom<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

