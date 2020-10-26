// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/combination.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::combination()")
{
    constexpr auto rule = lexy::dsl::combination(LEXY_LIT("a"), LEXY_LIT("b") >> lexy::dsl::id<0>,
                                                 LEXY_LIT("c") >> lexy::dsl::id<1>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void operator()(lexy::id<0>)
                {
                    ++count;
                }
                constexpr void operator()(lexy::id<1>)
                {
                    ++count;
                }

                constexpr int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        constexpr int success(const char* cur, int count)
        {
            assert(count == 2);
            assert(cur - str == 3);
            return *str;
        }

        constexpr int error(test_error<lexy::combination_duplicate> e)
        {
            assert(e.end() - e.begin() == 1);
            return -1;
        }
        constexpr int error(test_error<lexy::exhausted_choice>)
        {
            return -2;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -2);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 'a');
    constexpr auto acb = rule_matches<callback>(rule, "acb");
    CHECK(acb == 'a');
    constexpr auto bac = rule_matches<callback>(rule, "bac");
    CHECK(bac == 'b');
    constexpr auto bca = rule_matches<callback>(rule, "bca");
    CHECK(bca == 'b');
    constexpr auto cab = rule_matches<callback>(rule, "cab");
    CHECK(cab == 'c');
    constexpr auto cba = rule_matches<callback>(rule, "cba");
    CHECK(cba == 'c');

    constexpr auto aab = rule_matches<callback>(rule, "aab");
    CHECK(aab == -1);
    constexpr auto aba = rule_matches<callback>(rule, "aba");
    CHECK(aba == -1);

    constexpr auto ab = rule_matches<callback>(rule, "ab");
    CHECK(ab == -2);

    constexpr auto abca = rule_matches<callback>(rule, "abca");
    CHECK(abca == 'a');
}

TEST_CASE("dsl::partial_combination()")
{
    constexpr auto rule = lexy::dsl::partial_combination(LEXY_LIT("a") >> lexy::dsl::id<0>,
                                                         LEXY_LIT("b") >> lexy::dsl::id<1>,
                                                         LEXY_LIT("c") >> lexy::dsl::id<2>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void operator()(lexy::id<0>)
                {
                    ++count;
                }
                constexpr void operator()(lexy::id<1>)
                {
                    ++count;
                }
                constexpr void operator()(lexy::id<2>)
                {
                    ++count;
                }

                constexpr int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        constexpr int success(const char* cur, int count)
        {
            assert(cur - str == count);
            return count;
        }

        constexpr int error(test_error<lexy::combination_duplicate> e)
        {
            assert(e.end() - e.begin() == 1);
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto a = rule_matches<callback>(rule, "a");
    CHECK(a == 1);
    constexpr auto b = rule_matches<callback>(rule, "b");
    CHECK(b == 1);
    constexpr auto c = rule_matches<callback>(rule, "c");
    CHECK(c == 1);

    constexpr auto ab = rule_matches<callback>(rule, "ab");
    CHECK(ab == 2);
    constexpr auto ba = rule_matches<callback>(rule, "ba");
    CHECK(ba == 2);
    constexpr auto ac = rule_matches<callback>(rule, "ac");
    CHECK(ac == 2);
    constexpr auto ca = rule_matches<callback>(rule, "ca");
    CHECK(ca == 2);
    constexpr auto bc = rule_matches<callback>(rule, "bc");
    CHECK(bc == 2);
    constexpr auto cb = rule_matches<callback>(rule, "cb");
    CHECK(cb == 2);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 3);
    constexpr auto acb = rule_matches<callback>(rule, "acb");
    CHECK(acb == 3);
    constexpr auto bac = rule_matches<callback>(rule, "bac");
    CHECK(bac == 3);
    constexpr auto bca = rule_matches<callback>(rule, "bca");
    CHECK(bca == 3);
    constexpr auto cab = rule_matches<callback>(rule, "cab");
    CHECK(cab == 3);
    constexpr auto cba = rule_matches<callback>(rule, "cba");
    CHECK(cba == 3);

    constexpr auto aab = rule_matches<callback>(rule, "aab");
    CHECK(aab == -1);
    constexpr auto aba = rule_matches<callback>(rule, "aba");
    CHECK(aba == -1);

    constexpr auto abca = rule_matches<callback>(rule, "abca");
    CHECK(abca == 3);
}

