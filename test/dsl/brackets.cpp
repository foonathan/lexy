// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/brackets.hpp>

#include <doctest.h>
#include <lexy/dsl/label.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::brackets")
{
    // We want to make sure that inner does not have a matcher.
    constexpr auto inner = LEXY_LIT("abc") + lexy::dsl::id<0>;

    SUBCASE("round_brackets")
    {
        constexpr auto result     = lexy::dsl::round_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("(abc)"), result));
    }
    SUBCASE("square_brackets")
    {
        constexpr auto result     = lexy::dsl::square_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("[") >> inner + LEXY_LIT("]");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("[abc]"), result));
    }
    SUBCASE("curly_brackets")
    {
        constexpr auto result     = lexy::dsl::curly_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("{") >> inner + LEXY_LIT("}");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("{abc}"), result));
    }
    SUBCASE("angle_brackets")
    {
        constexpr auto result     = lexy::dsl::angle_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("<") >> inner + LEXY_LIT(">");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("<abc>"), result));
    }
    SUBCASE("parentheses")
    {
        constexpr auto result     = lexy::dsl::parenthesized(inner);
        constexpr auto equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("(abc)"), result));
    }

    SUBCASE("whitespace")
    {
        constexpr auto result = lexy::dsl::parenthesized[LEXY_LIT(" ")](inner);
        constexpr auto equivalent
            = LEXY_LIT(" ") + LEXY_LIT("(") >> inner + LEXY_LIT(" ") + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input(" (abc )"), result));
    }
    SUBCASE("opt")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")");

        constexpr auto result     = lexy::dsl::parenthesized.opt(inner);
        constexpr auto equivalent = o >> opt(!c >> inner + c);
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("()"), result));
        CHECK(lexy::match(lexy::zstring_input("(abc)"), result));
    }
    SUBCASE("list")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")");

        SUBCASE("no sep")
        {
            constexpr auto result     = lexy::dsl::parenthesized.list(inner);
            constexpr auto equivalent = o >> list(!c >> inner);
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("(abc)"), result));
            CHECK(lexy::match(lexy::zstring_input("(abcabc)"), result));
        }
        SUBCASE("sep")
        {
            constexpr auto result
                = lexy::dsl::parenthesized.list(inner, trailing_sep(LEXY_LIT(",")));
            constexpr auto equivalent = o >> list(!c >> inner, trailing_sep(LEXY_LIT(",")));
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("(abc)"), result));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc,)"), result));
        }
    }
}
