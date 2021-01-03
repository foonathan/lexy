// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
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
        static constexpr auto rule
            = LEXY_LIT("abc") >> lexy::dsl::id<0> | LEXY_LIT("def") >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::id<1>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "def");
                return 1;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
        auto def = LEXY_VERIFY("def");
        CHECK(def == 1);
    }
    SUBCASE("ordered")
    {
        static constexpr auto rule = LEXY_LIT("a") | LEXY_LIT("abc");
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "a");
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::exhausted_choice> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto a = LEXY_VERIFY("a");
        CHECK(a == 0);
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
    SUBCASE("else")
    {
        static constexpr auto rule
            = LEXY_LIT("abc") >> lexy::dsl::id<0> | lexy::dsl::else_ >> lexy::dsl::id<1>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::id<1>)
            {
                LEXY_VERIFY_CHECK(cur == str);
                return 1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
    }
    SUBCASE("error")
    {
        struct tag;
        static constexpr auto rule
            = LEXY_LIT("abc")
                  >> lexy::dsl::id<0> | LEXY_LIT("def") >> lexy::dsl::id<1> | lexy::dsl::error<tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::id<1>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "def");
                return 1;
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
        auto def = LEXY_VERIFY("def");
        CHECK(def == 1);
    }
}

