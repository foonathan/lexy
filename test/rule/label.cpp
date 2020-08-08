// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/label.hpp>

#include "verify.hpp"

TEST_CASE("rule: label")
{
    constexpr auto rule = lexy::dsl::label<struct lab>;
    CHECK(lexy::is_dsl<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::label<lab>)
        {
            assert(str == cur);
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 0);
}

TEST_CASE("rule: id")
{
    constexpr auto rule = lexy::dsl::id<0>;
    CHECK(lexy::is_dsl<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::id<0>)
        {
            static_assert(lexy::id<0>{} == 0);
            assert(str == cur);
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 0);
}

