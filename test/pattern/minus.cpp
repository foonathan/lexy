// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/minus.hpp>

#include "verify.hpp"
#include <lexy/dsl/while.hpp>

TEST_CASE("pattern: minus")
{
    SUBCASE("single")
    {
        constexpr auto pattern = while_(LEXY_LIT("a")) - LEXY_LIT("aa");
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match().string_view() == "a");
        constexpr auto aaa = pattern_matches(pattern, "aaa");
        CHECK(aaa);
        CHECK(aaa.match().string_view() == "aaa");

        constexpr auto aa = pattern_matches(pattern, "aa");
        CHECK(!aa);
        CHECK(aa.match().empty());
    }
    SUBCASE("sequence")
    {
        constexpr auto pattern = while_(LEXY_LIT("a")) - LEXY_LIT("a") - LEXY_LIT("aa");
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(!a);
        CHECK(a.match().empty());
        constexpr auto aa = pattern_matches(pattern, "aa");
        CHECK(!aa);
        CHECK(aa.match().empty());

        constexpr auto aaa = pattern_matches(pattern, "aaa");
        CHECK(aaa);
        CHECK(aaa.match().string_view() == "aaa");
    }
}

