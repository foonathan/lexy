// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/bom.hpp>

#include "verify.hpp"

TEST_CASE("dsl::bom")
{
    constexpr auto callback = token_callback;

    SUBCASE("no bom")
    {
        constexpr auto rule = dsl::bom<lexy::byte_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().literal(""));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal(""));
    }

    SUBCASE("UTF-8")
    {
        constexpr auto rule = dsl::bom<lexy::utf8_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "BOM.UTF-8").cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xEF, 0xBB, 0xBF);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(EF BB BF)"));
    }

    SUBCASE("UTF-16, little")
    {
        constexpr auto rule = dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "BOM.UTF-16-LE").cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xFF, 0xFE);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(FF FE)"));
    }
    SUBCASE("UTF-16, big")
    {
        constexpr auto rule = dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "BOM.UTF-16-BE").cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xFE, 0xFF);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(FE FF)"));
    }

    SUBCASE("UTF-32, little")
    {
        constexpr auto rule = dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "BOM.UTF-32-LE").cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xFF, 0xFE, 0x00, 0x00);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(FF FE 00 00)"));
    }
    SUBCASE("UTF-32, big")
    {
        constexpr auto rule = dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "BOM.UTF-32-BE").cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0x00, 0x00, 0xFE, 0xFF);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(00 00 FE FF)"));
    }
}

