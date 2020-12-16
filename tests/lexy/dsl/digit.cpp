// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/digit.hpp>

#include "verify.hpp"

namespace
{
template <typename Radix, typename... Digits>
void radix_match(Digits... digits)
{
    for (auto c = 0; c <= 255; ++c)
    {
        auto valid = ((c == digits) || ...);

        const char str[]  = {char(c), '\0'};
        auto       input  = lexy::zstring_input(str);
        auto       reader = input.reader();
        CHECK(lexy::engine_try_match<typename Radix::digit_set>(reader) == valid);

        if (valid)
        {
            CHECK(0 <= Radix::value(c));
            CHECK(Radix::value(c) < Radix::radix);
        }
        else
        {
            INFO(char(c));
            CHECK(Radix::value(c) >= Radix::radix);
        }
    }
}
} // namespace

TEST_CASE("dsl::binary")
{
    using radix = lexy::dsl::binary;
    CHECK(radix::radix == 2);
    CHECK(radix::name() == lexy::_detail::string_view("digit.binary"));

    for (auto digit = 0; digit < 2; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);

    radix_match<radix>('0', '1');
}

TEST_CASE("dsl::octal")
{
    using radix = lexy::dsl::octal;
    CHECK(radix::radix == 8);
    CHECK(radix::name() == lexy::_detail::string_view("digit.octal"));

    for (auto digit = 0; digit < 8; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7');
}

TEST_CASE("dsl::decimal")
{
    using radix = lexy::dsl::decimal;
    CHECK(radix::radix == 10);
    CHECK(radix::name() == lexy::_detail::string_view("digit.decimal"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
}

TEST_CASE("dsl::hex_lower")
{
    using radix = lexy::dsl::hex_lower;
    CHECK(radix::radix == 16);
    CHECK(radix::name() == lexy::_detail::string_view("digit.hex-lower"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value(test_encoding::char_type('a' + digit)) == 10 + digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                       'f');
}

TEST_CASE("dsl::hex_upper")
{
    using radix = lexy::dsl::hex_upper;
    CHECK(radix::radix == 16);
    CHECK(radix::name() == lexy::_detail::string_view("digit.hex-upper"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value(test_encoding::char_type('A' + digit)) == 10 + digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                       'F');
}

TEST_CASE("dsl::hex")
{
    using radix = lexy::dsl::hex;
    CHECK(radix::radix == 16);
    CHECK(radix::name() == lexy::_detail::string_view("digit.hex"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value(test_encoding::char_type('A' + digit)) == 10 + digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value(test_encoding::char_type('a' + digit)) == 10 + digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                       'f', 'A', 'B', 'C', 'D', 'E', 'F');
}

TEST_CASE("dsl::zero")
{
    constexpr auto rule = lexy::dsl::zero;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            CONSTEXPR_CHECK(cur == str + 1);
            return 0;
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.character_class() == "digit.zero");
            return -1;
        }
    };

    constexpr auto empty = verify<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto zero = verify<callback>(rule, "0");
    CHECK(zero == 0);
    constexpr auto zero_zero = verify<callback>(rule, "00");
    CHECK(zero_zero == 0);

    constexpr auto nine = verify<callback>(rule, "9");
    CHECK(nine == -1);
}

TEST_CASE("dsl::digit")
{
    constexpr auto rule = lexy::dsl::digit<lexy::dsl::octal>;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            CONSTEXPR_CHECK(cur == str + 1);
            return *str - '0';
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.character_class() == "digit.octal");
            return -1;
        }
    };

    constexpr auto empty = verify<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto zero = verify<callback>(rule, "0");
    CHECK(zero == 0);

    constexpr auto six = verify<callback>(rule, "6");
    CHECK(six == 6);

    constexpr auto three_seven = verify<callback>(rule, "37");
    CHECK(three_seven == 3);

    constexpr auto nine = verify<callback>(rule, "9");
    CHECK(nine == -1);
}

TEST_CASE("dsl::digits")
{
    SUBCASE("basic")
    {
        constexpr auto rule = lexy::dsl::digits<>;
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
                CONSTEXPR_CHECK(e.character_class() == "digit.decimal");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = verify<callback>(rule, "0");
        CHECK(zero == 1);

        constexpr auto one = verify<callback>(rule, "1");
        CHECK(one == 1);

        constexpr auto one_zero_one = verify<callback>(rule, "101");
        CHECK(one_zero_one == 3);

        constexpr auto zero_zero_seven = verify<callback>(rule, "007");
        CHECK(zero_zero_seven == 3);
    }
    SUBCASE("no leading zero")
    {
        constexpr auto rule = lexy::dsl::digits<>.no_leading_zero();
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
                CONSTEXPR_CHECK(e.character_class() == "digit.decimal");
                return -1;
            }
            constexpr int error(test_error<lexy::forbidden_leading_zero> e)
            {
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = verify<callback>(rule, "0");
        CHECK(zero == 1);

        constexpr auto one = verify<callback>(rule, "1");
        CHECK(one == 1);

        constexpr auto one_zero_one = verify<callback>(rule, "101");
        CHECK(one_zero_one == 3);

        constexpr auto zero_zero_seven = verify<callback>(rule, "007");
        CHECK(zero_zero_seven == -2);
    }
    SUBCASE("sep")
    {
        constexpr auto rule = lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick);
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
                CONSTEXPR_CHECK(e.character_class() == "digit.decimal");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = verify<callback>(rule, "0");
        CHECK(zero == 1);

        constexpr auto one = verify<callback>(rule, "1");
        CHECK(one == 1);

        constexpr auto one_zero_one = verify<callback>(rule, "1'01");
        CHECK(one_zero_one == 4);

        constexpr auto zero_zero_seven = verify<callback>(rule, "00'7");
        CHECK(zero_zero_seven == 4);

        constexpr auto leading_tick = verify<callback>(rule, "'0");
        CHECK(leading_tick == -1);
        constexpr auto trailing_tick = verify<callback>(rule, "0'");
        CHECK(trailing_tick == -1);
    }
    SUBCASE("sep + no leading zero")
    {
        constexpr auto rule = lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick).no_leading_zero();
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        constexpr auto equivalent
            = lexy::dsl::digits<>.no_leading_zero().sep(lexy::dsl::digit_sep_tick);
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

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
                CONSTEXPR_CHECK(e.character_class() == "digit.decimal");
                return -1;
            }
            constexpr int error(test_error<lexy::forbidden_leading_zero> e)
            {
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = verify<callback>(rule, "0");
        CHECK(zero == 1);

        constexpr auto one = verify<callback>(rule, "1");
        CHECK(one == 1);

        constexpr auto one_zero_one = verify<callback>(rule, "1'01");
        CHECK(one_zero_one == 4);

        constexpr auto zero_zero_seven = verify<callback>(rule, "00'7");
        CHECK(zero_zero_seven == -2);

        constexpr auto leading_tick = verify<callback>(rule, "'0");
        CHECK(leading_tick == -1);
        constexpr auto trailing_tick = verify<callback>(rule, "0'");
        CHECK(trailing_tick == -2);

        constexpr auto zero_tick_one = verify<callback>(rule, "0'1");
        CHECK(zero_tick_one == -2);
    }
}

TEST_CASE("dsl::n_digits")
{
    SUBCASE("basic")
    {
        constexpr auto rule = lexy::dsl::n_digits<3>;
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
                CONSTEXPR_CHECK(e.character_class() == "digit.decimal");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);
        constexpr auto zero = verify<callback>(rule, "0");
        CHECK(zero == -1);

        constexpr auto one_zero_one = verify<callback>(rule, "101");
        CHECK(one_zero_one == 3);

        constexpr auto zero_zero_seven = verify<callback>(rule, "007");
        CHECK(zero_zero_seven == 3);

        constexpr auto four_digits = verify<callback>(rule, "1234");
        CHECK(four_digits == 3);
    }
    SUBCASE("sep")
    {
        constexpr auto rule = lexy::dsl::n_digits<3>.sep(lexy::dsl::digit_sep_tick);
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
                CONSTEXPR_CHECK(e.character_class() == "digit.decimal");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);
        constexpr auto zero = verify<callback>(rule, "0");
        CHECK(zero == -1);

        constexpr auto one_zero_one = verify<callback>(rule, "1'01");
        CHECK(one_zero_one == 4);

        constexpr auto zero_zero_seven = verify<callback>(rule, "00'7");
        CHECK(zero_zero_seven == 4);

        constexpr auto leading_tick = verify<callback>(rule, "'0");
        CHECK(leading_tick == -1);
        constexpr auto trailing_tick = verify<callback>(rule, "123'");
        CHECK(trailing_tick == 3);

        constexpr auto four_digits = verify<callback>(rule, "1'2'3'4");
        CHECK(four_digits == 5);
    }
}

