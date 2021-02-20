// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include "verify.hpp"

TEST_CASE("dsl::while_()")
{
    SUBCASE("token")
    {
        static constexpr auto rule = while_(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto one = LEXY_VERIFY("abc");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("abcabc");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("abcabcabc");
        CHECK(three == 3);

        auto partial = LEXY_VERIFY("abcab");
        CHECK(partial == 1);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule = while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("bc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto one = LEXY_VERIFY("abc");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("abcabc");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("abcabcabc");
        CHECK(three == 3);

        auto partial = LEXY_VERIFY("abcab");
        CHECK(partial == -1);
    }
}

TEST_CASE("dsl::while_one()")
{
    SUBCASE("token")
    {
        static constexpr auto rule = while_one(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto one = LEXY_VERIFY("abc");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("abcabc");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("abcabcabc");
        CHECK(three == 3);

        auto partial = LEXY_VERIFY("abcab");
        CHECK(partial == 1);
    }
    SUBCASE("branch")
    {
        constexpr auto result = while_one(LEXY_LIT("a") >> LEXY_LIT("bc"));
        constexpr auto equivalent
            = LEXY_LIT("a") >> LEXY_LIT("bc") + while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
}

TEST_CASE("dsl::do_while()")
{
    constexpr auto result     = do_while(LEXY_LIT("a"), LEXY_LIT("b"));
    constexpr auto equivalent = LEXY_LIT("a") >> while_(LEXY_LIT("b") >> LEXY_LIT("a"));
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
}

