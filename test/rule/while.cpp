// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include "verify.hpp"

TEST_CASE("rule: atom")
{
    constexpr auto rule = while_(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            assert((cur - str) % 3 == 0);
            return int(cur - str) / 3;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abcabc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abcabcabc");
    CHECK(three == 3);

    constexpr auto partial = rule_matches<callback>(rule, "abcab");
    CHECK(partial == 1);
}
