// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/until.hpp>

#include "verify.hpp"
#include <lexy/dsl/peek.hpp>

TEST_CASE("dsl::until()")
{
    static constexpr auto rule = until(LEXY_LIT("!"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.position() == lexy::_detail::string_view(str).end());
            LEXY_VERIFY_CHECK(e.string() == "!");
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto zero = LEXY_VERIFY("!");
    CHECK(zero == 1);
    auto one = LEXY_VERIFY("a!");
    CHECK(one == 2);
    auto two = LEXY_VERIFY("ab!");
    CHECK(two == 3);

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated == -1);
}

TEST_CASE("dsl::until().or_eof()")
{
    static constexpr auto rule = until(LEXY_LIT("!")).or_eof();
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto zero = LEXY_VERIFY("!");
    CHECK(zero == 1);
    auto one = LEXY_VERIFY("a!");
    CHECK(one == 2);
    auto two = LEXY_VERIFY("xy!");
    CHECK(two == 3);

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated == 3);
}

