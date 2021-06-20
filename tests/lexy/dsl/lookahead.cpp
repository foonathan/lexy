// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/lookahead.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::lookahead()")
{
    SUBCASE("parsing")
    {
        static constexpr auto rule = lookahead(LEXY_LIT("!"), LEXY_LIT("\n"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::lookahead_failure> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.value == 0);
        CHECK(empty.errors(-1));

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing.value == 0);
        CHECK(nothing.errors(-1));

        auto nothing_newline = LEXY_VERIFY("abc\n");
        CHECK(nothing_newline.value == 0);
        CHECK(nothing_newline.errors(-1));

        auto something = LEXY_VERIFY("abc!def\n");
        CHECK(something == 0);

        auto something_after = LEXY_VERIFY("abc\n!def\n");
        CHECK(something_after.value == 0);
        CHECK(something_after.errors(-1));
    }
    SUBCASE("branch parsing")
    {
        static constexpr auto rule
            = if_(lookahead(LEXY_LIT("!"), LEXY_LIT("\n")) >> lexy::dsl::error<struct error>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<error> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing == 0);

        auto nothing_newline = LEXY_VERIFY("abc\n");
        CHECK(nothing_newline == 0);

        auto something = LEXY_VERIFY("abc!def\n");
        CHECK(something == -1);

        auto something_after = LEXY_VERIFY("abc\n!def\n");
        CHECK(something_after == 0);
    }

    SUBCASE(".error")
    {
        static constexpr auto rule = lookahead(LEXY_LIT("!"), LEXY_LIT("\n")).error<struct tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.value == 0);
        CHECK(empty.errors(-1));

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing.value == 0);
        CHECK(nothing.errors(-1));

        auto nothing_newline = LEXY_VERIFY("abc\n");
        CHECK(nothing_newline.value == 0);
        CHECK(nothing_newline.errors(-1));

        auto something = LEXY_VERIFY("abc!def\n");
        CHECK(something == 0);

        auto something_after = LEXY_VERIFY("abc\n!def\n");
        CHECK(something_after.value == 0);
        CHECK(something_after.errors(-1));
    }
}

