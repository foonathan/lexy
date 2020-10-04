// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/times.hpp>

#include "verify.hpp"

TEST_CASE("pattern: times")
{
    SUBCASE("no sep")
    {
        constexpr auto pattern = twice(LEXY_LIT("abc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto zero = pattern_matches(pattern, "");
        CHECK(!zero);
        constexpr auto one = pattern_matches(pattern, "abc");
        CHECK(!one);

        constexpr auto two = pattern_matches(pattern, "abcabc");
        CHECK(two);
        CHECK(two.match().string_view() == "abcabc");
    }
    SUBCASE("sep")
    {
        constexpr auto pattern = twice(LEXY_LIT("abc"), sep(LEXY_LIT(",")));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto zero = pattern_matches(pattern, "");
        CHECK(!zero);
        constexpr auto one = pattern_matches(pattern, "abc");
        CHECK(!one);

        constexpr auto two = pattern_matches(pattern, "abc,abc");
        CHECK(two);
        CHECK(two.match().string_view() == "abc,abc");

        constexpr auto no_sep = pattern_matches(pattern, "abcabc");
        CHECK(!no_sep);

        constexpr auto trailing_sep = pattern_matches(pattern, "abc,abc,");
        CHECK(trailing_sep);
        CHECK(trailing_sep.match().string_view() == "abc,abc");
    }
    SUBCASE("trailing_sep")
    {
        constexpr auto pattern = twice(LEXY_LIT("abc"), trailing_sep(LEXY_LIT(",")));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto zero = pattern_matches(pattern, "");
        CHECK(!zero);
        constexpr auto one = pattern_matches(pattern, "abc");
        CHECK(!one);

        constexpr auto two = pattern_matches(pattern, "abc,abc");
        CHECK(two);
        CHECK(two.match().string_view() == "abc,abc");

        constexpr auto no_sep = pattern_matches(pattern, "abcabc");
        CHECK(!no_sep);

        constexpr auto trailing_sep = pattern_matches(pattern, "abc,abc,");
        CHECK(trailing_sep);
        CHECK(trailing_sep.match().string_view() == "abc,abc,");
    }
}

