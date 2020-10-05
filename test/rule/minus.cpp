// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/minus.hpp>

#include "verify.hpp"
#include <lexy/dsl/while.hpp>

TEST_CASE("rule: minus")
{
    constexpr auto rule = while_(LEXY_LIT("a")) - LEXY_LIT("aa");
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::minus_failure> e)
        {
            assert(e.begin() == str);
            assert(e.end() == lexy::_detail::string_view(str).end());
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto a = rule_matches<callback>(rule, "a");
    CHECK(a == 1);
    constexpr auto aaa = rule_matches<callback>(rule, "aaa");
    CHECK(aaa == 3);

    constexpr auto aa = rule_matches<callback>(rule, "aa");
    CHECK(aa == -1);
}

