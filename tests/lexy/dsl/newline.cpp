// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/newline.hpp>

#include "verify.hpp"

TEST_CASE("dsl::newline")
{
    static constexpr auto rule = lexy::dsl::newline;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.character_class() == "newline");
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto nl = LEXY_VERIFY("\n");
    CHECK(nl == 1);

    auto cr = LEXY_VERIFY("\r");
    CHECK(cr == -1);
    auto cr_nl = LEXY_VERIFY("\r\n");
    CHECK(cr_nl == 2);

    auto extra_cr_nl = LEXY_VERIFY("\n\r\n");
    CHECK(extra_cr_nl == 1);
    auto extra_nl = LEXY_VERIFY("\r\n\n");
    CHECK(extra_nl == 2);
}

TEST_CASE("dsl::eol")
{
    static constexpr auto rule = lexy::dsl::eol;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            return int(cur - str);
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.character_class() == "EOL");
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto nl = LEXY_VERIFY("\n");
    CHECK(nl == 1);

    auto cr = LEXY_VERIFY("\r");
    CHECK(cr == -1);
    auto cr_nl = LEXY_VERIFY("\r\n");
    CHECK(cr_nl == 2);

    auto extra_cr_nl = LEXY_VERIFY("\n\r\n");
    CHECK(extra_cr_nl == 1);
    auto extra_nl = LEXY_VERIFY("\r\n\n");
    CHECK(extra_nl == 2);
}

