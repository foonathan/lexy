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
        static constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                         .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                         .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                CONSTEXPR_CHECK(str + i == cur);
                return i;
            }

            constexpr int error(test_error<lexy::exhausted_switch> e)
            {
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);

        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == -1);
    }
    SUBCASE("ordered")
    {
        static constexpr auto rule
            = switch_(while_(LEXY_LIT("a")))
                  .case_(LEXY_LIT("a") + lexy::dsl::any >> lexy::dsl::value_c<1>)
                  .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                for (auto iter = str; iter != cur; ++iter)
                    CONSTEXPR_CHECK(*iter == 'a');
                return i;
            }

            constexpr int error(test_error<lexy::exhausted_switch> e)
            {
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 1);
        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == 1);
    }
    SUBCASE("default")
    {
        static constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                         .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                         .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>)
                                         .default_(lexy::dsl::id<0>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                CONSTEXPR_CHECK(str + i == cur);
                return i;
            }
            constexpr int success(const char* cur, lexy::id<0>)
            {
                for (auto iter = str; iter != cur; ++iter)
                    CONSTEXPR_CHECK(*iter == 'a');
                return 0;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);

        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == 0);
    }
    SUBCASE("error")
    {
        static constexpr auto rule = switch_(while_(LEXY_LIT("a")))
                                         .case_(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                         .case_(LEXY_LIT("aa") >> lexy::dsl::value_c<2>)
                                         .error<struct tag>();
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                CONSTEXPR_CHECK(str + i == cur);
                return i;
            }

            constexpr int error(test_error<tag> e)
            {
                CONSTEXPR_CHECK(e.begin() == str);
                CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 1);
        auto aa = LEXY_VERIFY("aa");
        CHECK(aa == 2);

        auto aaa = LEXY_VERIFY("aaa");
        CHECK(aaa == -1);
    }
}

