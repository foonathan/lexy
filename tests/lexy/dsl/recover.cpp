// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/recover.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/newline.hpp>

TEST_CASE("dsl::find")
{
    SUBCASE("no limit")
    {
        static constexpr auto rule = lexy::dsl::find(lexy::dsl::ascii::space, LEXY_LIT("!"));
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
        CHECK(!empty.recovered);

        auto space0 = LEXY_VERIFY(" ");
        CHECK(space0 == 0);
        auto space1 = LEXY_VERIFY("- ");
        CHECK(space1 == 1);
        auto space2 = LEXY_VERIFY("-- ");
        CHECK(space2 == 2);

        auto excl0 = LEXY_VERIFY("!");
        CHECK(excl0 == 0);
        auto excl1 = LEXY_VERIFY("-!");
        CHECK(excl1 == 1);
        auto excl2 = LEXY_VERIFY("--!");
        CHECK(excl2 == 2);

        auto space_then_excl = LEXY_VERIFY("- !");
        CHECK(space_then_excl == 1);

        auto missing = LEXY_VERIFY("abc");
        CHECK(!missing.recovered);
    }
    SUBCASE("limit")
    {
        static constexpr auto rule
            = lexy::dsl::find(lexy::dsl::ascii::space, LEXY_LIT("!")).limit(LEXY_LIT("$"));
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
        CHECK(!empty.recovered);

        auto space0 = LEXY_VERIFY(" ");
        CHECK(space0 == 0);
        auto space1 = LEXY_VERIFY("- ");
        CHECK(space1 == 1);
        auto space2 = LEXY_VERIFY("-- ");
        CHECK(space2 == 2);

        auto excl0 = LEXY_VERIFY("!");
        CHECK(excl0 == 0);
        auto excl1 = LEXY_VERIFY("-!");
        CHECK(excl1 == 1);
        auto excl2 = LEXY_VERIFY("--!");
        CHECK(excl2 == 2);

        auto space_then_excl = LEXY_VERIFY("- !");
        CHECK(space_then_excl == 1);

        auto missing = LEXY_VERIFY("abc");
        CHECK(!missing.recovered);
        auto limited = LEXY_VERIFY("--$!");
        CHECK(!limited.recovered);
    }
}

TEST_CASE("dsl::recover_")
{
    SUBCASE("no limit")
    {
        static constexpr auto rule = lexy::dsl::recover(lexy::dsl::ascii::space, LEXY_LIT("!"));
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
        CHECK(!empty.recovered);

        auto space0 = LEXY_VERIFY(" ");
        CHECK(space0 == 1);
        auto space1 = LEXY_VERIFY("- ");
        CHECK(space1 == 2);
        auto space2 = LEXY_VERIFY("-- ");
        CHECK(space2 == 3);

        auto excl0 = LEXY_VERIFY("!");
        CHECK(excl0 == 1);
        auto excl1 = LEXY_VERIFY("-!");
        CHECK(excl1 == 2);
        auto excl2 = LEXY_VERIFY("--!");
        CHECK(excl2 == 3);

        auto space_then_excl = LEXY_VERIFY("- !");
        CHECK(space_then_excl == 2);

        auto missing = LEXY_VERIFY("abc");
        CHECK(!missing.recovered);
    }
    SUBCASE("limit")
    {
        static constexpr auto rule
            = lexy::dsl::recover(lexy::dsl::ascii::space, LEXY_LIT("!")).limit(LEXY_LIT("$"));
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
        CHECK(!empty.recovered);

        auto space0 = LEXY_VERIFY(" ");
        CHECK(space0 == 1);
        auto space1 = LEXY_VERIFY("- ");
        CHECK(space1 == 2);
        auto space2 = LEXY_VERIFY("-- ");
        CHECK(space2 == 3);

        auto excl0 = LEXY_VERIFY("!");
        CHECK(excl0 == 1);
        auto excl1 = LEXY_VERIFY("-!");
        CHECK(excl1 == 2);
        auto excl2 = LEXY_VERIFY("--!");
        CHECK(excl2 == 3);

        auto space_then_excl = LEXY_VERIFY("- !");
        CHECK(space_then_excl == 2);

        auto missing = LEXY_VERIFY("abc");
        CHECK(!missing.recovered);
        auto limited = LEXY_VERIFY("--$!");
        CHECK(!limited.recovered);
    }
}

TEST_CASE("dsl::try_")
{
    SUBCASE("catch")
    {
        static constexpr auto rule = lexy::dsl::try_(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.value == 0);
        CHECK(empty.errors(-1));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 3);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.value == 2);
        CHECK(ab.errors(-1));
        auto abd = LEXY_VERIFY("abd");
        CHECK(abd.value == 2);
        CHECK(abd.errors(-1));
    }
    SUBCASE("recover")
    {
        static constexpr auto rule
            = lexy::dsl::try_(LEXY_LIT("abc"),
                              lexy::dsl::recover(lexy::dsl::ascii::space, LEXY_LIT("!")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
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

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 3);

        auto recover_space0 = LEXY_VERIFY("ab ");
        CHECK(recover_space0.value == 3);
        CHECK(recover_space0.errors(-1));
        auto recover_space1 = LEXY_VERIFY("ab- ");
        CHECK(recover_space1.value == 4);
        CHECK(recover_space1.errors(-1));
        auto recover_space2 = LEXY_VERIFY("ab-- ");
        CHECK(recover_space2.value == 5);
        CHECK(recover_space2.errors(-1));

        auto recover_excl0 = LEXY_VERIFY("ab!");
        CHECK(recover_excl0.value == 3);
        CHECK(recover_excl0.errors(-1));
        auto recover_excl1 = LEXY_VERIFY("ab-!");
        CHECK(recover_excl1.value == 4);
        CHECK(recover_excl1.errors(-1));
        auto recover_excl2 = LEXY_VERIFY("ab--!");
        CHECK(recover_excl2.value == 5);
        CHECK(recover_excl2.errors(-1));

        auto recover_failed = LEXY_VERIFY("ab---");
        CHECK(recover_failed == -1);
    }

    SUBCASE("rule in sequence with try")
    {
        static constexpr auto rule = lexy::dsl::try_(LEXY_LIT("abc")) + lexy::dsl::newline;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("newline"));
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.value == -1);
        CHECK(empty.errors(-1, -2));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == -2);
        auto abc_nl = LEXY_VERIFY("abc\n");
        CHECK(abc_nl == 4);

        auto ab = LEXY_VERIFY("ab\n");
        CHECK(ab.value == 3);
        CHECK(ab.errors(-1));
        auto abd = LEXY_VERIFY("abd\n");
        CHECK(abd.value == -1);
        CHECK(abd.errors(-1, -2));
    }

    SUBCASE("branch")
    {
        static constexpr auto rule
            = lexy::dsl::while_(lexy::dsl::try_(LEXY_LIT("abc") >> LEXY_LIT("!")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("!"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab == 0);
        auto abd = LEXY_VERIFY("abd");
        CHECK(abd == 0);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.value == 3);
        CHECK(abc.errors(-1));

        auto one = LEXY_VERIFY("abc!");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc!abc!");
        CHECK(two == 8);
        auto three = LEXY_VERIFY("abc!abc!abc!");
        CHECK(three == 12);

        auto recover_first = LEXY_VERIFY("abcabc!abc!");
        CHECK(recover_first.value == 11);
        CHECK(recover_first.errors(-1));
        auto recover_second = LEXY_VERIFY("abc!abcabc!");
        CHECK(recover_second.value == 11);
        CHECK(recover_second.errors(-1));

        auto recover_all = LEXY_VERIFY("abcabcabc");
        CHECK(recover_all.value == 9);
        CHECK(recover_all.errors(-1, -1, -1));
    }
}

