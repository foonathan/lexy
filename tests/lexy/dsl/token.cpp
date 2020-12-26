// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/token.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/value.hpp>

TEST_CASE("dsl::token")
{
    static constexpr auto rule = token(list(LEXY_LIT("abc") >> lexy::dsl::value_c<0>));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::missing_token> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 3);
    auto two = LEXY_VERIFY("abcabc");
    CHECK(two == 6);
    auto three = LEXY_VERIFY("abcabcabc");
    CHECK(three == 9);
}

