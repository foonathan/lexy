// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
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
        CHECK(Radix::template match<lexy::ascii_encoding>(char(c)) == valid);

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
        CHECK(radix::value('0' + digit) == digit);

    radix_match<radix>('0', '1');
}

TEST_CASE("dsl::octal")
{
    using radix = lexy::dsl::octal;
    CHECK(radix::radix == 8);
    CHECK(radix::name() == lexy::_detail::string_view("digit.octal"));

    for (auto digit = 0; digit < 8; ++digit)
        CHECK(radix::value('0' + digit) == digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7');
}

TEST_CASE("dsl::decimal")
{
    using radix = lexy::dsl::decimal;
    CHECK(radix::radix == 10);
    CHECK(radix::name() == lexy::_detail::string_view("digit.decimal"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value('0' + digit) == digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
}

TEST_CASE("dsl::hex_lower")
{
    using radix = lexy::dsl::hex_lower;
    CHECK(radix::radix == 16);
    CHECK(radix::name() == lexy::_detail::string_view("digit.hex-lower"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value('0' + digit) == digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value('a' + digit) == 10 + digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                       'f');
}

TEST_CASE("dsl::hex_upper")
{
    using radix = lexy::dsl::hex_upper;
    CHECK(radix::radix == 16);
    CHECK(radix::name() == lexy::_detail::string_view("digit.hex-upper"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value('0' + digit) == digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value('A' + digit) == 10 + digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                       'F');
}

TEST_CASE("dsl::hex")
{
    using radix = lexy::dsl::hex;
    CHECK(radix::radix == 16);
    CHECK(radix::name() == lexy::_detail::string_view("digit.hex"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value('0' + digit) == digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value('A' + digit) == 10 + digit);
    for (auto digit = 0; digit < 6; ++digit)
        CHECK(radix::value('a' + digit) == 10 + digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                       'f', 'A', 'B', 'C', 'D', 'E', 'F');
}

TEST_CASE("dsl::zero")
{
    constexpr auto rule = dsl::zero;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.zero").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto zerozero = LEXY_VERIFY("00");
    CHECK(zerozero.status == test_result::success);
    CHECK(zerozero.trace == test_trace().token("digits", "0"));

    auto nine = LEXY_VERIFY("9");
    CHECK(nine.status == test_result::fatal_error);
    CHECK(nine.trace == test_trace().expected_char_class(0, "digit.zero").cancel());

    auto utf16 = LEXY_VERIFY(u"0");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "0"));
}

TEST_CASE("dsl::digit")
{
    // Exhaustive tests of base done above.
    constexpr auto rule = dsl::digit<dsl::octal>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.octal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));

    auto nine = LEXY_VERIFY("9");
    CHECK(nine.status == test_result::fatal_error);
    CHECK(nine.trace == test_trace().expected_char_class(0, "digit.octal").cancel());

    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "3"));

    auto utf16 = LEXY_VERIFY(u"0");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "0"));
}

TEST_CASE("dsl::digits<>")
{
    constexpr auto rule = dsl::digits<>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::success);
    CHECK(zero_zero_seven.trace == test_trace().token("digits", "007"));

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("dsl::digits<>.no_leading_zero()")
{
    constexpr auto rule = dsl::digits<>.no_leading_zero();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::fatal_error);
    CHECK(zero_zero_seven.trace
          == test_trace().error(0, 1, "forbidden leading zero").error_token("0").cancel());

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("dsl::digits<>.sep()")
{
    constexpr auto rule = dsl::digits<>.sep(LEXY_LIT("_"));
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::success);
    CHECK(zero_zero_seven.trace == test_trace().token("digits", "007"));

    auto with_sep = LEXY_VERIFY("1_2_3");
    CHECK(with_sep.status == test_result::success);
    CHECK(with_sep.trace == test_trace().token("digits", "1_2_3"));

    auto leading_sep = LEXY_VERIFY("_1");
    CHECK(leading_sep.status == test_result::fatal_error);
    CHECK(leading_sep.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
    auto trailing_sep = LEXY_VERIFY("1_");
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace
          == test_trace().expected_char_class(2, "digit.decimal").error_token("1_").cancel());

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("dsl::digits<>.sep().no_leading_zero")
{
    constexpr auto rule = dsl::digits<>.sep(LEXY_LIT("_")).no_leading_zero();
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, dsl::digits<>.no_leading_zero().sep(LEXY_LIT("_"))));

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::fatal_error);
    CHECK(zero_zero_seven.trace
          == test_trace().error(0, 1, "forbidden leading zero").error_token("0").cancel());
    auto zero_sep_zero_seven = LEXY_VERIFY("0_07");
    CHECK(zero_sep_zero_seven.status == test_result::fatal_error);
    CHECK(zero_sep_zero_seven.trace
          == test_trace().error(0, 1, "forbidden leading zero").error_token("0").cancel());

    auto with_sep = LEXY_VERIFY("1_2_3");
    CHECK(with_sep.status == test_result::success);
    CHECK(with_sep.trace == test_trace().token("digits", "1_2_3"));

    auto leading_sep = LEXY_VERIFY("_1");
    CHECK(leading_sep.status == test_result::fatal_error);
    CHECK(leading_sep.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
    auto trailing_sep = LEXY_VERIFY("1_");
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace
          == test_trace().expected_char_class(2, "digit.decimal").error_token("1_").cancel());

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("digit separators")
{
    CHECK(equivalent_rules(dsl::digit_sep_tick, LEXY_LIT("'")));
    CHECK(equivalent_rules(dsl::digit_sep_underscore, LEXY_LIT("_")));
}

TEST_CASE("dsl::n_digits")
{
    constexpr auto rule = dsl::n_digits<3>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto one = LEXY_VERIFY("1");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace
          == test_trace().expected_char_class(1, "digit.decimal").error_token("1").cancel());
    auto two = LEXY_VERIFY("12");
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace
          == test_trace().expected_char_class(2, "digit.decimal").error_token("12").cancel());

    auto three = LEXY_VERIFY("123");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().token("digits", "123"));
    auto four = LEXY_VERIFY("1234");
    CHECK(four.status == test_result::success);
    CHECK(four.trace == test_trace().token("digits", "123"));
}

TEST_CASE("dsl::n_digits.sep()")
{
    constexpr auto rule = dsl::n_digits<3>.sep(LEXY_LIT("_"));
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto one = LEXY_VERIFY("1");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace
          == test_trace().expected_char_class(1, "digit.decimal").error_token("1").cancel());
    auto two = LEXY_VERIFY("12");
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace
          == test_trace().expected_char_class(2, "digit.decimal").error_token("12").cancel());

    auto three = LEXY_VERIFY("123");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().token("digits", "123"));
    auto four = LEXY_VERIFY("1234");
    CHECK(four.status == test_result::success);
    CHECK(four.trace == test_trace().token("digits", "123"));

    auto with_sep = LEXY_VERIFY("1_2_3");
    CHECK(with_sep.status == test_result::success);
    CHECK(with_sep.trace == test_trace().token("digits", "1_2_3"));

    auto leading_sep = LEXY_VERIFY("_1");
    CHECK(leading_sep.status == test_result::fatal_error);
    CHECK(leading_sep.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
    auto trailing_sep = LEXY_VERIFY("1_");
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace
          == test_trace().expected_char_class(2, "digit.decimal").error_token("1_").cancel());
}

