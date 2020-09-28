// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/digit.hpp>

#include "verify.hpp"

// Note: we're only having a pattern check as it is just sugar for other rules.
TEST_CASE("pattern: digits")
{
    SUBCASE("basic")
    {
        constexpr auto pattern = lexy::dsl::digits<>;
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto zero = pattern_matches(pattern, "0");
        CHECK(zero);
        CHECK(zero.match().string_view() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match().string_view() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "101");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match().string_view() == "101");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "007");
        CHECK(zero_zero_seven);
        CHECK(zero_zero_seven.match().string_view() == "007");
    }
    SUBCASE("no leading zero")
    {
        constexpr auto pattern = lexy::dsl::digits<>.no_leading_zero();
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto zero = pattern_matches(pattern, "0");
        CHECK(zero);
        CHECK(zero.match().string_view() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match().string_view() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "101");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match().string_view() == "101");

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
        CHECK(zero.match().string_view() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match().string_view() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "1'01");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match().string_view() == "1'01");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "00'7");
        CHECK(zero_zero_seven);
        CHECK(zero_zero_seven.match().string_view() == "00'7");

        constexpr auto leading_tick = pattern_matches(pattern, "'0");
        CHECK(!leading_tick);
        constexpr auto trailing_tick = pattern_matches(pattern, "0'");
        CHECK(trailing_tick);
        CHECK(trailing_tick.match().string_view() == "0");
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
        CHECK(zero.match().string_view() == "0");

        constexpr auto one = pattern_matches(pattern, "1");
        CHECK(one);
        CHECK(one.match().string_view() == "1");

        constexpr auto one_zero_one = pattern_matches(pattern, "1'01");
        CHECK(one_zero_one);
        CHECK(one_zero_one.match().string_view() == "1'01");

        constexpr auto zero_zero_seven = pattern_matches(pattern, "00'7");
        CHECK(!zero_zero_seven);

        constexpr auto leading_tick = pattern_matches(pattern, "'0");
        CHECK(!leading_tick);
        constexpr auto trailing_tick = pattern_matches(pattern, "0'");
        CHECK(trailing_tick);
        CHECK(trailing_tick.match().string_view() == "0");
    }
}

