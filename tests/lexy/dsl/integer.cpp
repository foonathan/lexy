// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/integer.hpp>

#include "verify.hpp"
#include <cstdio>
#include <cstring>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/loop.hpp>
#include <string>

TEST_CASE("_digit_count")
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

namespace
{
template <typename Parser>
auto parse_int(Parser, const char* str)
{
    struct result_type
    {
        typename Parser::result_type value;
        bool                         success;

        bool operator==(int v) const
        {
            return success && value == typename Parser::result_type(v);
        }
    } result = {};

    result.success = Parser::parse(result.value, str, str + std::strlen(str));
    return result;
}
} // namespace

TEST_CASE("_integer_parser")
{
    SUBCASE("base 10, uint8_t")
    {
        constexpr auto parser = dsl::_integer_parser<std::uint8_t, dsl::decimal, false>{};

        for (auto i = 0; i < 256; ++i)
            CHECK(parse_int(parser, std::to_string(i).c_str()) == i);
        for (auto i = 256; i < 512; ++i)
        {
            auto result = parse_int(parser, std::to_string(i).c_str());
            CHECK(!result.success);
            CHECK(result.value == i / 10);
        }

        CHECK(parse_int(parser, "000000000000") == 0);
        CHECK(parse_int(parser, "000000000000255") == 255);

        auto overflow_zeroes = parse_int(parser, "000000000000256");
        CHECK(!overflow_zeroes.success);
        CHECK(overflow_zeroes.value == 25);

        CHECK(parse_int(parser, "1'2'3") == 123);
        CHECK(parse_int(parser, "0'0'0'0'0'0'1'2'3") == 123);
    }
    SUBCASE("base 10, int8_t")
    {
        constexpr auto parser = dsl::_integer_parser<std::int8_t, dsl::decimal, false>{};

        for (auto i = 0; i < 128; ++i)
            CHECK(parse_int(parser, std::to_string(i).c_str()) == i);
        for (auto i = 128; i < 512; ++i)
        {
            auto result = parse_int(parser, std::to_string(i).c_str());
            CHECK(!result.success);
            CHECK(result.value == i / 10);
        }

        CHECK(parse_int(parser, "000000000000") == 0);
        CHECK(parse_int(parser, "000000000000127") == 127);

        auto overflow_zeroes = parse_int(parser, "000000000000128");
        CHECK(!overflow_zeroes.success);
        CHECK(overflow_zeroes.value == 12);

        CHECK(parse_int(parser, "1'2'3") == 123);
        CHECK(parse_int(parser, "0'0'0'0'0'0'1'2'3") == 123);
    }
    SUBCASE("base 10, uint16_t")
    {
        constexpr auto parser = dsl::_integer_parser<std::uint16_t, dsl::decimal, false>{};

        for (auto i = 0; i < 256; ++i)
            CHECK(parse_int(parser, std::to_string(i).c_str()) == i);
        for (auto i = 0; i < 256; ++i)
        {
            auto value = i * i;
            CHECK(parse_int(parser, std::to_string(value).c_str()) == value);
        }
        for (auto i = 0; i < 256; ++i)
        {
            auto value = 65535 - i;
            CHECK(parse_int(parser, std::to_string(value).c_str()) == value);
        }

        CHECK(parse_int(parser, "000000000000") == 0);
        CHECK(parse_int(parser, "00000000000065535") == 65535);

        auto overflow_zeroes = parse_int(parser, "00000000000065536");
        CHECK(!overflow_zeroes.success);
        CHECK(overflow_zeroes.value == 6553);

        CHECK(parse_int(parser, "1'2'3'4'5") == 12345);
        CHECK(parse_int(parser, "0'0'0'0'0'0'1'2'3'4'5") == 12345);
    }
    SUBCASE("base 10, int")
    {
        constexpr auto parser = dsl::_integer_parser<int, dsl::decimal, false>{};

        for (auto i = 0; i < 256; ++i)
            CHECK(parse_int(parser, std::to_string(i).c_str()) == i);
        for (auto i = 0; i < 256; ++i)
        {
            auto value = i * i;
            CHECK(parse_int(parser, std::to_string(value).c_str()) == value);
        }
        for (auto i = 0; i < 256; ++i)
        {
            auto value = INT_MAX - i;
            CHECK(parse_int(parser, std::to_string(value).c_str()) == value);
        }

        CHECK(parse_int(parser, "000000000000") == 0);
        CHECK(parse_int(parser, ("000000000000" + std::to_string(INT_MAX)).c_str()) == INT_MAX);

        auto overflow_zeroes
            = parse_int(parser, ("000000000000" + std::to_string(INT_MAX + 1ll)).c_str());
        CHECK(!overflow_zeroes.success);
        CHECK(overflow_zeroes.value == INT_MAX / 10 * 10);

        CHECK(parse_int(parser, "1'2'3'4'5") == 12345);
        CHECK(parse_int(parser, "0'0'0'0'0'0'1'2'3'4'5") == 12345);
    }
    SUBCASE("base 10, unbounded")
    {
        constexpr auto parser
            = dsl::_integer_parser<lexy::unbounded<std::uint8_t>, dsl::decimal, false>{};

        for (auto i = 0; i < 256; ++i)
            CHECK(parse_int(parser, std::to_string(i).c_str()) == i);
        for (auto i = 256; i < 512; ++i)
            CHECK(parse_int(parser, std::to_string(i).c_str()) == i - 256);

        CHECK(parse_int(parser, "000000000000") == 0);
        CHECK(parse_int(parser, "000000000000255") == 255);
        CHECK(parse_int(parser, "000000000000256") == 0);

        CHECK(parse_int(parser, "1'2'3") == 123);
        CHECK(parse_int(parser, "0'0'0'0'0'0'1'2'3") == 123);
    }

    SUBCASE("base 16, uint8_t")
    {
        constexpr auto parser = dsl::_integer_parser<std::uint8_t, dsl::hex, false>{};

        char buffer[3];
        for (auto i = 0; i < 128; ++i)
        {
            INFO(i);

            std::sprintf(buffer, "%x", i);
            CHECK(parse_int(parser, buffer) == i);
            std::sprintf(buffer, "%X", i);
            CHECK(parse_int(parser, buffer) == i);
        }
        for (auto i = 128; i < 256; ++i)
        {
            INFO(i);

            std::sprintf(buffer, "%x", i);
            CHECK(parse_int(parser, buffer) == i);
            std::sprintf(buffer, "%X", i);
            CHECK(parse_int(parser, buffer) == i);
        }

        CHECK(parse_int(parser, "Aa") == 0xAA);

        CHECK(parse_int(parser, "0000") == 0);
        CHECK(parse_int(parser, "00FF") == 0xFF);

        auto overflow_zeroes = parse_int(parser, "0100");
        CHECK(!overflow_zeroes.success);
        CHECK(overflow_zeroes.value == 0x10);

        CHECK(parse_int(parser, "0'0'F'F") == 255);
        CHECK(parse_int(parser, "0'0'F'F") == 255);
    }
}

TEST_CASE("dsl::integer(rule)")
{
    constexpr auto rule = dsl::integer<int, dsl::decimal>(dsl::while_(dsl::digit<>));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char*, int value) { return value; };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());

    auto two = LEXY_VERIFY("11");
    CHECK(two.status == test_result::success);
    CHECK(two.value == 11);
    CHECK(two.trace == test_trace().token("1").token("1"));
    auto five = LEXY_VERIFY("12345");
    CHECK(five.status == test_result::success);
    CHECK(five.value == 12345);
    CHECK(five.trace == test_trace().token("1").token("2").token("3").token("4").token("5"));

    auto overflow = LEXY_VERIFY("12345678901234567890");
    CHECK(overflow.status == test_result::recovered_error);
    if (lexy::_digit_count(10, INT_MAX) == 10)
        CHECK(overflow.value == 1234567890);
    // Don't check for trace as it's too long.
}

TEST_CASE("dsl::integer(branch)")
{
    constexpr auto integer = dsl::integer<int, dsl::decimal>(dsl::while_one(dsl::digit<>));
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("1").token("1"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("1").token("2").token("3").token("4").token("5"));

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        // Don't check for trace as it's too long.
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("1").token("1"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("1").token("2").token("3").token("4").token("5"));

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        // Don't check for trace as it's too long.
    }
}

TEST_CASE("dsl::integer(dsl::digits)")
{
    constexpr auto integer = dsl::integer<int>(dsl::digits<>);
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
}

TEST_CASE("dsl::integer(dsl::digits.no_leading_zero())")
{
    constexpr auto integer = dsl::integer<int>(dsl::digits<>.no_leading_zero());
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto zero_zero_seven = LEXY_VERIFY("007");
        CHECK(zero_zero_seven.status == test_result::recovered_error);
        CHECK(zero_zero_seven.value == 7);
        CHECK(zero_zero_seven.trace
              == test_trace()
                     .error(0, 1, "forbidden leading zero")
                     .error_token("0")
                     .recovery()
                     .error_token("07"));

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto zero_zero_seven = LEXY_VERIFY("007");
        CHECK(zero_zero_seven.status == test_result::success);
        CHECK(zero_zero_seven.value == -11);
        CHECK(zero_zero_seven.trace == test_trace());

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
}

TEST_CASE("dsl::integer(dsl::digits.sep())")
{
    constexpr auto integer = dsl::integer<int>(dsl::digits<>.sep(LEXY_LIT("_")));
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto with_sep = LEXY_VERIFY("1_2_3");
        CHECK(with_sep.status == test_result::success);
        CHECK(with_sep.value == 123);
        CHECK(with_sep.trace == test_trace().token("1_2_3"));

        auto leading_sep = LEXY_VERIFY("_1");
        CHECK(leading_sep.status == test_result::recovered_error);
        CHECK(leading_sep.value == 1);
        CHECK(leading_sep.trace
              == test_trace().expected_char_class(0, "digit.decimal").recovery().error_token("_1"));
        auto trailing_sep = LEXY_VERIFY("1_");
        CHECK(trailing_sep.status == test_result::recovered_error);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace
              == test_trace().expected_char_class(2, "digit.decimal").error_token("1_").recovery());

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto with_sep = LEXY_VERIFY("1_2_3");
        CHECK(with_sep.status == test_result::success);
        CHECK(with_sep.value == 123);
        CHECK(with_sep.trace == test_trace().token("1_2_3"));

        auto leading_sep = LEXY_VERIFY("_1");
        CHECK(leading_sep.status == test_result::success);
        CHECK(leading_sep.value == -11);
        CHECK(leading_sep.trace == test_trace());
        auto trailing_sep = LEXY_VERIFY("1_");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == -11);
        CHECK(trailing_sep.trace == test_trace());

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
}

TEST_CASE("dsl::integer(dsl::digits.sep().no_leading_zero())")
{
    constexpr auto integer = dsl::integer<int>(dsl::digits<>.sep(LEXY_LIT("_")).no_leading_zero());
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto with_sep = LEXY_VERIFY("1_2_3");
        CHECK(with_sep.status == test_result::success);
        CHECK(with_sep.value == 123);
        CHECK(with_sep.trace == test_trace().token("1_2_3"));

        auto leading_sep = LEXY_VERIFY("_1");
        CHECK(leading_sep.status == test_result::recovered_error);
        CHECK(leading_sep.value == 1);
        CHECK(leading_sep.trace
              == test_trace().expected_char_class(0, "digit.decimal").recovery().error_token("_1"));
        auto trailing_sep = LEXY_VERIFY("1_");
        CHECK(trailing_sep.status == test_result::recovered_error);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace
              == test_trace().expected_char_class(2, "digit.decimal").error_token("1_").recovery());

        auto zero_zero_seven = LEXY_VERIFY("007");
        CHECK(zero_zero_seven.status == test_result::recovered_error);
        CHECK(zero_zero_seven.value == 7);
        CHECK(zero_zero_seven.trace
              == test_trace()
                     .error(0, 1, "forbidden leading zero")
                     .error_token("0")
                     .recovery()
                     .error_token("07"));

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto two = LEXY_VERIFY("11");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 11);
        CHECK(two.trace == test_trace().token("11"));
        auto five = LEXY_VERIFY("12345");
        CHECK(five.status == test_result::success);
        CHECK(five.value == 12345);
        CHECK(five.trace == test_trace().token("12345"));

        auto with_sep = LEXY_VERIFY("1_2_3");
        CHECK(with_sep.status == test_result::success);
        CHECK(with_sep.value == 123);
        CHECK(with_sep.trace == test_trace().token("1_2_3"));

        auto leading_sep = LEXY_VERIFY("_1");
        CHECK(leading_sep.status == test_result::success);
        CHECK(leading_sep.value == -11);
        CHECK(leading_sep.trace == test_trace());
        auto trailing_sep = LEXY_VERIFY("1_");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == -11);
        CHECK(trailing_sep.trace == test_trace());

        auto zero_zero_seven = LEXY_VERIFY("007");
        CHECK(zero_zero_seven.status == test_result::success);
        CHECK(zero_zero_seven.value == -11);
        CHECK(zero_zero_seven.trace == test_trace());

        auto overflow = LEXY_VERIFY("12345678901234567890");
        CHECK(overflow.status == test_result::recovered_error);
        if (lexy::_digit_count(10, INT_MAX) == 10)
            CHECK(overflow.value == 1234567890);
        CHECK(overflow.trace
              == test_trace().token("12345678901234567890").error(0, 20, "integer overflow"));
    }
}

TEST_CASE("dsl::integer(dsl::n_digits)")
{
    constexpr auto integer = dsl::integer<int>(dsl::n_digits<3>);
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto one = LEXY_VERIFY("1");
        CHECK(one.status == test_result::recovered_error);
        CHECK(one.value == 1);
        CHECK(one.trace
              == test_trace().expected_char_class(1, "digit.decimal").error_token("1").recovery());
        auto two = LEXY_VERIFY("12");
        CHECK(two.status == test_result::recovered_error);
        CHECK(two.value == 12);
        CHECK(two.trace
              == test_trace().expected_char_class(2, "digit.decimal").error_token("12").recovery());

        auto three = LEXY_VERIFY("123");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 123);
        CHECK(three.trace == test_trace().token("123"));
        auto four = LEXY_VERIFY("1234");
        CHECK(four.status == test_result::success);
        CHECK(four.value == 123);
        CHECK(four.trace == test_trace().token("123"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY("1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == -11);
        CHECK(one.trace == test_trace());
        auto two = LEXY_VERIFY("12");
        CHECK(two.status == test_result::success);
        CHECK(two.value == -11);
        CHECK(two.trace == test_trace());

        auto three = LEXY_VERIFY("123");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 123);
        CHECK(three.trace == test_trace().token("123"));
        auto four = LEXY_VERIFY("1234");
        CHECK(four.status == test_result::success);
        CHECK(four.value == 123);
        CHECK(four.trace == test_trace().token("123"));
    }
}

TEST_CASE("dsl::integer(dsl::n_digits.sep())")
{
    constexpr auto integer = dsl::integer<int>(dsl::n_digits<3>.sep(LEXY_LIT("_")));
    CHECK(lexy::is_rule<decltype(integer)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE("as rule")
    {
        constexpr auto rule = integer;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").recovery());

        auto one = LEXY_VERIFY("1");
        CHECK(one.status == test_result::recovered_error);
        CHECK(one.value == 1);
        CHECK(one.trace
              == test_trace().expected_char_class(1, "digit.decimal").error_token("1").recovery());
        auto two = LEXY_VERIFY("12");
        CHECK(two.status == test_result::recovered_error);
        CHECK(two.value == 12);
        CHECK(two.trace
              == test_trace().expected_char_class(2, "digit.decimal").error_token("12").recovery());

        auto three = LEXY_VERIFY("123");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 123);
        CHECK(three.trace == test_trace().token("123"));
        auto four = LEXY_VERIFY("1234");
        CHECK(four.status == test_result::success);
        CHECK(four.value == 123);
        CHECK(four.trace == test_trace().token("123"));

        auto with_sep = LEXY_VERIFY("1_2_3");
        CHECK(with_sep.status == test_result::success);
        CHECK(with_sep.value == 123);
        CHECK(with_sep.trace == test_trace().token("1_2_3"));

        auto leading_sep = LEXY_VERIFY("_1");
        CHECK(leading_sep.status == test_result::recovered_error);
        CHECK(leading_sep.value == 1);
        CHECK(leading_sep.trace
              == test_trace().expected_char_class(0, "digit.decimal").recovery().error_token("_1"));
        auto trailing_sep = LEXY_VERIFY("1_");
        CHECK(trailing_sep.status == test_result::recovered_error);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace
              == test_trace().expected_char_class(2, "digit.decimal").error_token("1_").recovery());
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(integer);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY("1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == -11);
        CHECK(one.trace == test_trace());
        auto two = LEXY_VERIFY("12");
        CHECK(two.status == test_result::success);
        CHECK(two.value == -11);
        CHECK(two.trace == test_trace());

        auto three = LEXY_VERIFY("123");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 123);
        CHECK(three.trace == test_trace().token("123"));
        auto four = LEXY_VERIFY("1234");
        CHECK(four.status == test_result::success);
        CHECK(four.value == 123);
        CHECK(four.trace == test_trace().token("123"));

        auto with_sep = LEXY_VERIFY("1_2_3");
        CHECK(with_sep.status == test_result::success);
        CHECK(with_sep.value == 123);
        CHECK(with_sep.trace == test_trace().token("1_2_3"));

        auto leading_sep = LEXY_VERIFY("_1");
        CHECK(leading_sep.status == test_result::success);
        CHECK(leading_sep.value == -11);
        CHECK(leading_sep.trace == test_trace());
        auto trailing_sep = LEXY_VERIFY("1_");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == -11);
        CHECK(trailing_sep.trace == test_trace());
    }
}

TEST_CASE("dsl::code_point_id")
{
    static constexpr auto id = dsl::code_point_id<6>;
    CHECK(lexy::is_branch_rule<decltype(id)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return int(lexy::code_point().value()); },
                              [](const char*, lexy::code_point cp) { return int(cp.value()); });

    SUBCASE("as rule")
    {
        constexpr auto rule = id;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.hex").recovery());

        auto latin_small_letter_e_with_acute = LEXY_VERIFY("0000E9");
        CHECK(latin_small_letter_e_with_acute.status == test_result::success);
        CHECK(latin_small_letter_e_with_acute.value == 0x0000E9);
        CHECK(latin_small_letter_e_with_acute.trace == test_trace().token("0000E9"));

        auto euro_sign = LEXY_VERIFY("0020AC");
        CHECK(euro_sign.status == test_result::success);
        CHECK(euro_sign.value == 0x20AC);
        CHECK(euro_sign.trace == test_trace().token("0020AC"));

        auto slightly_smiling_face = LEXY_VERIFY("01F92D");
        CHECK(slightly_smiling_face.status == test_result::success);
        CHECK(slightly_smiling_face.value == 0x1F92D);
        CHECK(slightly_smiling_face.trace == test_trace().token("01F92D"));

        auto extra_digits = LEXY_VERIFY("0000001");
        CHECK(extra_digits.status == test_result::success);
        CHECK(extra_digits.value == 0);
        CHECK(extra_digits.trace == test_trace().token("000000"));

        auto overflow = LEXY_VERIFY("ABCDEF");
        CHECK(overflow.status == test_result::recovered_error);
        CHECK(overflow.value == 0xABCDEF);
        CHECK(overflow.trace == test_trace().token("ABCDEF").error(0, 6, "invalid code point"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(id);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == lexy::code_point().value());
        CHECK(empty.trace == test_trace());

        auto latin_small_letter_e_with_acute = LEXY_VERIFY("0000E9");
        CHECK(latin_small_letter_e_with_acute.status == test_result::success);
        CHECK(latin_small_letter_e_with_acute.value == 0x0000E9);
        CHECK(latin_small_letter_e_with_acute.trace == test_trace().token("0000E9"));

        auto euro_sign = LEXY_VERIFY("0020AC");
        CHECK(euro_sign.status == test_result::success);
        CHECK(euro_sign.value == 0x20AC);
        CHECK(euro_sign.trace == test_trace().token("0020AC"));

        auto slightly_smiling_face = LEXY_VERIFY("01F92D");
        CHECK(slightly_smiling_face.status == test_result::success);
        CHECK(slightly_smiling_face.value == 0x1F92D);
        CHECK(slightly_smiling_face.trace == test_trace().token("01F92D"));

        auto extra_digits = LEXY_VERIFY("0000001");
        CHECK(extra_digits.status == test_result::success);
        CHECK(extra_digits.value == 0);
        CHECK(extra_digits.trace == test_trace().token("000000"));

        auto overflow = LEXY_VERIFY("ABCDEF");
        CHECK(overflow.status == test_result::recovered_error);
        CHECK(overflow.value == 0xABCDEF);
        CHECK(overflow.trace == test_trace().token("ABCDEF").error(0, 6, "invalid code point"));
    }
}

