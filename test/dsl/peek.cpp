// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/peek.hpp>

#include "verify.hpp"
#include <lexy/dsl/not.hpp>

TEST_CASE("dsl::peek()")
{
    constexpr auto rule = lexy::dsl::peek(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(abc);
        CHECK(abc.match().empty());
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "abc");
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
    }

    constexpr auto ws_rule = lexy::dsl::peek(LEXY_LIT("abc"))[LEXY_LIT(" ")];
    CHECK(lexy::is_rule<decltype(ws_rule)>);
    CHECK(lexy::is_pattern<decltype(ws_rule)>);

    SUBCASE("whitespace pattern")
    {
        constexpr auto empty = pattern_matches(ws_rule, "");
        CHECK(!empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(ws_rule, "abc");
        CHECK(abc);
        CHECK(abc.match().empty());

        constexpr auto space = pattern_matches(ws_rule, "  abc");
        CHECK(space);
        CHECK(space.match() == "  ");
    }
    SUBCASE("whitespace rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "abc");
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(ws_rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(ws_rule, "abc");
        CHECK(abc == 0);

        constexpr auto space = rule_matches<callback>(ws_rule, "  abc");
        CHECK(space == 2);
    }
}

TEST_CASE("dsl::peek(!)")
{
    constexpr auto rule = lexy::dsl::peek(!LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(!abc);
        CHECK(abc.match().empty());
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }

            constexpr int error(test_error<lexy::unexpected> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == -1);
    }
}

