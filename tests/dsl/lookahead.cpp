// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/lookahead.hpp>

#include "verify.hpp"

TEST_CASE("dsl::lookahead()")
{
    constexpr auto rule = lookahead(LEXY_LIT("!"), LEXY_LIT("\n"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);
        CHECK(empty.match().empty());

        constexpr auto nothing = pattern_matches(rule, "abc");
        CHECK(!nothing);
        CHECK(nothing.match().empty());

        constexpr auto nothing_newline = pattern_matches(rule, "abc\n");
        CHECK(!nothing_newline);
        CHECK(nothing_newline.match().empty());

        constexpr auto something = pattern_matches(rule, "abc!def\n");
        CHECK(something);
        CHECK(something.match().empty());

        constexpr auto something_after = pattern_matches(rule, "abc\n!def\n");
        CHECK(!something_after);
        CHECK(something_after.match().empty());
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

            constexpr int error(test_error<lexy::lookahead_failure> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto nothing = rule_matches<callback>(rule, "abc");
        CHECK(nothing == -1);

        constexpr auto nothing_newline = rule_matches<callback>(rule, "abc\n");
        CHECK(nothing_newline == -1);

        constexpr auto something = rule_matches<callback>(rule, "abc!def\n");
        CHECK(something == 0);

        constexpr auto something_after = rule_matches<callback>(rule, "abc\n!def\n");
        CHECK(something_after == -1);
    }
}

