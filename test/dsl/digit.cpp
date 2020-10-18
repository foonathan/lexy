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
        CHECK(Radix::template match<test_encoding>(test_encoding::int_type{c}) == valid);
        CHECK(Radix::template match_zero<test_encoding>(test_encoding::int_type{c}) == (c == '0'));

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

TEST_CASE("dsl::digit")
{
    constexpr auto atom = lexy::dsl::digit<lexy::dsl::octal>;
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.position() == empty.input);
    CHECK(empty.error.character_class() == "digit.octal");

    constexpr auto zero = atom_matches(atom, "0");
    CHECK(zero);
    CHECK(zero.count == 1);

    constexpr auto six = atom_matches(atom, "6");
    CHECK(six);
    CHECK(six.count == 1);

    constexpr auto three_seven = atom_matches(atom, "37");
    CHECK(three_seven);
    CHECK(three_seven.count == 1);

    constexpr auto nine = atom_matches(atom, "9");
    CHECK(!nine);
    CHECK(nine.count == 0);
    CHECK(nine.error.position() == nine.input);
    CHECK(nine.error.character_class() == "digit.octal");
}

TEST_CASE("dsl::digit.zero()")
{
    constexpr auto atom = lexy::dsl::digit<lexy::dsl::octal>.zero();
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.position() == empty.input);
    CHECK(empty.error.character_class() == "digit.zero");

    constexpr auto zero = atom_matches(atom, "0");
    CHECK(zero);
    CHECK(zero.count == 1);

    constexpr auto zero_zero = atom_matches(atom, "00");
    CHECK(zero_zero);
    CHECK(zero_zero.count == 1);

    constexpr auto nine = atom_matches(atom, "9");
    CHECK(!nine);
    CHECK(nine.count == 0);
    CHECK(nine.error.position() == nine.input);
    CHECK(nine.error.character_class() == "digit.zero");
}

TEST_CASE("dsl::digit.non_zero()")
{
    constexpr auto atom = lexy::dsl::digit<lexy::dsl::octal>.non_zero();
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.position() == empty.input);
    CHECK(empty.error.character_class() == "digit.octal");

    constexpr auto six = atom_matches(atom, "6");
    CHECK(six);
    CHECK(six.count == 1);

    constexpr auto three_seven = atom_matches(atom, "37");
    CHECK(three_seven);
    CHECK(three_seven.count == 1);

    constexpr auto nine = atom_matches(atom, "9");
    CHECK(!nine);
    CHECK(nine.count == 0);
    CHECK(nine.error.position() == nine.input);
    CHECK(nine.error.character_class() == "digit.octal");

    constexpr auto zero = atom_matches(atom, "0");
    CHECK(!zero);
    CHECK(zero.count == 0);
    CHECK(zero.error.position() == zero.input);
    CHECK(zero.error.character_class() == "digit.non-zero");
}

TEST_CASE("dsl::digits")
{
    SUBCASE("basic")
    {
        constexpr auto pattern = lexy::dsl::digits<>;
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto zero = pattern_matches(pattern, "0");
        CHECK(zero);
        CHECK(zero.match() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "101");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match() == "101");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "007");
        CHECK(zero_zero_seven);
        CHECK(zero_zero_seven.match() == "007");
    }
    SUBCASE("no leading zero")
    {
        constexpr auto pattern = lexy::dsl::digits<>.no_leading_zero();
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto zero = pattern_matches(pattern, "0");
        CHECK(zero);
        CHECK(zero.match() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "101");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match() == "101");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "007");
        CHECK(!zero_zero_seven);
    }
    SUBCASE("sep")
    {
        constexpr auto pattern = lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick);
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto zero = pattern_matches(pattern, "0");
        CHECK(zero);
        CHECK(zero.match() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "1'01");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match() == "1'01");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "00'7");
        CHECK(zero_zero_seven);
        CHECK(zero_zero_seven.match() == "00'7");

        constexpr auto leading_tick = pattern_matches(pattern, "'0");
        CHECK(!leading_tick);
        constexpr auto trailing_tick = pattern_matches(pattern, "0'");
        CHECK(trailing_tick);
        CHECK(trailing_tick.match() == "0");
    }
    SUBCASE("sep + no leading zero")
    {
        constexpr auto pattern
            = lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick).no_leading_zero();
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto zero = pattern_matches(pattern, "0");
        CHECK(zero);
        CHECK(zero.match() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "1'01");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match() == "1'01");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "00'7");
        CHECK(!zero_zero_seven);

        constexpr auto leading_tick = pattern_matches(pattern, "'0");
        CHECK(!leading_tick);
        constexpr auto trailing_tick = pattern_matches(pattern, "0'");
        CHECK(trailing_tick);
        CHECK(trailing_tick.match() == "0");
    }
}

