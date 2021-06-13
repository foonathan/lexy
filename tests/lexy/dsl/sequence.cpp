// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/sequence.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::operator+")
{
    SUBCASE("rule")
    {
        static constexpr auto rule
            = LEXY_LIT("a") + label<0> + LEXY_LIT("b") + capture(LEXY_LIT("c"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<0>, lexy::lexeme_for<test_input> lex)
            {
                LEXY_VERIFY_CHECK(str + 3 == cur);
                LEXY_VERIFY_CHECK(*lex.begin() == 'c');
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                if (e.character() == 'a')
                    return -1;
                else if (e.character() == 'b')
                    return -2;
                else if (e.character() == 'c')
                    return -3;
                else
                    return -4;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);
        auto a = LEXY_VERIFY("a");
        CHECK(a == -2);
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == -3);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule = if_(LEXY_LIT("a") + LEXY_LIT("b") + LEXY_LIT("c"));
        CHECK(lexy::is_rule<decltype(rule)>);

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
        auto a = LEXY_VERIFY("a");
        CHECK(a == 0);
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == 0);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 3);
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd == 3);
    }
}

