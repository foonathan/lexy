// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include "verify.hpp"
#include <lexy/match.hpp>

TEST_CASE("dsl::while_()")
{
    SUBCASE("token")
    {
        constexpr auto rule = while_(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto one = verify<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = verify<callback>(rule, "abcabc");
        CHECK(two == 2);
        constexpr auto three = verify<callback>(rule, "abcabcabc");
        CHECK(three == 3);

        constexpr auto partial = verify<callback>(rule, "abcab");
        CHECK(partial == 1);
    }

    SUBCASE("branch")
    {
        constexpr auto rule = while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto one = verify<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = verify<callback>(rule, "abcabc");
        CHECK(two == 2);
        constexpr auto three = verify<callback>(rule, "abcabcabc");
        CHECK(three == 3);

        constexpr auto partial = verify<callback>(rule, "abcab");
        CHECK(partial == -1);
    }

    SUBCASE("choice")
    {
        constexpr auto rule = while_(LEXY_LIT("a") >> LEXY_LIT("bc") | LEXY_LIT("bbc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto one = verify<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = verify<callback>(rule, "abcbbc");
        CHECK(two == 2);
        constexpr auto three = verify<callback>(rule, "bbcabcabc");
        CHECK(three == 3);

        constexpr auto partial = verify<callback>(rule, "abcab");
        CHECK(partial == -1);
    }
}

TEST_CASE("dsl::while_one()")
{
    SUBCASE("token")
    {
        constexpr auto rule = while_one(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK((cur - str) % 3 == 0);
                return int(cur - str) / 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                CONSTEXPR_CHECK(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto one = verify<callback>(rule, "abc");
        CHECK(one == 1);
        constexpr auto two = verify<callback>(rule, "abcabc");
        CHECK(two == 2);
        constexpr auto three = verify<callback>(rule, "abcabcabc");
        CHECK(three == 3);

        constexpr auto partial = verify<callback>(rule, "abcab");
        CHECK(partial == 1);
    }
    SUBCASE("branch")
    {
        constexpr auto result = while_one(LEXY_LIT("a") >> LEXY_LIT("bc"));
        constexpr auto equivalent
            = LEXY_LIT("a") >> LEXY_LIT("bc") + while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(!lexy::match(lexy::zstring_input(""), result));
        CHECK(lexy::match(lexy::zstring_input("abc"), result));
        CHECK(lexy::match(lexy::zstring_input("abcabc"), result));
    }
}

TEST_CASE("dsl::do_while()")
{
    constexpr auto result     = do_while(LEXY_LIT("a"), LEXY_LIT("b"));
    constexpr auto equivalent = LEXY_LIT("a") >> while_(LEXY_LIT("b") >> LEXY_LIT("a"));
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

    CHECK(!lexy::match(lexy::zstring_input(""), result));
    CHECK(lexy::match(lexy::zstring_input("a"), result));
    CHECK(lexy::match(lexy::zstring_input("aba"), result));
    CHECK(lexy::match(lexy::zstring_input("ababa"), result));
}

