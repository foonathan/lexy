// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/peek.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::peek()")
{
    constexpr auto rule = if_(peek(LEXY_LIT("abc")) >> LEXY_LIT("a"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::expected_literal>)
        {
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto a = rule_matches<callback>(rule, "a");
    CHECK(a == 0);
    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 1);
}

TEST_CASE("dsl::peek_not()")
{
    constexpr auto rule = if_(peek_not(LEXY_LIT("abc")) >> LEXY_LIT("a"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.string() == "a");
            CONSTEXPR_CHECK(e.position() == str);
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto a = rule_matches<callback>(rule, "a");
    CHECK(a == 1);
    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 0);
}

