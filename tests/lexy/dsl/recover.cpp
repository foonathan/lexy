// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/recover.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

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
                // This used to be `int(cur - str)`, but for GCC 7 in Release mode, this was
                // negative, even though `cur >= str`. Changing this line fixes that miscompilation.
                // I could just be hiding some UB, but given that this is executed at compile-time,
                // where UB is (should be) a compile error, and there is no issue with any other
                // compiler, let's assume this is not the case and be proven wrong later.
                return int(lexy::_detail::string_view(str, cur).size());
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
                // Same deal as with the `dsl::find - no limit`, but this time to workaround GCC 7
                // in Debug mode.
                return int(lexy::_detail::string_view(str, cur).size());
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
}

