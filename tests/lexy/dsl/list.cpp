// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/list.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("dsl::list()")
{
    static LEXY_VERIFY_FN auto rule
        = list(LEXY_LIT("ab") >> lexy::dsl::lit_c<'c'> + lexy::dsl::id<0>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                LEXY_VERIFY_FN void operator()(lexy::id<0>)
                {
                    ++count;
                }

                LEXY_VERIFY_FN int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur, int count)
        {
            LEXY_VERIFY_CHECK(cur - str == 3 * count);
            return count;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            if (e.position() == str)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("ab"));
                return -1;
            }
            else
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("c"));
                return -2;
            }
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abcabc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abcabcabc");
    CHECK(three == 3);

    auto condition_partial = LEXY_VERIFY("a");
    CHECK(condition_partial == -1);
    auto one_condition_partial = LEXY_VERIFY("abca");
    CHECK(one_condition_partial == 1);

    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == -2);
    auto one_partial = LEXY_VERIFY("abcab");
    CHECK(one_partial == -2);
}

TEST_CASE("dsl::list() sep")
{
    SUBCASE("cannot check for trailing sep")
    {
        static constexpr auto rule = list(lexy::dsl::id<0> + LEXY_LIT("abc"), sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::id<0>)
                    {
                        ++count;
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }

            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                LEXY_VERIFY_CHECK(cur - str == 4 * count - 1);
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto one = LEXY_VERIFY("abc");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("abc,abc");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("abc,abc,abc");
        CHECK(three == 3);

        auto partial = LEXY_VERIFY("ab");
        CHECK(partial == -1);
        auto one_partial = LEXY_VERIFY("abc,ab");
        CHECK(one_partial == -1);

        auto no_sep = LEXY_VERIFY("abcabc");
        CHECK(no_sep == 1);

        auto trailing = LEXY_VERIFY("abc,");
        CHECK(trailing == -1);
    }
    SUBCASE("can check for trailing sep")
    {
        static constexpr auto rule = list(LEXY_LIT("abc") >> lexy::dsl::id<0>, sep(LEXY_LIT(",")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::id<0>)
                    {
                        ++count;
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }

            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                if (cur[-1] == ',')
                    LEXY_VERIFY_CHECK(cur - str == 4 * count);
                else
                    LEXY_VERIFY_CHECK(cur - str == 4 * count - 1);
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::unexpected_trailing_separator> e)
            {
                LEXY_VERIFY_CHECK(*e.begin() == ',');
                return -2;
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
        CHECK(trailing.value == 1);
        CHECK(trailing.errors(-2));
    }
}

TEST_CASE("dsl::list() trailing_sep")
{
    static constexpr auto rule
        = list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                LEXY_VERIFY_FN void operator()(lexy::id<0>)
                {
                    ++count;
                }

                LEXY_VERIFY_FN int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur, int count)
        {
            if (cur[-1] == ',')
                LEXY_VERIFY_CHECK(cur - str == 4 * count);
            else
                LEXY_VERIFY_CHECK(cur - str == 4 * count - 1);
            return count;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
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

TEST_CASE("dsl::opt_list())")
{
    static constexpr auto rule = opt_list(LEXY_LIT("abc") >> lexy::dsl::id<0>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                LEXY_VERIFY_FN void operator()(lexy::id<0>)
                {
                    ++count;
                }

                LEXY_VERIFY_FN int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur, int count)
        {
            LEXY_VERIFY_CHECK(cur - str == 3 * count);
            return count;
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

TEST_CASE("dsl::opt_list() sep")
{
    static constexpr auto rule = opt_list(LEXY_LIT("abc") >> lexy::dsl::id<0>, sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                LEXY_VERIFY_FN void operator()(lexy::id<0>)
                {
                    ++count;
                }

                LEXY_VERIFY_FN int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur, int count)
        {
            if (count == 0)
                LEXY_VERIFY_CHECK(cur == str);
            else if (cur[-1] == ',')
                LEXY_VERIFY_CHECK(cur - str == 4 * count);
            else
                LEXY_VERIFY_CHECK(cur - str == 4 * count - 1);
            return count;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
            return -1;
        }
        LEXY_VERIFY_FN int error(test_error<lexy::unexpected_trailing_separator> e)
        {
            LEXY_VERIFY_CHECK(*e.begin() == ',');
            return -2;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto one = LEXY_VERIFY("abc");
    CHECK(one == 1);
    auto two = LEXY_VERIFY("abc,abc");
    CHECK(two == 2);
    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three == 3);

    auto no_sep = LEXY_VERIFY("abcabc");
    CHECK(no_sep == 1);

    auto partial = LEXY_VERIFY("ab");
    CHECK(partial == 0);
    auto one_partial = LEXY_VERIFY("abc,ab");
    CHECK(one_partial.value == 1);
    CHECK(one_partial.errors(-2));
}

TEST_CASE("dsl::opt_list() trailing_sep")
{
    static constexpr auto rule
        = opt_list(LEXY_LIT("abc") >> lexy::dsl::id<0>, trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                int count = 0;

                using return_type = int;

                LEXY_VERIFY_FN void operator()(lexy::id<0>)
                {
                    ++count;
                }

                LEXY_VERIFY_FN int finish() &&
                {
                    return count;
                }
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur, int count)
        {
            if (count == 0)
                LEXY_VERIFY_CHECK(cur == str);
            else if (cur[-1] == ',')
                LEXY_VERIFY_CHECK(cur - str == 4 * count);
            else
                LEXY_VERIFY_CHECK(cur - str == 4 * count - 1);
            return count;
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

