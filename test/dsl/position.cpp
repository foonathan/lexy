// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/position.hpp>

#include "verify.hpp"

TEST_CASE("dsl::position")
{
    constexpr auto rule = lexy::dsl::position;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, const char* pos)
        {
            assert(str == cur);
            assert(cur == pos);
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 0);
}

