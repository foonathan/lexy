// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/integer.hpp>

#include "verify.hpp"
#include <cstdint>
#include <cstdio>
#include <string>

TEST_CASE("lexyd::_digit_count")
{
    using lexy::_digit_count;

    SUBCASE("base 2")
    {
        CHECK(_digit_count(2, 0b0) == 1);
        CHECK(_digit_count(2, 0b1) == 1);
        CHECK(_digit_count(2, 0b10) == 2);
        CHECK(_digit_count(2, 0b11) == 2);
        CHECK(_digit_count(2, 0b100) == 3);
        CHECK(_digit_count(2, 0b101) == 3);
        CHECK(_digit_count(2, 0b110) == 3);
        CHECK(_digit_count(2, 0b111) == 3);
        CHECK(_digit_count(2, 0b1000) == 4);

        CHECK(_digit_count(2, 0b1111'1111) == 8);
        CHECK(_digit_count(2, 0b1'0000'0000) == 9);
        CHECK(_digit_count(2, 0b1'0000'0001) == 9);

        CHECK(_digit_count(2, 0b1111'1111'1111'1111) == 16);
        CHECK(_digit_count(2, 0b1'0000'0000'0000'0000) == 17);
        CHECK(_digit_count(2, 0b1'0000'0000'0000'0001) == 17);
    }
    SUBCASE("base 10")
    {
        for (auto value = 0; value < 10; ++value)
        {
            INFO(value);
            CHECK(_digit_count(10, value) == 1);
        }

        for (auto value = 10; value < 100; ++value)
        {
            INFO(value);
            CHECK(_digit_count(10, value) == 2);
        }

        CHECK(_digit_count(10, 100) == 3);
        CHECK(_digit_count(10, 101) == 3);

        CHECK(_digit_count(10, 999) == 3);
        CHECK(_digit_count(10, 1000) == 4);
        CHECK(_digit_count(10, 1001) == 4);
    }
    SUBCASE("base 16")
    {
        for (auto value = 0x0; value < 0x10; ++value)
        {
            INFO(value);
            CHECK(_digit_count(16, value) == 1);
        }

        for (auto value = 0x10; value < 0x100; ++value)
        {
            INFO(value);
            CHECK(_digit_count(16, value) == 2);
        }

        CHECK(_digit_count(16, 0x100) == 3);
        CHECK(_digit_count(16, 0x101) == 3);

        CHECK(_digit_count(16, 0xFFFF) == 4);
        CHECK(_digit_count(16, 0x1'0000) == 5);
        CHECK(_digit_count(16, 0x1'0001) == 5);
    }
}

TEST_CASE("dsl::integer")
{
    auto parse = [](auto rule, const char* str) -> int {
        CHECK(lexy::is_rule<decltype(rule)>);
        struct callback
        {
            const char* str;

            constexpr int success(const char*, int value)
            {
                return value;
            }

            constexpr int error(test_error<lexy::integer_overflow> e)
            {
                CONSTEXPR_CHECK(e.message() == "integer overflow");
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
            constexpr int error(test_error<lexy::expected_char_class>)
            {
                return -2;
            }
        };

        return rule_matches<callback>(rule, str);
    };

    SUBCASE("base 10, uint8_t")
    {
        constexpr auto rule
            = lexy::dsl::integer<std::uint8_t>(lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick));

        for (auto i = 0; i < 256; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i);
        for (auto i = 256; i < 512; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == -1);

        CHECK(parse(rule, "000000000000") == 0);
        CHECK(parse(rule, "000000000000255") == 255);
        CHECK(parse(rule, "000000000000256") == -1);

        CHECK(parse(rule, "1'2'3") == 123);
        CHECK(parse(rule, "0'0'0'0'0'0'1'2'3") == 123);
    }
    SUBCASE("base 10, int8_t")
    {
        constexpr auto rule
            = lexy::dsl::integer<std::int8_t>(lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick));

        for (auto i = 0; i < 128; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i);
        for (auto i = 128; i < 512; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == -1);

        CHECK(parse(rule, "000000000000") == 0);
        CHECK(parse(rule, "000000000000127") == 127);
        CHECK(parse(rule, "000000000000128") == -1);

        CHECK(parse(rule, "1'2'3") == 123);
        CHECK(parse(rule, "0'0'0'0'0'0'1'2'3") == 123);
    }
    SUBCASE("base 10, uint16_t")
    {
        constexpr auto rule
            = lexy::dsl::integer<std::uint16_t>(lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick));

        for (auto i = 0; i < 256; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i);
        for (auto i = 0; i < 256; ++i)
        {
            auto value = i * i;
            CHECK(parse(rule, std::to_string(value).c_str()) == value);
        }
        for (auto i = 0; i < 256; ++i)
        {
            auto value = 65535 - i;
            CHECK(parse(rule, std::to_string(value).c_str()) == value);
        }

        CHECK(parse(rule, "000000000000") == 0);
        CHECK(parse(rule, "00000000000065535") == 65535);
        CHECK(parse(rule, "00000000000065536") == -1);

        CHECK(parse(rule, "1'2'3'4'5") == 12345);
        CHECK(parse(rule, "0'0'0'0'0'0'1'2'3'4'5") == 12345);
    }
    SUBCASE("base 10, unbounded")
    {
        constexpr auto rule = lexy::dsl::integer<lexy::unbounded<std::uint8_t>>(
            lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick));

        for (auto i = 0; i < 256; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i);
        for (auto i = 256; i < 512; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i - 256);

        CHECK(parse(rule, "000000000000") == 0);
        CHECK(parse(rule, "000000000000255") == 255);
        CHECK(parse(rule, "000000000000256") == 0);

        CHECK(parse(rule, "1'2'3") == 123);
        CHECK(parse(rule, "0'0'0'0'0'0'1'2'3") == 123);
    }

    SUBCASE("base 16, uint8_t")
    {
        constexpr auto rule = lexy::dsl::integer<std::uint8_t>(
            lexy::dsl::digits<lexy::dsl::hex>.sep(lexy::dsl::digit_sep_tick));

        char buffer[3];
        for (auto i = 0; i < 128; ++i)
        {
            INFO(i);

            std::sprintf(buffer, "%x", i);
            CHECK(parse(rule, buffer) == i);
            std::sprintf(buffer, "%X", i);
            CHECK(parse(rule, buffer) == i);
        }
        for (auto i = 128; i < 256; ++i)
        {
            INFO(i);

            std::sprintf(buffer, "%x", i);
            CHECK(parse(rule, buffer) == i);
            std::sprintf(buffer, "%X", i);
            CHECK(parse(rule, buffer) == i);
        }

        CHECK(parse(rule, "Aa") == 0xAA);

        CHECK(parse(rule, "0000") == 0);
        CHECK(parse(rule, "00FF") == 0xFF);
        CHECK(parse(rule, "0100") == -1);

        CHECK(parse(rule, "0'0'F'F") == 255);
        CHECK(parse(rule, "0'0'F'F") == 255);
    }

    SUBCASE("generic rule")
    {
        constexpr auto rule = lexy::dsl::integer<std::uint8_t, lexy::dsl::decimal>(
            lexy::dsl::digit<> + lexy::dsl::digit<>);

        for (auto i = 10; i < 100; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i);
    }
    SUBCASE("n_digits")
    {
        constexpr auto rule = lexy::dsl::integer<std::uint8_t>(lexy::dsl::n_digits<2>);

        for (auto i = 10; i < 100; ++i)
            CHECK(parse(rule, std::to_string(i).c_str()) == i);
    }
}

TEST_CASE("dsl::code_point_id")
{
    constexpr auto rule = lexy::dsl::code_point_id<6>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::code_point cp)
        {
            CONSTEXPR_CHECK(cur == str + 6);
            return int(cp.value());
        }

        constexpr int error(test_error<lexy::integer_overflow> e)
        {
            CONSTEXPR_CHECK(e.message() == "integer overflow");
            CONSTEXPR_CHECK(e.begin() == str);
            CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
            return -1;
        }
        constexpr int error(test_error<lexy::expected_char_class>)
        {
            return -2;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -2);

    constexpr auto latin_small_letter_e_with_acute = rule_matches<callback>(rule, "0000E9");
    CHECK(latin_small_letter_e_with_acute == 0x0000E9);
    constexpr auto euro_sign = rule_matches<callback>(rule, "0020AC");
    CHECK(euro_sign == 0x20AC);
    constexpr auto slightly_smiling_face = rule_matches<callback>(rule, "01F92D");
    CHECK(slightly_smiling_face == 0x1F92D);

    constexpr auto extra_digits = rule_matches<callback>(rule, "0000001");
    CHECK(extra_digits == 0);
    constexpr auto overflow = rule_matches<callback>(rule, "ABCDEF");
    CHECK(overflow == -1);
}

