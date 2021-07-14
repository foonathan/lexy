// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/action/match.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
struct production
{
    static constexpr auto rule = list(LEXY_LIT("abc"));
};
} // namespace

TEST_CASE("match")
{
    SUBCASE("match one")
    {
        auto input  = lexy::zstring_input("abc");
        auto result = lexy::match<production>(input);
        CHECK(result);
    }
    SUBCASE("match twice")
    {
        auto input  = lexy::zstring_input("abcabc");
        auto result = lexy::match<production>(input);
        CHECK(result);
    }
    SUBCASE("no match")
    {
        auto input  = lexy::zstring_input("def");
        auto result = lexy::match<production>(input);
        CHECK(!result);
    }
    SUBCASE("partial match")
    {
        auto input  = lexy::zstring_input("abc123");
        auto result = lexy::match<production>(input);
        CHECK(result);
    }
}

