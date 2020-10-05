// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context.hpp>

#include "verify.hpp"
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/while.hpp>

TEST_CASE("rule: context_*")
{
    constexpr auto pattern = while_(lexy::dsl::lit_c<'*'> / lexy::dsl::lit_c<'+'>);

    SUBCASE("push + pop")
    {
        constexpr auto rule = lexy::dsl::context_push(pattern)
                              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str) / 2;
            }

            constexpr int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "-");
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -2);

        constexpr auto zero = rule_matches<callback>(rule, "-");
        CHECK(zero == 0);
        constexpr auto one = rule_matches<callback>(rule, "*-*");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "*+-*+");
        CHECK(two == 2);

        constexpr auto length_mismatch = rule_matches<callback>(rule, "**-*");
        CHECK(length_mismatch == -1);
        constexpr auto char_mismatch = rule_matches<callback>(rule, "**-*+");
        CHECK(char_mismatch == -1);
    }
    SUBCASE("push + pop - length_eq")
    {
        constexpr auto rule
            = lexy::dsl::context_push(pattern)
              + lexy::dsl::lit_c<
                  '-'> + lexy::dsl::context_pop<lexy::dsl::context_eq_length>(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str) / 2;
            }

            constexpr int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "-");
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -2);

        constexpr auto zero = rule_matches<callback>(rule, "-");
        CHECK(zero == 0);
        constexpr auto one = rule_matches<callback>(rule, "*-*");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "*+-*+");
        CHECK(two == 2);

        constexpr auto length_mismatch = rule_matches<callback>(rule, "**-*");
        CHECK(length_mismatch == -1);
        constexpr auto char_mismatch = rule_matches<callback>(rule, "**-*+");
        CHECK(char_mismatch == 2);
    }
    SUBCASE("push + top + pop")
    {
        constexpr auto rule = lexy::dsl::context_push(pattern)
                              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_top(pattern)
                              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str) / 3;
            }

            constexpr int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "-");
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -2);

        constexpr auto zero = rule_matches<callback>(rule, "--");
        CHECK(zero == 0);
        constexpr auto one = rule_matches<callback>(rule, "*-*-*");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "*+-*+-*+");
        CHECK(two == 2);

        constexpr auto length_mismatch = rule_matches<callback>(rule, "**-*-**");
        CHECK(length_mismatch == -1);
        constexpr auto char_mismatch = rule_matches<callback>(rule, "**-**-*+");
        CHECK(char_mismatch == -1);
    }
    SUBCASE("push + drop")
    {
        constexpr auto rule = lexy::dsl::context_push(pattern) + lexy::dsl::context_drop;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);
        constexpr auto one = rule_matches<callback>(rule, "*");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "*+");
        CHECK(two == 2);
    }

    SUBCASE("nested")
    {
        constexpr auto rule = lexy::dsl::context_push(pattern)
                              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_push(pattern)
                              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern)
                              + lexy::dsl::lit_c<'-'> + lexy::dsl::context_pop(pattern);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            constexpr int error(test_error<lexy::context_mismatch>)
            {
                return -1;
            }
            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "-");
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -2);

        constexpr auto zero_zero = rule_matches<callback>(rule, "---");
        CHECK(zero_zero == 0);
        constexpr auto zero_one = rule_matches<callback>(rule, "-+-+-");
        CHECK(zero_one == 2);
        constexpr auto one_one = rule_matches<callback>(rule, "*-*-*-*");
        CHECK(one_one == 4);
        constexpr auto two_one = rule_matches<callback>(rule, "**-*-*-**");
        CHECK(two_one == 6);

        constexpr auto mismatch_outer = rule_matches<callback>(rule, "**-+-+-*");
        CHECK(mismatch_outer == -1);
        constexpr auto mismatch_inner = rule_matches<callback>(rule, "**-+-++-**");
        CHECK(mismatch_inner == -1);
    }
}

