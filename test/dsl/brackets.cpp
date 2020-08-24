// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/brackets.hpp>

#include <doctest.h>

TEST_CASE("dsl::bracketed")
{
    constexpr auto inner = LEXY_LIT("abc");

    SUBCASE("round_brackets")
    {
        constexpr auto result     = lexy::dsl::round_brackets(inner);
        constexpr auto equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("square_brackets")
    {
        constexpr auto result     = lexy::dsl::square_brackets(inner);
        constexpr auto equivalent = LEXY_LIT("[") >> inner + LEXY_LIT("]");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("curly_brackets")
    {
        constexpr auto result     = lexy::dsl::curly_brackets(inner);
        constexpr auto equivalent = LEXY_LIT("{") >> inner + LEXY_LIT("}");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("angle_brackets")
    {
        constexpr auto result     = lexy::dsl::angle_brackets(inner);
        constexpr auto equivalent = LEXY_LIT("<") >> inner + LEXY_LIT(">");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("parentheses")
    {
        constexpr auto result     = lexy::dsl::parentheses(inner);
        constexpr auto equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }

    SUBCASE("whitespace")
    {
        constexpr auto result = lexy::dsl::parentheses[LEXY_LIT(" ")](inner);
        constexpr auto equivalent
            = LEXY_LIT(" ") + LEXY_LIT("(") >> inner + LEXY_LIT(" ") + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
}

