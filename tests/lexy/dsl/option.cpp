// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/option.hpp>

#include "verify.hpp"
#include <lexy/dsl/value.hpp>

TEST_CASE("dsl::nullopt")
{
    static constexpr auto rule = lexy::dsl::nullopt;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
        {
            LEXY_VERIFY_CHECK(cur == str);
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto string = LEXY_VERIFY("abc");
    CHECK(string == 0);
}

TEST_CASE("dsl::opt()")
{
    static constexpr auto rule = opt(LEXY_LIT("a") >> LEXY_LIT("bc") + lexy::dsl::value_c<1>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
        {
            LEXY_VERIFY_CHECK(cur == str);
            return 0;
        }
        LEXY_VERIFY_FN int success(const char* cur, int i)
        {
            LEXY_VERIFY_CHECK(cur - str == 3);
            return i;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.string() == "bc");
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto success = LEXY_VERIFY("abc");
    CHECK(success == 1);

    auto condition = LEXY_VERIFY("a");
    CHECK(condition == -1);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -1);
}

