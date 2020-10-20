// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/switch.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/value.hpp>
#include <lexy/dsl/while.hpp>

TEST_CASE("dsl::switch_()")
{
    SUBCASE("basic")
    {
        constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                  .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                  .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                assert(str + i == cur);
                return i;
            }

            constexpr int error(test_error<lexy::exhausted_switch> e)
            {
                assert(e.begin() == str);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 2);

        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == -1);
    }
    SUBCASE("success")
    {
        constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                  .case_(lexy::dsl::success)
                                  .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                  .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                assert(cur == str);
                return 0;
            }
            constexpr int success(const char* cur, int i)
            {
                assert(i > 0);
                assert(str + i == cur);
                return i;
            }

            constexpr int error(test_error<lexy::exhausted_switch> e)
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
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 2);

        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == -1);
    }
    SUBCASE("ordered")
    {
        constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                  .case_(LEXY_LIT("a") + lexy::dsl::any >> lexy::dsl::value_c<1>)
                                  .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                for (auto iter = str; iter != cur; ++iter)
                    assert(*iter == 'a');
                return i;
            }

            constexpr int error(test_error<lexy::exhausted_switch> e)
            {
                assert(e.begin() == str);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 1);
        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == 1);
    }
    SUBCASE("default")
    {
        constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                  .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                  .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>)
                                  .default_(lexy::dsl::id<0>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                assert(str + i == cur);
                return i;
            }
            constexpr int success(const char* cur, lexy::id<0>)
            {
                for (auto iter = str; iter != cur; ++iter)
                    assert(*iter == 'a');
                return 0;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 2);

        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == 0);
    }
    SUBCASE("error")
    {
        constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                  .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                  .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>)
                                  .error<struct tag>();
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                assert(str + i == cur);
                return i;
            }

            constexpr int error(test_error<tag> e)
            {
                assert(e.begin() == str);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "a");
        CHECK(a == 1);
        constexpr auto aa = rule_matches<callback>(rule, "aa");
        CHECK(aa == 2);

        constexpr auto aaa = rule_matches<callback>(rule, "aaa");
        CHECK(aaa == -1);
    }
}

