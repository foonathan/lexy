// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/terminator.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/match.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/while.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::terminator")
{
    constexpr auto terminator = lexy::dsl::terminator(LEXY_LIT(";"));
    // We want to make sure that inner does not have a matcher.
    constexpr auto inner = LEXY_LIT("abc") + lexy::dsl::id<0>;

    SUBCASE("whitespace")
    {
        constexpr auto result     = terminator[LEXY_LIT(" ")](inner);
        constexpr auto equivalent = inner + whitespaced(LEXY_LIT(";"), LEXY_LIT(" "));
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("abc ;"), result + lexy::dsl::eof));
    }

    SUBCASE("while")
    {
        constexpr auto result = terminator.while_(match(inner));

        CHECK(lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abcabc;"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abcabcabc;"), result + lexy::dsl::eof));
    }
    SUBCASE("while_one")
    {
        constexpr auto result = terminator.while_one(match(inner));

        CHECK(!lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abcabc;"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abcabcabc;"), result + lexy::dsl::eof));
    }
    SUBCASE("opt")
    {
        constexpr auto result = terminator.opt(inner);

        CHECK(lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
    }
    SUBCASE("list")
    {
        SUBCASE("no sep")
        {
            constexpr auto result = terminator.list(inner);

            CHECK(!lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abcabc;"), result + lexy::dsl::eof));
        }
        SUBCASE("sep")
        {
            constexpr auto result = terminator.list(inner, sep(LEXY_LIT(",")));

            CHECK(!lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc,abc;"), result + lexy::dsl::eof));
            CHECK(!lexy::match(lexy::zstring_input("abc,abc,;"), result + lexy::dsl::eof));
        }
        SUBCASE("trailing sep")
        {
            constexpr auto result = terminator.list(inner, trailing_sep(LEXY_LIT(",")));

            CHECK(!lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc,abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc,abc,;"), result + lexy::dsl::eof));
        }
    }
    SUBCASE("opt_list")
    {
        SUBCASE("no sep")
        {
            constexpr auto result = terminator.opt_list(inner);

            CHECK(lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abcabc;"), result + lexy::dsl::eof));
        }
        SUBCASE("sep")
        {
            constexpr auto result = terminator.opt_list(inner, sep(LEXY_LIT(",")));

            CHECK(lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc,abc;"), result + lexy::dsl::eof));
            CHECK(!lexy::match(lexy::zstring_input("abc,abc,;"), result + lexy::dsl::eof));
        }
        SUBCASE("trailing sep")
        {
            constexpr auto result = terminator.opt_list(inner, trailing_sep(LEXY_LIT(",")));

            CHECK(lexy::match(lexy::zstring_input(";"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc,abc;"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("abc,abc,;"), result + lexy::dsl::eof));
        }
    }
}

