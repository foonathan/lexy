// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/option.hpp>

#include "verify.hpp"
#include <lexy/dsl/value.hpp>

TEST_CASE("dsl::nullopt")
{
    constexpr auto rule = lexy::dsl::nullopt;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::nullopt)
        {
            CONSTEXPR_CHECK(cur == str);
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 0);
}

TEST_CASE("dsl::opt()")
{
    SUBCASE("pattern")
    {
        constexpr auto rule = opt(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::nullopt)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }
            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str + 3);
                return 1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 1);

        constexpr auto partial = rule_matches<callback>(rule, "ab");
        CHECK(partial == 0);
    }
    SUBCASE("branch")
    {
        constexpr auto rule = opt(LEXY_LIT("a") >> LEXY_LIT("bc") + lexy::dsl::value_c<1>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                CONSTEXPR_CHECK(cur - str == 3 || cur == str);
                return i;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 1);

        constexpr auto condition = rule_matches<callback>(rule, "a");
        CHECK(condition == -1);
        constexpr auto partial = rule_matches<callback>(rule, "ab");
        CHECK(partial == -1);
    }
}

