// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom_impl
{
    static constexpr auto value  = nullptr;
    static constexpr auto length = 0u;
};
template <lexy::encoding_endianness DontCare>
struct _bom_impl<lexy::utf8_encoding, DontCare>
{
    static constexpr unsigned char value[] = {0xEF, 0xBB, 0xBF};
    static constexpr auto          length  = 3u;
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::little>
{
    static constexpr unsigned char value[] = {0xFF, 0xFE};
    static constexpr auto          length  = 2u;
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::big>
{
    static constexpr unsigned char value[] = {0xFE, 0xFF};
    static constexpr auto          length  = 2u;
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::little>
{
    static constexpr unsigned char value[] = {0xFF, 0xFE, 0x00, 0x00};
    static constexpr auto          length  = 4u;
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::big>
{
    static constexpr unsigned char value[] = {0x00, 0x00, 0xFE, 0xFF};
    static constexpr auto          length  = 4u;
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom : _lit<_bom<Encoding, Endianness>>
{
    using char_type = unsigned char;

    static LEXY_CONSTEVAL auto get()
    {
        using impl = _bom_impl<Encoding, Endianness>;
        return lexy::_detail::basic_string_view<unsigned char>(impl::value, impl::length);
    }
};

/// The BOM for that particular encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = _bom<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

