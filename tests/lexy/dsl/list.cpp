// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/list.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("dsl::list()")
{
    static constexpr auto rule = list(LEXY_LIT("abc") >> lexy::dsl::id<0>);
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abcabc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abcabcabc");
    CHECK(three == 3);
}

TEST_CASE("dsl::list() sep")
{
    static constexpr auto rule = list(lexy::dsl::id<0> + LEXY_LIT("abc"), sep(LEXY_LIT(",")));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 3);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 1);
}

TEST_CASE("dsl::list() sep capture")
{
    static constexpr auto rule
        = list(lexy::dsl::id<0> + LEXY_LIT("abc"), sep(capture(LEXY_LIT(","))));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 3);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 7);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 11);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 3);
}

TEST_CASE("dsl::list() trailing_sep")
{
    static constexpr auto rule
        = list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(",")));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 3);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 1);

    auto trailing = LEXY_VERIFY("abc,");
    CHECK(trailing == 1);
}

TEST_CASE("dsl::list() trailing_sep capture")
{
    static constexpr auto rule
        = list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(capture(LEXY_LIT(","))));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 3);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 7);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 11);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 3);

    auto trailing = LEXY_VERIFY("abc,");
    CHECK(trailing == 4);
}

TEST_CASE("dsl::opt(list())")
{
    static constexpr auto rule = opt(list(LEXY_LIT("abc") >> lexy::dsl::id<0>));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == 0);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abcabc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abcabcabc");
    CHECK(three == 3);
}

TEST_CASE("dsl::opt(list()) sep")
{
    static constexpr auto rule = opt(list(LEXY_LIT("abc") >> lexy::dsl::id<0>, sep(LEXY_LIT(","))));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == 0);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 3);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 1);
}

TEST_CASE("dsl::opt(list()) trailing_sep")
{
    static constexpr auto rule
        = opt(list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(","))));
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

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);
    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == 0);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 3);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 1);

    auto trailing = LEXY_VERIFY("abc,");
    CHECK(trailing == 1);
}

