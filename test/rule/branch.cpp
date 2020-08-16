// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/branch.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>

TEST_CASE("rule: operator>>")
{
    constexpr auto rule = LEXY_LIT("a") >> lexy::dsl::label<struct lab>;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::label<lab>)
        {
            assert(str + 1 == cur);
            return 0;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.string() == "a");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto success = rule_matches<callback>(rule, "a");
    CHECK(success == 0);
}

