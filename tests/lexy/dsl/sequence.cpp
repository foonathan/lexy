// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/sequence.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::operator+")
{
    static constexpr auto rule
        = LEXY_LIT("a") + lexy::dsl::label<struct lab> + LEXY_LIT("b") + capture(LEXY_LIT("c"));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::label<lab>, lexy::lexeme_for<test_input> lex)
        {
            CONSTEXPR_CHECK(str + 3 == cur);
            CONSTEXPR_CHECK(*lex.begin() == 'c');
            return 0;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            if (e.string() == "a")
                return -1;
            else if (e.string() == "b")
                return -2;
            else if (e.string() == "c")
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

