// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/until.hpp>

#include "verify.hpp"

TEST_CASE("rule: until")
{
    constexpr auto rule = until(LEXY_LIT("!"));
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
            assert(e.position() == lexy::_detail::string_view(str).end());
            assert(e.string() == "!");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto zero = rule_matches<callback>(rule, "!");
    CHECK(zero == 1);
    constexpr auto one = rule_matches<callback>(rule, "a!");
    CHECK(one == 2);
    constexpr auto two = rule_matches<callback>(rule, "ab!");
    CHECK(two == 3);

    constexpr auto unterminated = rule_matches<callback>(rule, "abc");
    CHECK(unterminated == -1);
}

