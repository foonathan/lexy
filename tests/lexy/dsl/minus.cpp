// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/minus.hpp>

#include "verify.hpp"
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/until.hpp>

TEST_CASE("dsl::operator-")
{
    SUBCASE("rule")
    {
        constexpr auto rule = until(LEXY_LIT("!")) - LEXY_LIT("aa!");
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str);
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.position() == lexy::_detail::string_view(str).end());
                CONSTEXPR_CHECK(e.string() == "!");
                return -1;
            }
            constexpr int error(test_error<lexy::minus_failure> e)
            {
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);
        constexpr auto zero = rule_matches<callback>(rule, "!");
        CHECK(zero == 1);

        constexpr auto a = rule_matches<callback>(rule, "a!");
        CHECK(a == 2);
        constexpr auto aaa = rule_matches<callback>(rule, "aaa!");
        CHECK(aaa == 4);

        constexpr auto aa = rule_matches<callback>(rule, "aa!");
        CHECK(aa == -2);
    }
    SUBCASE("sequence")
    {
        constexpr auto pattern = until(LEXY_LIT("!")) - LEXY_LIT("a!") - LEXY_LIT("aa!");

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);
        constexpr auto zero = pattern_matches(pattern, "!");
        CHECK(zero);
        CHECK(zero.match() == "!");

        constexpr auto a = pattern_matches(pattern, "a!");
        CHECK(!a);
        CHECK(a.match().empty());
        constexpr auto aa = pattern_matches(pattern, "aa!");
        CHECK(!aa);
        CHECK(aa.match().empty());

        constexpr auto aaa = pattern_matches(pattern, "aaa!");
        CHECK(aaa);
        CHECK(aaa.match() == "aaa!");
    }
    SUBCASE("any")
    {
        constexpr auto pattern = until(LEXY_LIT("!")) - lexy::dsl::any;

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);
        constexpr auto zero = pattern_matches(pattern, "!");
        CHECK(!zero);

        constexpr auto a = pattern_matches(pattern, "a!");
        CHECK(!a);
        CHECK(a.match().empty());
        constexpr auto aa = pattern_matches(pattern, "aa!");
        CHECK(!aa);
        CHECK(aa.match().empty());
        constexpr auto aaa = pattern_matches(pattern, "aaa!");
        CHECK(!aaa);
        CHECK(aaa.match().empty());
    }
}

