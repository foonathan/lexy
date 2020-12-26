// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/peek.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::peek()")
{
    static constexpr auto rule = if_(peek(LEXY_LIT("abc")) >> LEXY_LIT("a"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::expected_literal>)
        {
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto a = LEXY_VERIFY("a");
    CHECK(a == 0);
    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 1);
}

TEST_CASE("dsl::peek_not()")
{
    static constexpr auto rule = if_(peek_not(LEXY_LIT("abc")) >> LEXY_LIT("a"));
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
            CONSTEXPR_CHECK(e.string() == "a");
            CONSTEXPR_CHECK(e.position() == str);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto a = LEXY_VERIFY("a");
    CHECK(a == 1);
    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
}

