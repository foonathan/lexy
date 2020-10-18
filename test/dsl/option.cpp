// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/option.hpp>

#include "verify.hpp"

TEST_CASE("dsl::opt()")
{
    SUBCASE("pattern")
    {
        constexpr auto pattern = opt(LEXY_LIT("abc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto ab = pattern_matches(pattern, "ab");
        CHECK(ab);
        CHECK(ab.match().empty());
    }
    SUBCASE("rule")
    {
        constexpr auto rule = opt(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                if (cur == str)
                    return 0;
                else
                {
                    assert(cur - str == 3);
                    return 1;
                }
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 1);

        constexpr auto partial = rule_matches<callback>(rule, "ab");
        CHECK(partial == 0);
    }
    SUBCASE("pattern branch")
    {
        constexpr auto pattern = opt(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto ab = pattern_matches(pattern, "ab");
        CHECK(!ab);
        CHECK(ab.match().empty());
    }
    SUBCASE("rule branch")
    {
        constexpr auto rule = opt(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                if (cur == str)
                    return 0;
                else
                {
                    assert(cur - str == 3);
                    return 1;
                }
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 1);

        constexpr auto condition = rule_matches<callback>(rule, "a");
        CHECK(condition == -1);
        constexpr auto partial = rule_matches<callback>(rule, "ab");
        CHECK(partial == -1);
    }
}

