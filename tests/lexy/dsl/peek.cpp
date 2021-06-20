// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/peek.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::peek()")
{
    SUBCASE("parsing")
    {
        static constexpr auto rule = peek(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::peek_failure> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.value == 0);
        CHECK(empty.errors(-1));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
    SUBCASE("branch parsing")
    {
        static constexpr auto rule = if_(peek(LEXY_LIT("abc")) >> LEXY_LIT("a"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
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

    SUBCASE(".error")
    {
        static constexpr auto rule = peek(LEXY_LIT("abc")).error<struct tag>;
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

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
}

TEST_CASE("dsl::peek_not()")
{
    SUBCASE("parsing")
    {
        static constexpr auto rule = peek_not(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::unexpected> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == str + 3);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.value == 0);
        CHECK(abc.errors(-1));
    }
    SUBCASE("branch parsing")
    {
        static constexpr auto rule = if_(peek_not(LEXY_LIT("abc")) >> LEXY_LIT("a"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == 'a');
                LEXY_VERIFY_CHECK(e.position() == str);
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

    SUBCASE(".error")
    {
        static constexpr auto rule = peek_not(LEXY_LIT("abc")).error<struct tag>;
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
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == str + 3);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.value == 0);
        CHECK(abc.errors(-1));
    }
}

