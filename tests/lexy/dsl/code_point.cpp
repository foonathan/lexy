// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/code_point.hpp>

#include "verify.hpp"
#include <lexy/parse.hpp>

TEST_CASE("dsl::code_point")
{
    constexpr auto rule = lexy::dsl::code_point;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    // Only basic sanity checks needed, the actual engine is tested extensively.

    SUBCASE("ASCII")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 1);
                return 0;
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::ascii_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.code_point");
                return -1;
            }
        };

        constexpr auto empty = verify<callback, lexy::ascii_encoding>(rule, "");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::ascii_encoding>(rule, "a");
        CHECK(cp == 0);
    }
    SUBCASE("UTF-8")
    {
        struct callback
        {
            const LEXY_CHAR8_T* str;

            constexpr int success(const LEXY_CHAR8_T* cur)
            {
                return int(cur - str);
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::utf8_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "UTF-8.code_point");
                return -1;
            }
        };

#if LEXY_HAS_CHAR8_T
        constexpr auto empty = verify<callback, lexy::utf8_encoding>(rule, u8"");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::utf8_encoding>(rule, u8"ä");
        CHECK(cp == 2);
#else
        auto empty_str = reinterpret_cast<const LEXY_CHAR8_T*>(u8"");
        auto empty     = verify<callback, lexy::utf8_encoding>(rule, empty_str);
        CHECK(empty == -1);

        auto cp_str = reinterpret_cast<const LEXY_CHAR8_T*>(u8"ä");
        auto cp     = verify<callback, lexy::utf8_encoding>(rule, cp_str);
        CHECK(cp == 2);
#endif
    }
    SUBCASE("UTF-16")
    {
        struct callback
        {
            const char16_t* str;

            constexpr int success(const char16_t* cur)
            {
                return int(cur - str);
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::utf16_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "UTF-16.code_point");
                return -1;
            }
        };

        constexpr auto empty = verify<callback, lexy::utf16_encoding>(rule, u"");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::utf16_encoding>(rule, u"ä");
        CHECK(cp == 1);
    }
    SUBCASE("UTF-32")
    {
        struct callback
        {
            const char32_t* str;

            constexpr int success(const char32_t* cur)
            {
                return int(cur - str);
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::utf32_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "UTF-32.code_point");
                return -1;
            }
        };

        constexpr auto empty = verify<callback, lexy::utf32_encoding>(rule, U"");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::utf32_encoding>(rule, U"ä");
        CHECK(cp == 1);
    }
}

TEST_CASE("dsl::code_point.capture()")
{
    constexpr auto rule = lexy::dsl::code_point.capture();
    CHECK(lexy::is_rule<decltype(rule)>);

    // Only basic sanity checks needed, the actual engine is tested extensively.

    SUBCASE("ASCII")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char*, lexy::code_point cp)
            {
                return int(cp.value());
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::ascii_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "ASCII.code_point");
                return -1;
            }
        };

        constexpr auto empty = verify<callback, lexy::ascii_encoding>(rule, "");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::ascii_encoding>(rule, "a");
        CHECK(cp == 0x61);
    }
    SUBCASE("UTF-8")
    {
        struct callback
        {
            const LEXY_CHAR8_T* str;

            constexpr int success(const LEXY_CHAR8_T*, lexy::code_point cp)
            {
                return int(cp.value());
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::utf8_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "UTF-8.code_point");
                return -1;
            }
        };

#if LEXY_HAS_CHAR8_T
        constexpr auto empty = verify<callback, lexy::utf8_encoding>(rule, u8"");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::utf8_encoding>(rule, u8"ä");
        CHECK(cp == 0xE4);
#else
        auto empty_str = reinterpret_cast<const LEXY_CHAR8_T*>(u8"");
        auto empty     = verify<callback, lexy::utf8_encoding>(rule, empty_str);
        CHECK(empty == -1);

        auto cp_str = reinterpret_cast<const LEXY_CHAR8_T*>(u8"ä");
        auto cp     = verify<callback, lexy::utf8_encoding>(rule, cp_str);
        CHECK(cp == 0xE4);
#endif
    }
    SUBCASE("UTF-16")
    {
        struct callback
        {
            const char16_t* str;

            constexpr int success(const char16_t*, lexy::code_point cp)
            {
                return int(cp.value());
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::utf16_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "UTF-16.code_point");
                return -1;
            }
        };

        constexpr auto empty = verify<callback, lexy::utf16_encoding>(rule, u"");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::utf16_encoding>(rule, u"ä");
        CHECK(cp == 0xE4);
    }
    SUBCASE("UTF-32")
    {
        struct callback
        {
            const char32_t* str;

            constexpr int success(const char32_t*, lexy::code_point cp)
            {
                return int(cp.value());
            }

            constexpr int error(
                lexy::string_error<lexy::expected_char_class, lexy::utf32_encoding> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.character_class() == "UTF-32.code_point");
                return -1;
            }
        };

        constexpr auto empty = verify<callback, lexy::utf32_encoding>(rule, U"");
        CHECK(empty == -1);

        constexpr auto cp = verify<callback, lexy::utf32_encoding>(rule, U"ä");
        CHECK(cp == 0xE4);
    }
}

