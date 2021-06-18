// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_lexeme.hpp>

#include "verify.hpp"
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/loop.hpp>

TEST_CASE("dsl::context_lexeme")
{
    struct error;

    static constexpr auto lex     = lexy::dsl::context_lexeme<struct lex_id>;
    static constexpr auto pattern = while_(lexy::dsl::lit_c<'*'> / lexy::dsl::lit_c<'+'>);
    static constexpr auto rule    = lex.create() + lex.capture(pattern)
                                 + lexy::dsl::lit_c<'-'> + lex.require(pattern).error<error>;

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str) / 2;
        }

        LEXY_VERIFY_FN int error(test_error<error>)
        {
            return -1;
        }
        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("-"));
            return -2;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -2);

    auto zero = LEXY_VERIFY("-");
    CHECK(zero == 0);
    auto one = LEXY_VERIFY("*-*");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("*+-*+");
    CHECK(two == 2);

    auto length_mismatch = LEXY_VERIFY("**-*");
    CHECK(length_mismatch == -1);
    auto char_mismatch = LEXY_VERIFY("**-*+");
    CHECK(char_mismatch == -1);
}

