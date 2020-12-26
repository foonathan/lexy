// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/encode.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::encoded")
{
    SUBCASE("ASCII")
    {
        constexpr auto        encode = lexy::dsl::encode<lexy::ascii_encoding>;
        static constexpr auto rule   = encode(LEXY_LIT("abc")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        CHECK(lexy::match(lexy::zstring_input<lexy::raw_encoding>("abc"), rule));
    }
    SUBCASE("UTF-8, bom")
    {
        constexpr auto        encode = lexy::dsl::encode<lexy::utf8_encoding>;
        static constexpr auto rule   = encode(LEXY_LIT("abc")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char input[] = {0xEF, 0xBB, 0xBF, 'a', 'b', 'c'};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(input + 3, 3), rule));
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(input, 6), rule));
    }
    SUBCASE("utf16, little")
    {
        constexpr auto encode
            = lexy::dsl::encode<lexy::utf16_encoding, lexy::encoding_endianness::little>;
        static constexpr auto rule = encode(LEXY_LIT(u"\uAABB")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char input[] = {0xBB, 0xAA};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(input, 2), rule));
    }
    SUBCASE("utf16, big")
    {
        constexpr auto encode
            = lexy::dsl::encode<lexy::utf16_encoding, lexy::encoding_endianness::big>;
        static constexpr auto rule = encode(LEXY_LIT(u"\uAABB")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char input[] = {0xAA, 0xBB};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(input, 2), rule));
    }
    SUBCASE("utf16, bom")
    {
        constexpr auto        encode = lexy::dsl::encode<lexy::utf16_encoding>;
        static constexpr auto rule   = encode(LEXY_LIT(u"\uAABB")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char no_bom[] = {0xAA, 0xBB};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(no_bom, 2), rule));
        unsigned char little_bom[] = {0xFF, 0xFE, 0xBB, 0xAA};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(little_bom, 4), rule));
        unsigned char big_bom[] = {0xFE, 0xFF, 0xAA, 0xBB};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(big_bom, 4), rule));
    }
    SUBCASE("utf32, little")
    {
        constexpr auto encode
            = lexy::dsl::encode<lexy::utf32_encoding, lexy::encoding_endianness::little>;
        static constexpr auto rule = encode(LEXY_LIT(U"\uAABB")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char input[] = {0xBB, 0xAA, 0x00, 0x00};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(input, 4), rule));
    }
    SUBCASE("utf32, big")
    {
        constexpr auto encode
            = lexy::dsl::encode<lexy::utf32_encoding, lexy::encoding_endianness::big>;
        static constexpr auto rule = encode(LEXY_LIT(U"\uAABB")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char input[] = {0x00, 0x00, 0xAA, 0xBB};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(input, 4), rule));
    }
    SUBCASE("utf32, bom")
    {
        constexpr auto        encode = lexy::dsl::encode<lexy::utf32_encoding>;
        static constexpr auto rule   = encode(LEXY_LIT(U"\uAABB")) + lexy::dsl::eof;
        CHECK(lexy::is_rule<decltype(rule)>);

        unsigned char no_bom[] = {0x00, 0x00, 0xAA, 0xBB};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(no_bom, 4), rule));
        unsigned char little_bom[] = {0xFF, 0xFE, 0x00, 0x00, 0xBB, 0xAA, 0x00, 0x00};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(little_bom, 8), rule));
        unsigned char big_bom[] = {0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xAA, 0xBB};
        CHECK(lexy::match(lexy::string_input<lexy::raw_encoding>(big_bom, 8), rule));
    }
}

