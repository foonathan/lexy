// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/token.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/value.hpp>

TEST_CASE("dsl::token")
{
    constexpr auto rule = token(list(LEXY_LIT("abc") >> lexy::dsl::value_c<0>));
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

    constexpr auto empty = verify<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto one = verify<callback>(rule, "abc");
    CHECK(one == 3);
    constexpr auto two = verify<callback>(rule, "abcabc");
    CHECK(two == 6);
    constexpr auto three = verify<callback>(rule, "abcabcabc");
    CHECK(three == 9);
}

