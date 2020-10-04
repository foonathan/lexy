// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/times.hpp>

#include "verify.hpp"
#include <lexy/dsl/value.hpp>

TEST_CASE("rule: times")
{
    SUBCASE("no sep")
    {
        constexpr auto rule = twice(LEXY_LIT("abc") + lexy::dsl::value_c<0>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::twice<int> value)
            {
                assert(cur - str == 6);
                assert(value[0] == 0);
                assert(value[1] == 0);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto zero = rule_matches<callback>(rule, "");
        CHECK(zero == -1);
        constexpr auto one = rule_matches<callback>(rule, "abc");
        CHECK(one == -1);

        constexpr auto two = rule_matches<callback>(rule, "abcabc");
        CHECK(two == 0);
    }
    SUBCASE("sep")
    {
        constexpr auto rule = twice(LEXY_LIT("abc") + lexy::dsl::value_c<0>, sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::twice<int> value)
            {
                assert(cur - str == 7);
                assert(value[0] == 0);
                assert(value[1] == 0);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                if (e.string() == "abc")
                    return -1;
                else if (e.string() == ",")
                    return -2;
                else
                    assert(false);
            }
        };

        constexpr auto zero = rule_matches<callback>(rule, "");
        CHECK(zero == -1);
        constexpr auto one = rule_matches<callback>(rule, "abc");
        CHECK(one == -2);

        constexpr auto two = rule_matches<callback>(rule, "abc,abc");
        CHECK(two == 0);

        constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
        CHECK(no_sep == -2);

        constexpr auto trailing_sep = rule_matches<callback>(rule, "abc,abc,");
        CHECK(trailing_sep == 0);
    }
    SUBCASE("trailing_sep")
    {
        constexpr auto rule
            = twice(LEXY_LIT("abc") + lexy::dsl::value_c<0>, trailing_sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::twice<int> value)
            {
                assert(value[0] == 0);
                assert(value[1] == 0);

                if (cur - str == 7)
                    return 0;
                else if (cur - str == 8)
                    return 1; // trailing sep
                else
                    assert(false);
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                if (e.string() == "abc")
                    return -1;
                else if (e.string() == ",")
                    return -2;
                else
                    assert(false);
            }
        };

        constexpr auto zero = rule_matches<callback>(rule, "");
        CHECK(zero == -1);
        constexpr auto one = rule_matches<callback>(rule, "abc");
        CHECK(one == -2);

        constexpr auto two = rule_matches<callback>(rule, "abc,abc");
        CHECK(two == 0);

        constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
        CHECK(no_sep == -2);

        constexpr auto trailing_sep = rule_matches<callback>(rule, "abc,abc,");
        CHECK(trailing_sep == 1);
    }
}

