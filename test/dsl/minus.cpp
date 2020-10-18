// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/minus.hpp>

#include "verify.hpp"
#include <lexy/dsl/while.hpp>

TEST_CASE("dsl::operator-")
{
    SUBCASE("pattern")
    {
        constexpr auto pattern = while_(LEXY_LIT("a")) - LEXY_LIT("aa");
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match() == "a");
        constexpr auto aaa = pattern_matches(pattern, "aaa");
        CHECK(aaa);
        CHECK(aaa.match() == "aaa");

        constexpr auto aa = pattern_matches(pattern, "aa");
        CHECK(!aa);
        CHECK(aa.match().empty());
    }
    SUBCASE("rule")
    {
        constexpr auto rule = while_(LEXY_LIT("a")) - LEXY_LIT("aa");
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::minus_failure> e)
            {
                assert(e.begin() == str);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == 3);

        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == -1);
    }
    SUBCASE("sequence")
    {
        constexpr auto pattern = while_(LEXY_LIT("a")) - LEXY_LIT("a") - LEXY_LIT("aa");
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(!a);
        CHECK(a.match().empty());
        constexpr auto aa = pattern_matches(pattern, "aa");
        CHECK(!aa);
        CHECK(aa.match().empty());

        constexpr auto aaa = pattern_matches(pattern, "aaa");
        CHECK(aaa);
        CHECK(aaa.match() == "aaa");
    }
}

