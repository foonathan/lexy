// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/choice.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::operator|")
{
    SUBCASE("branch")
    {
        constexpr auto rule
            = LEXY_LIT("abc") >> lexy::dsl::id<0> | LEXY_LIT("def") >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);

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
    SUBCASE("ordered")
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
    SUBCASE("error")
    {
        struct tag;
        constexpr auto rule
            = LEXY_LIT("abc")
                  >> lexy::dsl::id<0> | LEXY_LIT("def") >> lexy::dsl::id<1> | lexy::dsl::error<tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

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

            constexpr int error(test_error<tag> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = verify<callback>(rule, "abc");
        CHECK(abc == 0);
        constexpr auto def = verify<callback>(rule, "def");
        CHECK(def == 1);
    }
}

