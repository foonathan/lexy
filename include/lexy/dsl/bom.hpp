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
/// The BOM for that particular encoding.
/// Requires that there is a BOM for this encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = success;

template <lexy::encoding_endianness Endianness>
inline constexpr auto bom<lexy::utf8_encoding, Endianness> = LEXY_LIT("\xEF\xBB\xBF");

template <>
inline constexpr auto bom<lexy::utf16_encoding, lexy::encoding_endianness::little> = LEXY_LIT(
    "\xFF\xFE");

template <>
inline constexpr auto bom<lexy::utf16_encoding, lexy::encoding_endianness::big> = LEXY_LIT(
    "\xFE\xFF");

template <>
inline constexpr auto bom<lexy::utf32_encoding, lexy::encoding_endianness::little> = LEXY_LIT(
    "\xFF\xFE\x00\x00");

template <>
inline constexpr auto bom<lexy::utf32_encoding, lexy::encoding_endianness::big> = LEXY_LIT(
    "\x00\x00\xFE\xFF");
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

