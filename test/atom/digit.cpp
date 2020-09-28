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

TEST_CASE("radix: binary")
{
    using radix = lexy::dsl::binary;
    CHECK(radix::radix == 2);
    CHECK(radix::name() == lexy::_detail::string_view("digit.binary"));

    for (auto digit = 0; digit < 2; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);

    radix_match<radix>('0', '1');
}

TEST_CASE("radix: octal")
{
    using radix = lexy::dsl::octal;
    CHECK(radix::radix == 8);
    CHECK(radix::name() == lexy::_detail::string_view("digit.octal"));

    for (auto digit = 0; digit < 8; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7');
}

TEST_CASE("radix: decimal")
{
    using radix = lexy::dsl::decimal;
    CHECK(radix::radix == 10);
    CHECK(radix::name() == lexy::_detail::string_view("digit.decimal"));

    for (auto digit = 0; digit < 10; ++digit)
        CHECK(radix::value(test_encoding::char_type('0' + digit)) == digit);

    radix_match<radix>('0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
}

TEST_CASE("radix: hex_lower")
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

TEST_CASE("radix: hex_upper")
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

TEST_CASE("radix: hex")
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

TEST_CASE("atom: digit")
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

TEST_CASE("atom: digit.zero")
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

TEST_CASE("atom: digit.non_zero")
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

