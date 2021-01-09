// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/terminator.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/while.hpp>

TEST_CASE("dsl::terminator")
{
    constexpr auto terminator = lexy::dsl::terminator(LEXY_LIT(";"));
    constexpr auto inner      = LEXY_LIT("abc");

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                using return_type = void;

                LEXY_VERIFY_FN void operator()() {}

                LEXY_VERIFY_FN void finish() && {}
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }
        LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
        {
            return -1;
        }
    };

    SUBCASE("basic")
    {
        static constexpr auto rule       = terminator(inner);
        constexpr auto        equivalent = inner + LEXY_LIT(";");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto result = LEXY_VERIFY("abc;");
        CHECK(result == 4);
    }

    SUBCASE("while")
    {
        static constexpr auto rule = terminator.while_(inner);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abcabc;");
        CHECK(two == 7);
    }
    SUBCASE("while_one")
    {
        static constexpr auto rule = terminator.while_one(inner);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abcabc;");
        CHECK(two == 7);
    }
    SUBCASE("opt")
    {
        static constexpr auto rule = terminator.opt(inner);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
    }
    SUBCASE("list - no sep")
    {
        static constexpr auto rule = terminator.list(inner);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abcabc;");
        CHECK(two == 7);
    }
    SUBCASE("list - sep")
    {
        static constexpr auto rule = terminator.list(inner, lexy::dsl::sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);
        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing == -1);
    }
    SUBCASE("list - trailing sep")
    {
        static constexpr auto rule = terminator.list(inner, lexy::dsl::trailing_sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);
        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing == 9);
    }
    SUBCASE("opt_list - no sep")
    {
        static constexpr auto rule = terminator.opt_list(inner);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abcabc;");
        CHECK(two == 7);
    }
    SUBCASE("opt_list - sep")
    {
        static constexpr auto rule = terminator.opt_list(inner, lexy::dsl::sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);
        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing == -1);
    }
    SUBCASE("opt_list - trailing sep")
    {
        static constexpr auto rule
            = terminator.opt_list(inner, lexy::dsl::trailing_sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);
        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing == 9);
    }
}

