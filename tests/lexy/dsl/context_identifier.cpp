// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_identifier.hpp>

#include "verify.hpp"
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/error.hpp>

TEST_CASE("dsl::context_identifier")
{
    static constexpr auto identifier
        = lexy::dsl::identifier(lexy::dsl::lit_c<'*'> / lexy::dsl::lit_c<'+'>);
    static constexpr auto var = lexy::dsl::context_identifier<struct var_id>(identifier);

    SUBCASE("parsing")
    {
        static constexpr auto rule = var.create() + var.capture()
                                     + lexy::dsl::lit_c<'-'> + var.rematch().error<struct error>;

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                LEXY_VERIFY_CHECK(lex.size() == std::size_t(cur - str) / 2);
                return int(lex.size());
            }

            LEXY_VERIFY_FN int error(test_error<error>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '-');
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_alternatives> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -3;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -3);

        auto zero = LEXY_VERIFY("-");
        CHECK(zero == -3);
        auto one = LEXY_VERIFY("*-*");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("*+-*+");
        CHECK(two == 2);

        auto length_mismatch = LEXY_VERIFY("**-*");
        CHECK(length_mismatch.value == 2);
        CHECK(length_mismatch.errors(-1));
        auto char_mismatch = LEXY_VERIFY("**-*+");
        CHECK(char_mismatch.value == 2);
        CHECK(char_mismatch.errors(-1));
    }
    SUBCASE("branch parsing")
    {
        static constexpr auto rule
            = var.create() + var.capture()
              + lexy::dsl::lit_c<'-'> + must(var.rematch()).error<struct error>;

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                LEXY_VERIFY_CHECK(lex.size() == std::size_t(cur - str) / 2);
                return int(lex.size());
            }

            LEXY_VERIFY_FN int error(test_error<error>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '-');
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_alternatives> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -3;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -3);

        auto zero = LEXY_VERIFY("-");
        CHECK(zero == -3);
        auto one = LEXY_VERIFY("*-*");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("*+-*+");
        CHECK(two == 2);

        auto length_mismatch = LEXY_VERIFY("**-*");
        CHECK(length_mismatch == -1);
        auto char_mismatch = LEXY_VERIFY("**-*+");
        CHECK(char_mismatch == -1);
    }
}

