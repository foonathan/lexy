// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/loop.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>

TEST_CASE("dsl::loop()")
{
    constexpr auto rule = loop(LEXY_LIT("a") | LEXY_LIT("!") >> lexy::dsl::break_);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str) - 1;
        }

        constexpr int error(test_error<lexy::exhausted_choice>)
        {
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto one = rule_matches<callback>(rule, "a!");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "aa!");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "aaa!");
    CHECK(three == 3);

    constexpr auto unterminated = rule_matches<callback>(rule, "aaa");
    CHECK(unterminated == -1);
}

