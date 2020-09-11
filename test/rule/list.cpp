// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/list.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("rule: list without separator")
{
    constexpr auto rule = list(LEXY_LIT("abc") >> lexy::dsl::id<0>);
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void item(lexy::id<0>)
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
            assert(cur - str == 3 * count);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.position() == str);
            assert(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == -1);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abcabc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abcabcabc");
    CHECK(three == 3);
}

TEST_CASE("rule: list with separator")
{
    constexpr auto rule = list(lexy::dsl::id<0> + LEXY_LIT("abc"), sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void item(lexy::id<0>)
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
            assert(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.position() == str);
            assert(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == -1);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abc,abc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abc,abc,abc");
    CHECK(three == 3);

    constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
    CHECK(no_sep == 1);
}

TEST_CASE("rule: list with trailing separator")
{
    constexpr auto rule = list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void item(lexy::id<0>)
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
            if (cur[-1] == ',')
                assert(cur - str == 4 * count);
            else
                assert(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.position() == str);
            assert(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == -1);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abc,abc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abc,abc,abc");
    CHECK(three == 3);

    constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
    CHECK(no_sep == 1);

    constexpr auto trailing = rule_matches<callback>(rule, "abc,");
    CHECK(trailing == 1);
}

TEST_CASE("rule: opt_list without separator")
{
    constexpr auto rule = opt_list(LEXY_LIT("abc") >> lexy::dsl::id<0>);
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void item(lexy::id<0>)
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
            assert(cur - str == 3 * count);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.position() == str);
            assert(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == 0);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abcabc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abcabcabc");
    CHECK(three == 3);
}

TEST_CASE("rule: opt_list with separator")
{
    constexpr auto rule = opt_list(LEXY_LIT("abc") >> lexy::dsl::id<0>, sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void item(lexy::id<0>)
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
            if (str == cur)
                return count;

            assert(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.position() == str);
            assert(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == 0);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abc,abc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abc,abc,abc");
    CHECK(three == 3);

    constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
    CHECK(no_sep == 1);
}

TEST_CASE("rule: opt_list with trailing separator")
{
    constexpr auto rule
        = opt_list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                constexpr void item(lexy::id<0>)
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
            if (cur == str)
                return count;

            if (cur[-1] == ',')
                assert(cur - str == 4 * count);
            else
                assert(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            assert(e.position() == str);
            assert(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == 0);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 1);
    constexpr auto two = rule_matches<callback>(rule, "abc,abc");
    CHECK(two == 2);
    constexpr auto three = rule_matches<callback>(rule, "abc,abc,abc");
    CHECK(three == 3);

    constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
    CHECK(no_sep == 1);

    constexpr auto trailing = rule_matches<callback>(rule, "abc,");
    CHECK(trailing == 1);
}

