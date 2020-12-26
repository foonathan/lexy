// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/times.hpp>

#include "verify.hpp"
#include <lexy/dsl/value.hpp>

TEST_CASE("times")
{
    SUBCASE("no sep")
    {
        static constexpr auto rule = twice(LEXY_LIT("abc") + lexy::dsl::value_c<1>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::twice<int> value)
            {
                CONSTEXPR_CHECK(cur - str == 6);
                CONSTEXPR_CHECK(value[0] == 1);
                CONSTEXPR_CHECK(value[1] == 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto zero = LEXY_VERIFY("");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc");
        CHECK(one == -1);

        auto two = LEXY_VERIFY("abcabc");
        CHECK(two == 0);
    }
    SUBCASE("sep")
    {
        static constexpr auto rule
            = twice(LEXY_LIT("abc") + lexy::dsl::value_c<1>, sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::twice<int> value)
            {
                CONSTEXPR_CHECK(cur - str == 7);
                CONSTEXPR_CHECK(value[0] == 1);
                CONSTEXPR_CHECK(value[1] == 1);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                if (e.string() == "abc")
                    return -1;
                else if (e.string() == ",")
                    return -2;
                else
                    CONSTEXPR_CHECK(false);
            }
        };

        auto zero = LEXY_VERIFY("");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc");
        CHECK(one == -2);

        auto two = LEXY_VERIFY("abc,abc");
        CHECK(two == 0);

        auto no_sep = LEXY_VERIFY("abcabc");
        CHECK(no_sep == -2);

        auto trailing_sep = LEXY_VERIFY("abc,abc,");
        CHECK(trailing_sep == 0);
    }
    SUBCASE("trailing_sep")
    {
        static constexpr auto rule
            = twice(LEXY_LIT("abc") + lexy::dsl::value_c<1>, trailing_sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::twice<int> value)
            {
                CONSTEXPR_CHECK(value[0] == 1);
                CONSTEXPR_CHECK(value[1] == 1);

                if (cur - str == 7)
                    return 0;
                else if (cur - str == 8)
                    return 1; // trailing sep
                else
                    CONSTEXPR_CHECK(false);
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                if (e.string() == "abc")
                    return -1;
                else if (e.string() == ",")
                    return -2;
                else
                    CONSTEXPR_CHECK(false);
            }
        };

        auto zero = LEXY_VERIFY("");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc");
        CHECK(one == -2);

        auto two = LEXY_VERIFY("abc,abc");
        CHECK(two == 0);

        auto no_sep = LEXY_VERIFY("abcabc");
        CHECK(no_sep == -2);

        auto trailing_sep = LEXY_VERIFY("abc,abc,");
        CHECK(trailing_sep == 1);
    }
}

