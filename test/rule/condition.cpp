// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/condition.hpp>

#include "verify.hpp"

TEST_CASE("rule: if_")
{
    constexpr auto rule = if_(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            assert(cur == str);
            return 0;
        }

        constexpr int error(test_error<lexy::expected_pattern> e)
        {
            assert(e.position() == str);
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 0);
}

TEST_CASE("rule: unless")
{
    constexpr auto rule = unless(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            assert(cur == str);
            return 0;
        }

        constexpr int error(test_error<lexy::unexpected_pattern> e)
        {
            assert(e.position() == str);
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == -1);
}

TEST_CASE("rule: not")
{
    constexpr auto rule = !LEXY_LIT("abc");
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            assert(cur == str);
            return 0;
        }

        constexpr int error(test_error<lexy::unexpected_pattern> e)
        {
            assert(e.position() == str);
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == -1);
}

