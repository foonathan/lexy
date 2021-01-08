// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#define LEXY_IGNORE_DEPRECATED_CONTEXT
#include <lexy/dsl/context.hpp>

#include "verify.hpp"
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/value.hpp>
#include <lexy/dsl/while.hpp>

TEST_CASE("dsl::context_*")
{
    constexpr auto pattern = while_(lexy::dsl::lit_c<'*'> / lexy::dsl::lit_c<'+'>);

    SUBCASE("push + pop")
    {
        static constexpr auto rule = lexy::dsl::context_push(pattern)
                                     + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) / 2;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "-");
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
    SUBCASE("push + pop - length_eq")
    {
        static constexpr auto rule
            = lexy::dsl::context_push(pattern)
              + lexy::dsl::lit_c<
                  '-'> + lexy::dsl::context_pop<lexy::dsl::context_eq_length>(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) / 2;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "-");
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
        CHECK(char_mismatch == 2);
    }
    SUBCASE("push + top + pop")
    {
        static constexpr auto rule = lexy::dsl::context_push(pattern)
                                     + lexy::dsl::lit_c<'-'> + lexy::dsl::context_top(pattern)
                                     + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) / 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "-");
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto zero = LEXY_VERIFY("--");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("*-*-*");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("*+-*+-*+");
        CHECK(two == 2);

        auto length_mismatch = LEXY_VERIFY("**-*-**");
        CHECK(length_mismatch == -1);
        auto char_mismatch = LEXY_VERIFY("**-**-*+");
        CHECK(char_mismatch == -1);
    }
    SUBCASE("push + drop")
    {
        static constexpr auto rule = lexy::dsl::context_push(pattern) + lexy::dsl::context_drop;
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
        auto one = LEXY_VERIFY("*");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("*+");
        CHECK(two == 2);
    }

    SUBCASE("nested")
    {
        static constexpr auto rule = lexy::dsl::context_push(pattern)
                                     + lexy::dsl::lit_c<'-'> + lexy::dsl::context_push(pattern)
                                     + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern)
                                     + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "-");
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto zero_zero = LEXY_VERIFY("---");
        CHECK(zero_zero == 0);
        auto zero_one = LEXY_VERIFY("-+-+-");
        CHECK(zero_one == 2);
        auto one_one = LEXY_VERIFY("*-*-*-*");
        CHECK(one_one == 4);
        auto two_one = LEXY_VERIFY("**-*-*-**");
        CHECK(two_one == 6);

        auto mismatch_outer = LEXY_VERIFY("**-+-+-*");
        CHECK(mismatch_outer == -1);
        auto mismatch_inner = LEXY_VERIFY("**-+-++-**");
        CHECK(mismatch_inner == -1);
    }
    SUBCASE("pop discards values")
    {
        static constexpr auto rule
            = lexy::dsl::context_push(pattern)
              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern + lexy::dsl::value_c<0>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) / 2;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "-");
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
}

