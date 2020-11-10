// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/choice.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::operator|")
{
    SUBCASE("pattern")
    {
        constexpr auto pattern = LEXY_LIT("abc") | LEXY_LIT("a") | LEXY_LIT("ab") | LEXY_LIT("def");
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match() == "a");

        constexpr auto ab = pattern_matches(pattern, "ab");
        CHECK(ab);
        CHECK(ab.match() == "a");

        constexpr auto def = pattern_matches(pattern, "def");
        CHECK(def);
        CHECK(def.match() == "def");
    }
    SUBCASE("branch pattern")
    {
        constexpr auto pattern = LEXY_LIT("a") >> LEXY_LIT("bc") | LEXY_LIT("a") | LEXY_LIT("ab");
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(!a);
        CHECK(a.match().empty());

        constexpr auto ab = pattern_matches(pattern, "ab");
        CHECK(!ab);
        CHECK(ab.match().empty());
    }
    SUBCASE("branch rule")
    {
        constexpr auto rule
            = LEXY_LIT("abc") >> lexy::dsl::id<0> | LEXY_LIT("def") >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                CONSTEXPR_CHECK(match == "abc");
                return 0;
            }
            constexpr int success(const char* cur, lexy::id<1>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                CONSTEXPR_CHECK(match == "def");
                return 1;
            }

            constexpr int error(test_error<lexy::exhausted_choice> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
        constexpr auto def = rule_matches<callback>(rule, "def");
        CHECK(def == 1);
    }
    SUBCASE("ordered rule")
    {
        constexpr auto rule = LEXY_LIT("a") | LEXY_LIT("abc");
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                auto match = lexy::_detail::string_view(str, cur);
                CONSTEXPR_CHECK(match == "a");
                return 0;
            }

            constexpr int error(test_error<lexy::exhausted_choice> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 0);
        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
    }
    SUBCASE("else")
    {
        constexpr auto rule
            = LEXY_LIT("abc") >> lexy::dsl::id<0> | lexy::dsl::else_ >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                CONSTEXPR_CHECK(match == "abc");
                return 0;
            }
            constexpr int success(const char* cur, lexy::id<1>)
            {
                CONSTEXPR_CHECK(cur == str);
                return 1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
    }
}

