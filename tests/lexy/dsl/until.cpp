// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/until.hpp>

#include "verify.hpp"
#include <lexy/dsl/peek.hpp>

TEST_CASE("dsl::until()")
{
    constexpr auto rule = until(LEXY_LIT("!"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == lexy::_detail::string_view(str).end());
            CONSTEXPR_CHECK(e.string() == "!");
            return -1;
        }
    };

    constexpr auto empty = verify<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto zero = verify<callback>(rule, "!");
    CHECK(zero == 1);
    constexpr auto one = verify<callback>(rule, "a!");
    CHECK(one == 2);
    constexpr auto two = verify<callback>(rule, "ab!");
    CHECK(two == 3);

    constexpr auto unterminated = verify<callback>(rule, "abc");
    CHECK(unterminated == -1);
}

TEST_CASE("dsl::until().or_eof()")
{
    constexpr auto rule = until(LEXY_LIT("!")).or_eof();
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }
    };

    constexpr auto empty = verify<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto zero = verify<callback>(rule, "!");
    CHECK(zero == 1);
    constexpr auto one = verify<callback>(rule, "a!");
    CHECK(one == 2);
    constexpr auto two = verify<callback>(rule, "xy!");
    CHECK(two == 3);

    constexpr auto unterminated = verify<callback>(rule, "abc");
    CHECK(unterminated == 3);
}

