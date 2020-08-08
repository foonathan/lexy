// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/match.hpp>

#include <doctest.h>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/while.hpp>
#include <lexy/input/string_input.hpp>

TEST_CASE("pattern_match")
{
    SUBCASE("basic")
    {
        constexpr auto result
            = lexy::pattern_match(lexy::zstring_input("abc"), [] { return LEXY_LIT("abc"); });
        CHECK(result);
        CHECK(result.id() == 0);
        CHECK(result.match().string_view() == "abc");
    }
    SUBCASE("partial")
    {
        auto input = lexy::zstring_input("abcab");

        auto result = lexy::pattern_match(lexy::zstring_input("abc"),
                                          [] { return while_(LEXY_LIT("abc")); });
        CHECK(result);
        CHECK(result.id() == 0);
        CHECK(result.match().string_view() == "abc");

        CHECK(input.peek() == 'a');
    }
}

