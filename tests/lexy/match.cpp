// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/match.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/input/string_input.hpp>

TEST_CASE("match")
{
    SUBCASE("pattern")
    {
        SUBCASE("match")
        {
            auto input  = lexy::zstring_input("abc");
            auto result = lexy::match(input, LEXY_LIT("abc"));
            CHECK(result);
        }
        SUBCASE("no match")
        {
            auto input  = lexy::zstring_input("def");
            auto result = lexy::match(input, LEXY_LIT("abc"));
            CHECK(!result);
        }
        SUBCASE("partial match")
        {
            auto input  = lexy::zstring_input("abc123");
            auto result = lexy::match(input, LEXY_LIT("abc"));
            CHECK(result);
        }
    }
    SUBCASE("rule")
    {
        SUBCASE("match one")
        {
            auto input  = lexy::zstring_input("abc");
            auto result = lexy::match(input, list(LEXY_LIT("abc")));
            CHECK(result);
        }
        SUBCASE("match twice")
        {
            auto input  = lexy::zstring_input("abcabc");
            auto result = lexy::match(input, list(LEXY_LIT("abc")));
            CHECK(result);
        }
        SUBCASE("no match")
        {
            auto input  = lexy::zstring_input("def");
            auto result = lexy::match(input, list(LEXY_LIT("abc")));
            CHECK(!result);
        }
        SUBCASE("partial match")
        {
            auto input  = lexy::zstring_input("abc123");
            auto result = lexy::match(input, list(LEXY_LIT("abc")));
            CHECK(result);
        }
    }
}

