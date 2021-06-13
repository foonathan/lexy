// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/times.hpp>

#include "verify.hpp"

TEST_CASE("times")
{
    SUBCASE("no sep")
    {
        static constexpr auto rule = twice(LEXY_LIT("abc") + label<1>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<1>, id<1>)
            {
                LEXY_VERIFY_CHECK(cur - str == 6);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
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
        static constexpr auto rule = twice(LEXY_LIT("abc") + label<1>, sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<1>, id<1>)
            {
                LEXY_VERIFY_CHECK((cur - str == 7 || cur - str == 8));
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                if (e.string() == lexy::_detail::string_view("abc"))
                    return -1;
                else if (e.character() == ',')
                    return -2;
                else
                    LEXY_VERIFY_CHECK(false);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::unexpected_trailing_separator>)
            {
                return -3;
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
        CHECK(trailing_sep.value == 0);
        CHECK(trailing_sep.errors(-3));
    }
    SUBCASE("trailing_sep")
    {
        static constexpr auto rule = twice(LEXY_LIT("abc") + label<1>, trailing_sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<1>, id<1>)
            {
                if (cur - str == 7)
                    return 0;
                else if (cur - str == 8)
                    return 1; // trailing sep
                else
                    LEXY_VERIFY_CHECK(false);
                return -1;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                if (e.string() == lexy::_detail::string_view("abc"))
                    return -1;
                else if (e.character() == ',')
                    return -2;
                else
                    LEXY_VERIFY_CHECK(false);
                return -1;
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

