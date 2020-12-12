// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/code_point.hpp>

#include "verify.hpp"

#if LEXY_HAS_CHAR8_T
#    define LEXY_CONSTEXPR constexpr
#else
#    define LEXY_CONSTEXPR const
#endif

TEST_CASE("engine_cp_ascii")
{
    using engine = lexy::engine_cp_ascii;
    CHECK(lexy::engine_is_matcher<engine>);
    CHECK(lexy::engine_is_parser<engine>);

    auto parse = [](auto str) {
        auto match_result = engine_matches<engine, lexy::ascii_encoding>(str);
        auto parse_result = engine_parses<engine, lexy::ascii_encoding>(str);

        CONSTEXPR_CHECK(match_result.ec == parse_result.ec);
        CONSTEXPR_CHECK(match_result.count == parse_result.count);

        return parse_result;
    };

    SUBCASE("constexpr")
    {
        constexpr auto empty = parse("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::error_code::eof);

        constexpr auto a = parse("a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value.value() == 'a');

        constexpr auto out_of_range = parse("\x90");
        CHECK(!out_of_range);
        CHECK(out_of_range.count == 1);
        CHECK(out_of_range.ec == engine::error_code::out_of_range);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const char str[]  = {char(i), char(i), char(i), '\0'};
            auto       result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value.value() == i);
        }
    }
    SUBCASE("non ASCII")
    {
        for (auto i = 0x80; i < 0xFF; ++i)
        {
            INFO(i);

            const char str[]  = {char(i), char(i), char(i), '\0'};
            auto       result = parse(str);
            CHECK(!result);
            CHECK(result.count == 1);
            CHECK(result.ec == engine::error_code::out_of_range);
        }
    }
}

TEST_CASE("engine_cp_utf8")
{
    using engine = lexy::engine_cp_utf8;
    CHECK(lexy::engine_is_matcher<engine>);
    CHECK(lexy::engine_is_parser<engine>);

    auto parse = [](auto str) {
        auto match_result = engine_matches<engine, lexy::utf8_encoding>(str);
        auto parse_result = engine_parses<engine, lexy::utf8_encoding>(str);

        CONSTEXPR_CHECK(match_result.ec == parse_result.ec);
        CONSTEXPR_CHECK(match_result.count == parse_result.count);

        return parse_result;
    };
    auto parse_seq = [](auto... c) {
        LEXY_CHAR8_T str[]        = {LEXY_CHAR8_T(c)..., 0x0};
        auto         match_result = engine_matches<engine, lexy::utf8_encoding>(str);
        auto         parse_result = engine_parses<engine, lexy::utf8_encoding>(str);

        CONSTEXPR_CHECK(match_result.ec == parse_result.ec);
        CONSTEXPR_CHECK(match_result.count == parse_result.count);

        return parse_result;
    };

    SUBCASE("constexpr")
    {
        LEXY_CONSTEXPR auto empty = parse(u8"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::error_code::eof);

        LEXY_CONSTEXPR auto a = parse(u8"a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value.value() == 'a');
        LEXY_CONSTEXPR auto umlaut = parse(u8"ä");
        CHECK(umlaut);
        CHECK(umlaut.count == 2);
        CHECK(umlaut.value.value() == 0xE4);
        LEXY_CONSTEXPR auto euro = parse(u8"€");
        CHECK(euro);
        CHECK(euro.count == 3);
        CHECK(euro.value.value() == 0x20AC);
        LEXY_CONSTEXPR auto emojii = parse(u8"🙂");
        CHECK(emojii);
        CHECK(emojii.count == 4);
        CHECK(emojii.value.value() == 0x1F642);

        constexpr auto leads_with_trailing = parse_seq(0b1000'0001);
        CHECK(!leads_with_trailing);
        CHECK(leads_with_trailing.count == 0);
        CHECK(leads_with_trailing.ec == engine::error_code::leads_with_trailing);

        constexpr auto missing_first1 = parse_seq(0b1101'0000);
        CHECK(!missing_first1);
        CHECK(missing_first1.count == 1);
        CHECK(missing_first1.ec == engine::error_code::missing_trailing);
        constexpr auto missing_first2 = parse_seq(0b1110'1000);
        CHECK(!missing_first2);
        CHECK(missing_first2.count == 1);
        CHECK(missing_first2.ec == engine::error_code::missing_trailing);
        constexpr auto missing_first3 = parse_seq(0b1111'0100);
        CHECK(!missing_first3);
        CHECK(missing_first3.count == 1);
        CHECK(missing_first3.ec == engine::error_code::missing_trailing);
        constexpr auto missing_second2 = parse_seq(0b1110'1000, 0b1000'0001);
        CHECK(!missing_second2);
        CHECK(missing_second2.count == 2);
        CHECK(missing_second2.ec == engine::error_code::missing_trailing);
        constexpr auto missing_second3 = parse_seq(0b1111'0100, 0b1000'0001);
        CHECK(!missing_second3);
        CHECK(missing_second3.count == 2);
        CHECK(missing_second3.ec == engine::error_code::missing_trailing);
        constexpr auto missing_third3 = parse_seq(0b1111'0100, 0b1000'0001, 0b1000'0001);
        CHECK(!missing_third3);
        CHECK(missing_third3.count == 3);
        CHECK(missing_third3.ec == engine::error_code::missing_trailing);

        constexpr auto invalid_first1 = parse_seq(0b1101'0000, 0b1111);
        CHECK(!invalid_first1);
        CHECK(invalid_first1.count == 1);
        CHECK(invalid_first1.ec == engine::error_code::missing_trailing);
        constexpr auto invalid_first2 = parse_seq(0b1110'1000, 0b1111);
        CHECK(!invalid_first2);
        CHECK(invalid_first2.count == 1);
        CHECK(invalid_first2.ec == engine::error_code::missing_trailing);
        constexpr auto invalid_first3 = parse_seq(0b1111'0100, 0b1111);
        CHECK(!invalid_first3);
        CHECK(invalid_first3.count == 1);
        CHECK(invalid_first3.ec == engine::error_code::missing_trailing);
        constexpr auto invalid_second2 = parse_seq(0b1110'1000, 0b1000'0001, 0b1111);
        CHECK(!invalid_second2);
        CHECK(invalid_second2.count == 2);
        CHECK(invalid_second2.ec == engine::error_code::missing_trailing);
        constexpr auto invalid_second3 = parse_seq(0b1111'0100, 0b1000'0001, 0b1111);
        CHECK(!invalid_second3);
        CHECK(invalid_second3.count == 2);
        CHECK(invalid_second3.ec == engine::error_code::missing_trailing);
        constexpr auto invalid_third3 = parse_seq(0b1111'0100, 0b1000'0001, 0b1000'0001, 0b1111);
        CHECK(!invalid_third3);
        CHECK(invalid_third3.count == 3);
        CHECK(invalid_third3.ec == engine::error_code::missing_trailing);

        constexpr auto surrogate = parse_seq(0b1110'1101, 0b1011'1111, 0b1011'1111);
        CHECK(!surrogate);
        CHECK(surrogate.count == 3);
        CHECK(surrogate.ec == engine::error_code::surrogate);
        constexpr auto out_of_range = parse_seq(0b1111'0111, 0b1011'1111, 0b1011'1111, 0b1011'1111);
        CHECK(!out_of_range);
        CHECK(out_of_range.count == 4);
        CHECK(out_of_range.ec == engine::error_code::out_of_range);

        constexpr auto overlong_two1 = parse_seq(0xC0, 0x84);
        CHECK(!overlong_two1);
        CHECK(overlong_two1.count == 0);
        CHECK(overlong_two1.ec == engine::error_code::overlong_sequence);
        constexpr auto overlong_two2 = parse_seq(0xC1, 0x84);
        CHECK(!overlong_two2);
        CHECK(overlong_two2.count == 0);
        CHECK(overlong_two2.ec == engine::error_code::overlong_sequence);
        constexpr auto overlong_three = parse_seq(0xE0, 0x80, 0x80);
        CHECK(!overlong_three);
        CHECK(overlong_three.count == 1);
        CHECK(overlong_three.ec == engine::error_code::overlong_sequence);
        constexpr auto overlong_four = parse_seq(0xF0, 0x80, 0x80, 0x80);
        CHECK(!overlong_four);
        CHECK(overlong_four.count == 1);
        CHECK(overlong_four.ec == engine::error_code::overlong_sequence);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const LEXY_CHAR8_T str[]  = {LEXY_CHAR8_T(i), LEXY_CHAR8_T(i), LEXY_CHAR8_T(i), '\0'};
            auto               result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value.value() == i);
        }
    }
}

TEST_CASE("engine_cp_utf16")
{
    using engine = lexy::engine_cp_utf16;
    CHECK(lexy::engine_is_matcher<engine>);
    CHECK(lexy::engine_is_parser<engine>);

    auto parse = [](auto str) {
        auto match_result = engine_matches<engine, lexy::utf16_encoding>(str);
        auto parse_result = engine_parses<engine, lexy::utf16_encoding>(str);

        CONSTEXPR_CHECK(match_result.ec == parse_result.ec);
        CONSTEXPR_CHECK(match_result.count == parse_result.count);

        return parse_result;
    };

    SUBCASE("constexpr")
    {
        constexpr auto empty = parse(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::error_code::eof);

        constexpr auto a = parse(u"a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value.value() == 'a');
        constexpr auto umlaut = parse(u"ä");
        CHECK(umlaut);
        CHECK(umlaut.count == 1);
        CHECK(umlaut.value.value() == 0xE4);
        constexpr auto euro = parse(u"€");
        CHECK(euro);
        CHECK(euro.count == 1);
        CHECK(euro.value.value() == 0x20AC);
        constexpr auto emojii = parse(u"🙂");
        CHECK(emojii);
        CHECK(emojii.count == 2);
        CHECK(emojii.value.value() == 0x1F642);

        constexpr char16_t leads_with_trailing_str[] = {0xDC44, 0x0};
        constexpr auto     leads_with_trailing       = parse(leads_with_trailing_str);
        CHECK(!leads_with_trailing);
        CHECK(leads_with_trailing.count == 0);
        CHECK(leads_with_trailing.ec == engine::error_code::leads_with_trailing);

        constexpr char16_t missing_trailing_str[] = {0xDA44, 0x0};
        constexpr auto     missing_trailing       = parse(missing_trailing_str);
        CHECK(!missing_trailing);
        CHECK(missing_trailing.count == 1);
        CHECK(missing_trailing.ec == engine::error_code::missing_trailing);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const char16_t str[]  = {char16_t(i), char16_t(i), char16_t(i), '\0'};
            auto           result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value.value() == i);
        }
    }
    SUBCASE("BMP")
    {
        for (auto i = 0x80; i <= 0xFFFF; ++i)
        {
            INFO(i);
            auto cp = lexy::code_point(char32_t(i));

            const char16_t str[]  = {char16_t(i), char16_t(i), char16_t(i), '\0'};
            auto           result = parse(str);
            if (cp.is_surrogate())
            {
                CHECK(!result);
                if (i < 0xDC00)
                {
                    CHECK(result.count == 1);
                    CHECK(result.ec == engine::error_code::missing_trailing);
                }
                else
                {
                    CHECK(result.count == 0);
                    CHECK(result.ec == engine::error_code::leads_with_trailing);
                }
            }
            else
            {
                CHECK(result);
                CHECK(result.count == 1);
                CHECK(result.value.value() == i);
            }
        }
    }
}

TEST_CASE("engine_cp_utf32")
{
    using engine = lexy::engine_cp_utf32;
    CHECK(lexy::engine_is_matcher<engine>);
    CHECK(lexy::engine_is_parser<engine>);

    auto parse = [](auto str) {
        auto match_result = engine_matches<engine, lexy::utf32_encoding>(str);
        auto parse_result = engine_parses<engine, lexy::utf32_encoding>(str);

        CONSTEXPR_CHECK(match_result.ec == parse_result.ec);
        CONSTEXPR_CHECK(match_result.count == parse_result.count);

        return parse_result;
    };

    SUBCASE("constexpr")
    {
        constexpr auto empty = parse(U"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::error_code::eof);

        constexpr auto a = parse(U"a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value.value() == 'a');
        constexpr auto umlaut = parse(U"ä");
        CHECK(umlaut);
        CHECK(umlaut.count == 1);
        CHECK(umlaut.value.value() == 0xE4);
        constexpr auto euro = parse(U"€");
        CHECK(euro);
        CHECK(euro.count == 1);
        CHECK(euro.value.value() == 0x20AC);
        constexpr auto emojii = parse(U"🙂");
        CHECK(emojii);
        CHECK(emojii.count == 1);
        CHECK(emojii.value.value() == 0x1F642);

        constexpr char32_t surrogate_str[] = {0xD844, 0x0};
        constexpr auto     surrogate       = parse(surrogate_str);
        CHECK(!surrogate);
        CHECK(surrogate.count == 1);
        CHECK(surrogate.ec == engine::error_code::surrogate);

        constexpr char32_t out_of_range_str[] = {0xFF1234, 0x0};
        constexpr auto     out_of_range       = parse(out_of_range_str);
        CHECK(!out_of_range);
        CHECK(out_of_range.count == 1);
        CHECK(out_of_range.ec == engine::error_code::out_of_range);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const char32_t str[]  = {char32_t(i), char32_t(i), char32_t(i), '\0'};
            auto           result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value.value() == i);
        }
    }
    SUBCASE("BMP")
    {
        for (auto i = 0x80; i <= 0xFFFF; ++i)
        {
            INFO(i);
            auto cp = lexy::code_point(char32_t(i));

            const char32_t str[]  = {char32_t(i), char32_t(i), char32_t(i), '\0'};
            auto           result = parse(str);
            if (cp.is_surrogate())
            {
                CHECK(!result);
                CHECK(result.count == 1);
                CHECK(result.ec == engine::error_code::surrogate);
            }
            else
            {
                CHECK(result);
                CHECK(result.count == 1);
                CHECK(result.value.value() == i);
            }
        }
    }
}

TEST_CASE("engine_cp_auto")
{
    using engine = lexy::engine_cp_auto;
    CHECK(lexy::engine_is_matcher<engine>);
    CHECK(lexy::engine_is_parser<engine>);

    SUBCASE("parsing")
    {
        constexpr auto ascii = engine_parses<engine, lexy::ascii_encoding>("a");
        CHECK(ascii);
        CHECK(ascii.value.value() == 'a');

        LEXY_CONSTEXPR auto utf8 = engine_parses<engine, lexy::utf8_encoding>(u8"ä");
        CHECK(utf8);
        CHECK(utf8.value.value() == 0xE4);

        constexpr auto utf16 = engine_parses<engine, lexy::utf16_encoding>(u"ä");
        CHECK(utf16);
        CHECK(utf16.value.value() == 0xE4);

        constexpr auto utf32 = engine_parses<engine, lexy::utf32_encoding>(U"ä");
        CHECK(utf32);
        CHECK(utf32.value.value() == 0xE4);
    }
    SUBCASE("matching")
    {
        constexpr auto ascii = engine_matches<engine, lexy::ascii_encoding>("a");
        CHECK(ascii);

        LEXY_CONSTEXPR auto utf8 = engine_matches<engine, lexy::utf8_encoding>(u8"ä");
        CHECK(utf8);

        constexpr auto utf16 = engine_matches<engine, lexy::utf16_encoding>(u"ä");
        CHECK(utf16);

        constexpr auto utf32 = engine_matches<engine, lexy::utf32_encoding>(U"ä");
        CHECK(utf32);
    }
}

