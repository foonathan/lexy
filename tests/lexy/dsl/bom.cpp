// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/bom.hpp>

#include "verify.hpp"

TEST_CASE("dsl::bom")
{
    SUBCASE("no bom")
    {
        constexpr auto rule
            = lexy::dsl::bom<lexy::default_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == 0);
    }
    SUBCASE("UTF-8")
    {
        constexpr auto rule
            = lexy::dsl::bom<lexy::utf8_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "BOM.UTF-8");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        static constexpr char str[] = {char(0xEF), char(0xBB), char(0xBF)};
        constexpr auto        bom   = verify<callback>(rule, str, 3);
        CHECK(bom == 3);
    }
    SUBCASE("UTF-16 little")
    {
        constexpr auto rule
            = lexy::dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "BOM.UTF-16-LE");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        static constexpr char str[] = {char(0xFF), char(0xFE)};
        constexpr auto        bom   = verify<callback>(rule, str, 2);
        CHECK(bom == 2);
    }
    SUBCASE("UTF-16 big")
    {
        constexpr auto rule = lexy::dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "BOM.UTF-16-BE");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        static constexpr char str[] = {char(0xFE), char(0xFF)};
        constexpr auto        bom   = verify<callback>(rule, str, 2);
        CHECK(bom == 2);
    }
    SUBCASE("UTF-32 little")
    {
        constexpr auto rule
            = lexy::dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "BOM.UTF-32-LE");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        static constexpr char str[] = {char(0xFF), char(0xFE), 0, 0};
        constexpr auto        bom   = verify<callback>(rule, str, 4);
        CHECK(bom == 4);
    }
    SUBCASE("UTF-32 big")
    {
        constexpr auto rule = lexy::dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "BOM.UTF-32-BE");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        static constexpr char str[] = {0, 0, char(0xFE), char(0xFF)};
        constexpr auto        bom   = verify<callback>(rule, str, 4);
        CHECK(bom == 4);
    }
}

