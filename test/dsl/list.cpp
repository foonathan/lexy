// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/list.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("dsl::make_list()")
{
    constexpr auto rule
        = make_list(LEXY_LIT("a") + item(lexy::dsl::id<0>) + LEXY_LIT("b")
                    + lexy::dsl::id<1> + item(lexy::dsl::id<2> + LEXY_LIT("c") + lexy::dsl::id<3>));
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

                constexpr void operator()(lexy::id<0>)
                {
                    CONSTEXPR_CHECK(count == 0);
                    ++count;
                }
                constexpr void operator()(lexy::id<2>, lexy::id<3>)
                {
                    CONSTEXPR_CHECK(count == 1);
                    ++count;
                }

                constexpr int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        constexpr int success(const char* cur, lexy::id<1>, int count)
        {
            CONSTEXPR_CHECK(count == 2);
            CONSTEXPR_CHECK(cur - str == 3);
            return 0;
        }

        constexpr int error(test_error<lexy::expected_literal>)
        {
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 0);
}

TEST_CASE("dsl::list()")
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

                constexpr void operator()(lexy::id<0>)
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
            CONSTEXPR_CHECK(cur - str == 3 * count);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
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

TEST_CASE("dsl::list() sep")
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

                constexpr void operator()(lexy::id<0>)
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
            CONSTEXPR_CHECK(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
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

TEST_CASE("dsl::list() sep capture")
{
    constexpr auto rule = list(lexy::dsl::id<0> + LEXY_LIT("abc"), sep(LEXY_LIT(",")).capture());
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

                constexpr void operator()(lexy::id<0>)
                {
                    count += 3;
                }
                constexpr void operator()(lexy::lexeme_for<test_input> lex)
                {
                    CONSTEXPR_CHECK(lex.size() == 1);
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
            CONSTEXPR_CHECK(cur - str == count);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == -1);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 3);
    constexpr auto two = rule_matches<callback>(rule, "abc,abc");
    CHECK(two == 7);
    constexpr auto three = rule_matches<callback>(rule, "abc,abc,abc");
    CHECK(three == 11);

    constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
    CHECK(no_sep == 3);
}

TEST_CASE("dsl::list() trailing_sep")
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

                constexpr void operator()(lexy::id<0>)
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
                CONSTEXPR_CHECK(cur - str == 4 * count);
            else
                CONSTEXPR_CHECK(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
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

TEST_CASE("dsl::list() trailing_sep capture")
{
    constexpr auto rule
        = list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(",")).capture());
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

                constexpr void operator()(lexy::id<0>)
                {
                    count += 3;
                }
                constexpr void operator()(lexy::lexeme_for<test_input> lex)
                {
                    CONSTEXPR_CHECK(lex.size() == 1);
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
            CONSTEXPR_CHECK(cur - str == count);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);
    constexpr auto partial = rule_matches<callback>(rule, "ab");
    CHECK(partial == -1);

    constexpr auto one = rule_matches<callback>(rule, "abc");
    CHECK(one == 3);
    constexpr auto two = rule_matches<callback>(rule, "abc,abc");
    CHECK(two == 7);
    constexpr auto three = rule_matches<callback>(rule, "abc,abc,abc");
    CHECK(three == 11);

    constexpr auto no_sep = rule_matches<callback>(rule, "abcabc");
    CHECK(no_sep == 3);

    constexpr auto trailing = rule_matches<callback>(rule, "abc,");
    CHECK(trailing == 4);
}

TEST_CASE("dsl::opt(list())")
{
    constexpr auto rule = opt(list(LEXY_LIT("abc") >> lexy::dsl::id<0>));
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

                constexpr void operator()(lexy::id<0>)
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

        constexpr int success(const char* cur)
        {
            CONSTEXPR_CHECK(cur == str);
            return 0;
        }
        constexpr int success(const char* cur, int count)
        {
            CONSTEXPR_CHECK(cur - str == 3 * count);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
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

TEST_CASE("dsl::opt(list()) sep")
{
    constexpr auto rule = opt(list(LEXY_LIT("abc") >> lexy::dsl::id<0>, sep(LEXY_LIT(","))));
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

                constexpr void operator()(lexy::id<0>)
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
            if (count == 0)
                CONSTEXPR_CHECK(cur == str);
            else
                CONSTEXPR_CHECK(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
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

TEST_CASE("dsl::opt(list()) trailing_sep")
{
    constexpr auto rule
        = opt(list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(","))));
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

                constexpr void operator()(lexy::id<0>)
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
            if (count == 0)
                CONSTEXPR_CHECK(cur == str);
            else if (cur[-1] == ',')
                CONSTEXPR_CHECK(cur - str == 4 * count);
            else
                CONSTEXPR_CHECK(cur - str == 4 * count - 1);
            return count;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
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

