// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/combination.hpp>

#include "verify.hpp"
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/error.hpp>

TEST_CASE("dsl::combination()")
{
    SUBCASE("basic")
    {
        static constexpr auto rule
            = lexy::dsl::combination(LEXY_LIT("a"), LEXY_LIT("b") >> label<0>,
                                     LEXY_LIT("c") >> label<1>);
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

                    LEXY_VERIFY_FN void operator()(id<0>)
                    {
                        ++count;
                    }
                    LEXY_VERIFY_FN void operator()(id<1>)
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
                LEXY_VERIFY_CHECK(count == 2);
                LEXY_VERIFY_CHECK(cur - str == 3);
                return *str;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::combination_duplicate> e)
            {
                LEXY_VERIFY_CHECK(e.end() - e.begin() == 1);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice>)
            {
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 'a');
        auto acb = LEXY_VERIFY("acb");
        CHECK(acb == 'a');
        auto bac = LEXY_VERIFY("bac");
        CHECK(bac == 'b');
        auto bca = LEXY_VERIFY("bca");
        CHECK(bca == 'b');
        auto cab = LEXY_VERIFY("cab");
        CHECK(cab == 'c');
        auto cba = LEXY_VERIFY("cba");
        CHECK(cba == 'c');

        auto aab = LEXY_VERIFY("aab");
        CHECK(aab == -1);
        auto aba = LEXY_VERIFY("aba");
        CHECK(aba == -1);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == -2);

        auto abca = LEXY_VERIFY("abca");
        CHECK(abca == 'a');
    }
    SUBCASE(".duplicate_error")
    {
        static constexpr auto rule
            = lexy::dsl::combination(LEXY_LIT("a"), LEXY_LIT("b") >> label<0>,
                                     LEXY_LIT("c") >> label<1>)
                  .duplicate_error<struct tag>;
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

                    LEXY_VERIFY_FN void operator()(id<0>)
                    {
                        ++count;
                    }
                    LEXY_VERIFY_FN void operator()(id<1>)
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
                LEXY_VERIFY_CHECK(count == 2);
                LEXY_VERIFY_CHECK(cur - str == 3);
                return *str;
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.end() - e.begin() == 1);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice>)
            {
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 'a');
        auto acb = LEXY_VERIFY("acb");
        CHECK(acb == 'a');
        auto bac = LEXY_VERIFY("bac");
        CHECK(bac == 'b');
        auto bca = LEXY_VERIFY("bca");
        CHECK(bca == 'b');
        auto cab = LEXY_VERIFY("cab");
        CHECK(cab == 'c');
        auto cba = LEXY_VERIFY("cba");
        CHECK(cba == 'c');

        auto aab = LEXY_VERIFY("aab");
        CHECK(aab == -1);
        auto aba = LEXY_VERIFY("aba");
        CHECK(aba == -1);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == -2);

        auto abca = LEXY_VERIFY("abca");
        CHECK(abca == 'a');
    }
    SUBCASE(".missing_error")
    {
        static constexpr auto rule
            = lexy::dsl::combination(LEXY_LIT("a"), LEXY_LIT("b") >> label<0>,
                                     LEXY_LIT("c") >> label<1>)
                  .missing_error<struct tag>;
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

                    LEXY_VERIFY_FN void operator()(id<0>)
                    {
                        ++count;
                    }
                    LEXY_VERIFY_FN void operator()(id<1>)
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
                LEXY_VERIFY_CHECK(count == 2);
                LEXY_VERIFY_CHECK(cur - str == 3);
                return *str;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::combination_duplicate> e)
            {
                LEXY_VERIFY_CHECK(e.end() - e.begin() == 1);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<tag>)
            {
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 'a');
        auto acb = LEXY_VERIFY("acb");
        CHECK(acb == 'a');
        auto bac = LEXY_VERIFY("bac");
        CHECK(bac == 'b');
        auto bca = LEXY_VERIFY("bca");
        CHECK(bca == 'b');
        auto cab = LEXY_VERIFY("cab");
        CHECK(cab == 'c');
        auto cba = LEXY_VERIFY("cba");
        CHECK(cba == 'c');

        auto aab = LEXY_VERIFY("aab");
        CHECK(aab == -1);
        auto aba = LEXY_VERIFY("aba");
        CHECK(aba == -1);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == -2);

        auto abca = LEXY_VERIFY("abca");
        CHECK(abca == 'a');
    }
}

TEST_CASE("dsl::partial_combination()")
{
    static constexpr auto rule
        = lexy::dsl::partial_combination(LEXY_LIT("a") >> label<0>, LEXY_LIT("b") >> label<1>,
                                         LEXY_LIT("c") >> label<2>);
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

                LEXY_VERIFY_FN void operator()(id<0>)
                {
                    ++count;
                }
                LEXY_VERIFY_FN void operator()(id<1>)
                {
                    ++count;
                }
                LEXY_VERIFY_FN void operator()(id<2>)
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
            LEXY_VERIFY_CHECK(cur - str == count);
            return count;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::combination_duplicate> e)
        {
            LEXY_VERIFY_CHECK(e.end() - e.begin() == 1);
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto a = LEXY_VERIFY("a");
    CHECK(a == 1);
    auto b = LEXY_VERIFY("b");
    CHECK(b == 1);
    auto c = LEXY_VERIFY("c");
    CHECK(c == 1);

    auto ab = LEXY_VERIFY("ab");
    CHECK(ab == 2);
    auto ba = LEXY_VERIFY("ba");
    CHECK(ba == 2);
    auto ac = LEXY_VERIFY("ac");
    CHECK(ac == 2);
    auto ca = LEXY_VERIFY("ca");
    CHECK(ca == 2);
    auto bc = LEXY_VERIFY("bc");
    CHECK(bc == 2);
    auto cb = LEXY_VERIFY("cb");
    CHECK(cb == 2);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 3);
    auto acb = LEXY_VERIFY("acb");
    CHECK(acb == 3);
    auto bac = LEXY_VERIFY("bac");
    CHECK(bac == 3);
    auto bca = LEXY_VERIFY("bca");
    CHECK(bca == 3);
    auto cab = LEXY_VERIFY("cab");
    CHECK(cab == 3);
    auto cba = LEXY_VERIFY("cba");
    CHECK(cba == 3);

    auto aab = LEXY_VERIFY("aab");
    CHECK(aab == -1);
    auto aba = LEXY_VERIFY("aba");
    CHECK(aba == -1);

    auto abca = LEXY_VERIFY("abca");
    CHECK(abca == 3);
}

