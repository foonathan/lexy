// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/production.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("rule: p")
{
    SUBCASE("basic")
    {
        struct prod
        {
            LEXY_CONSTEVAL auto rule()
            {
                return LEXY_LIT("abc") + lexy::dsl::id<0>;
            }
        };
        constexpr auto rule = lexy::dsl::p<prod>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(prod, const char* cur, lexy::id<0>)
            {
                assert(cur - str == 3);
                return 0;
            }
            constexpr int success(const char*, int result)
            {
                assert(result == 0);
                return result;
            }

            constexpr int error(prod, test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
    }
    SUBCASE("branch")
    {
        struct prod
        {
            LEXY_CONSTEVAL auto rule()
            {
                return LEXY_LIT("abc") >> lexy::dsl::id<0>;
            }
        };
        constexpr auto rule = lexy::dsl::p<prod> | LEXY_LIT("def") >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(prod, const char* cur, lexy::id<0>)
            {
                assert(lexy::_detail::string_view(str, cur) == "abc");
                return 0;
            }

            constexpr int success(const char*, int result)
            {
                assert(result == 0);
                return result;
            }
            constexpr int success(const char* cur, lexy::id<1>)
            {
                assert(lexy::_detail::string_view(str, cur) == "def");
                return 1;
            }

            constexpr int error(test_error<lexy::exhausted_choice> e)
            {
                assert(e.position() == str);
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
}

TEST_CASE("rule: recurse")
{
    SUBCASE("indirect recursion")
    {
        struct outer;
        struct inner
        {
            LEXY_CONSTEVAL auto rule()
            {
                return lexy::dsl::recurse<outer>;
            }
        };
        struct outer
        {
            LEXY_CONSTEVAL auto rule()
            {
                return opt(LEXY_LIT("a") >> lexy::dsl::p<inner>);
            }
        };
        constexpr auto rule = lexy::dsl::p<outer>;

        struct callback
        {
            const char* str;

            constexpr int success(inner, const char*, int outer_result)
            {
                return outer_result + 1;
            }
            constexpr int success(outer, const char*)
            {
                return 0;
            }
            constexpr int success(outer, const char*, int inner_result)
            {
                return inner_result;
            }
            constexpr int success(const char*, int result)
            {
                return result;
            }

            int error(inner, int)
            {
                assert(false);
                return -1;
            }
            int error(outer, int)
            {
                assert(false);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 2);
        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == 3);
    }
    SUBCASE("right recursion")
    {
        struct prod
        {
            LEXY_CONSTEVAL auto rule()
            {
                return opt(LEXY_LIT("a") >> lexy::dsl::recurse<prod>);
            }
        };
        constexpr auto rule = lexy::dsl::p<prod>;

        struct callback
        {
            const char* str;

            constexpr int success(prod, const char*)
            {
                return 0;
            }
            constexpr int success(prod, const char*, int result)
            {
                return result + 1;
            }
            constexpr int success(const char*, int result)
            {
                return result;
            }

            int error(prod, int)
            {
                assert(false);
                return -1;
            }
            int error(int)
            {
                assert(false);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 2);
        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == 3);
    }
}

