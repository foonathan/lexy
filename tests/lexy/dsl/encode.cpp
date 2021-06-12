// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/encode.hpp>

#include "verify.hpp"

namespace
{
struct callback
{
    const unsigned char* str;

    LEXY_VERIFY_FN int success(const unsigned char* cur)
    {
        return int(cur - str);
    }

    template <typename Reader>
    LEXY_VERIFY_FN int error(lexy::error<Reader, lexy::expected_literal>)
    {
        return -1;
    }
};

} // namespace

TEST_CASE("dsl::encoded")
{
    SUBCASE("ASCII")
    {
        static constexpr unsigned char input[] = {'a', 'b', 'c'};
        static constexpr auto rule = lexy::dsl::encode<lexy::ascii_encoding>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto abc = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 3);
        CHECK(abc == 3);
    }
    SUBCASE("UTF-8")
    {
        static constexpr unsigned char input[] = {0xEF, 0xBB, 0xBF, 'a', 'b', 'c'};
        static constexpr auto rule = lexy::dsl::encode<lexy::utf8_encoding>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input + 3, 3);
        CHECK(no_bom == 3);
        auto bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 6);
        CHECK(bom == 6);
    }
    SUBCASE("UTF-16, little")
    {
        static constexpr unsigned char input[] = {0xBB, 0xAA};

        static constexpr auto encode
            = lexy::dsl::encode<lexy::utf16_encoding, lexy::encoding_endianness::little>;
        static constexpr auto rule = encode(LEXY_LIT(u"\uAABB"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 2);
        CHECK(no_bom == 2);
    }
    SUBCASE("UTF-16, big")
    {
        static constexpr unsigned char input[] = {0xAA, 0xBB};

        static constexpr auto encode
            = lexy::dsl::encode<lexy::utf16_encoding, lexy::encoding_endianness::big>;
        static constexpr auto rule = encode(LEXY_LIT(u"\uAABB"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 2);
        CHECK(no_bom == 2);
    }
    SUBCASE("UTF-16, bom")
    {
        static constexpr unsigned char input[]        = {0xAA, 0xBB};
        static constexpr unsigned char input_little[] = {0xFF, 0xFE, 0xBB, 0xAA};
        static constexpr unsigned char input_big[]    = {0xFE, 0xFF, 0xAA, 0xBB};

        static constexpr auto encode
            = lexy::dsl::encode<lexy::utf16_encoding, lexy::encoding_endianness::bom>;
        static constexpr auto rule = encode(LEXY_LIT(u"\uAABB"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 2);
        CHECK(no_bom == 2);

        auto bom_little = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input_little, 4);
        CHECK(bom_little == 4);
        auto bom_big = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input_big, 4);
        CHECK(bom_big == 4);
    }
    SUBCASE("UTF-32, little")
    {
        static constexpr unsigned char input[] = {0xBB, 0xAA, 0x00, 0x00};

        static constexpr auto encode
            = lexy::dsl::encode<lexy::utf32_encoding, lexy::encoding_endianness::little>;
        static constexpr auto rule = encode(LEXY_LIT(U"\uAABB"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 4);
        CHECK(no_bom == 4);
    }
    SUBCASE("UTF-32, big")
    {
        static constexpr unsigned char input[] = {0x00, 0x00, 0xAA, 0xBB};

        static constexpr auto encode
            = lexy::dsl::encode<lexy::utf32_encoding, lexy::encoding_endianness::big>;
        static constexpr auto rule = encode(LEXY_LIT(U"\uAABB"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 4);
        CHECK(no_bom == 4);
    }
    SUBCASE("UTF-32, bom")
    {
        static constexpr unsigned char input[] = {0x00, 0x00, 0xAA, 0xBB};
        static constexpr unsigned char input_little[]
            = {0xFF, 0xFE, 0x00, 0x00, 0xBB, 0xAA, 0x00, 0x00};
        static constexpr unsigned char input_big[]
            = {0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xAA, 0xBB};

        static constexpr auto encode
            = lexy::dsl::encode<lexy::utf32_encoding, lexy::encoding_endianness::bom>;
        static constexpr auto rule = encode(LEXY_LIT(U"\uAABB"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 0);
        CHECK(empty == -1);
        auto no_bom = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input, 4);
        CHECK(no_bom == 4);

        auto bom_little = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input_little, 8);
        CHECK(bom_little == 8);
        auto bom_big = LEXY_VERIFY_ENCODING(lexy::byte_encoding, input_big, 8);
        CHECK(bom_big == 8);
    }
}

