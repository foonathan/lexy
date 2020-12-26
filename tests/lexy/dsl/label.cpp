// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/label.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::label")
{
    SUBCASE("simple")
    {
        static constexpr auto rule = lexy::dsl::label<struct lab>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::label<lab>)
            {
                CONSTEXPR_CHECK(str == cur);
                return 0;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto string = LEXY_VERIFY("abc");
        CHECK(string == 0);
    }
    SUBCASE("operator()")
    {
        static constexpr auto rule = lexy::dsl::label<struct lab>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::label<lab>)
            {
                CONSTEXPR_CHECK(cur == str + 3);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto string = LEXY_VERIFY("abc");
        CHECK(string == 0);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule = if_(lexy::dsl::label<struct lab>(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }
            constexpr int success(const char* cur, lexy::label<lab>)
            {
                CONSTEXPR_CHECK(cur == str + 3);
                return 1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto string = LEXY_VERIFY("abc");
        CHECK(string == 1);
    }
}

TEST_CASE("dsl::id")
{
    static constexpr auto rule = lexy::dsl::id<0>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::id<0>)
        {
            static_assert(lexy::id<0>{} == 0);
            CONSTEXPR_CHECK(str == cur);
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto string = LEXY_VERIFY("abc");
    CHECK(string == 0);
}

