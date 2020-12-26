// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/loop.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>

TEST_CASE("dsl::loop()")
{
    static constexpr auto rule = loop(LEXY_LIT("a") | LEXY_LIT("!") >> lexy::dsl::break_);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str) - 1;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice>)
        {
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto one = LEXY_VERIFY("a!");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("aa!");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("aaa!");
    CHECK(three == 3);

    auto unterminated = LEXY_VERIFY("aaa");
    CHECK(unterminated == -1);
}

