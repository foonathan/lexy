// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include <doctest.h>
#include <lexy/dsl/literal.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::while_one")
{
    constexpr auto result     = while_one(LEXY_LIT("abc"));
    constexpr auto equivalent = LEXY_LIT("abc") >> while_(LEXY_LIT("abc"));
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

    CHECK(!lexy::match(lexy::zstring_input(""), result));
    CHECK(lexy::match(lexy::zstring_input("abc"), result));
    CHECK(lexy::match(lexy::zstring_input("abcabc"), result));
}

TEST_CASE("dsl::do_while")
{
    constexpr auto result     = do_while(LEXY_LIT("a"), LEXY_LIT("b"));
    constexpr auto equivalent = LEXY_LIT("a") >> while_(LEXY_LIT("b") >> LEXY_LIT("a"));
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

    CHECK(!lexy::match(lexy::zstring_input(""), result));
    CHECK(lexy::match(lexy::zstring_input("a"), result));
    CHECK(lexy::match(lexy::zstring_input("aba"), result));
    CHECK(lexy::match(lexy::zstring_input("ababa"), result));
}

