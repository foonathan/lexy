// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/newline.hpp>

#include "verify.hpp"

TEST_CASE("dsl::newline")
{
    static constexpr auto rule = lexy::dsl::newline;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("newline"));
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
    CHECK(lexy::is_token_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOL"));
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

