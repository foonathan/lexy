// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom_impl
{
    static_assert(Endianness != lexy::encoding_endianness::bom,
                  "bom with BOM-endianness doesn't make sense");

    static constexpr auto name = "";

    using literal     = _lit<unsigned char>;
    using branch_base = unconditional_branch_base;
};
template <lexy::encoding_endianness DontCare>
struct _bom_impl<lexy::utf8_encoding, DontCare>
{
    static constexpr auto name = "BOM.UTF-8";

    using literal     = decltype(lit_b<0xEF, 0xBB, 0xBF>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-16-LE";

    using literal     = decltype(lit_b<0xFF, 0xFE>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-16-BE";

    using literal     = decltype(lit_b<0xFE, 0xFF>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-32-LE";

    using literal     = decltype(lit_b<0xFF, 0xFE, 0x00, 0x00>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-32-BE";

    using literal     = decltype(lit_b<0x00, 0x00, 0xFE, 0xFF>);
    using branch_base = lexyd::branch_base;
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom
: token_base<_bom<Encoding, Endianness>, typename _bom_impl<Encoding, Endianness>::branch_base>
{
    using _impl = _bom_impl<Encoding, Endianness>;

    template <typename Reader>
    struct tp : lexy::token_parser_for<typename _impl::literal, Reader>
    {
        constexpr explicit tp(const Reader& reader)
        : lexy::token_parser_for<typename _impl::literal, Reader>(reader)
        {}

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), _impl::name);
            context.on(_ev::error{}, err);
        }
    };
};

/// The BOM for that particular encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = _bom<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

