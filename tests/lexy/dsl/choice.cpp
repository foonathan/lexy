// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/choice.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::operator|")
{
    SUBCASE("simple")
    {
        static constexpr auto rule = LEXY_LIT("abc") >> label<0> | LEXY_LIT("def") >> label<1>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, id<1>)
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
        static constexpr auto rule = LEXY_LIT("abc") >> label<0> | lexy::dsl::else_ >> label<1>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_branch<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, id<1>)
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
            = LEXY_LIT("abc") >> label<0> | LEXY_LIT("def") >> label<1> | lexy::dsl::error<tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, id<0>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, id<1>)
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

    SUBCASE("as branch")
    {
        static constexpr auto rule = lexy::dsl::if_(LEXY_LIT("abc") >> label<1> //
                                                    | LEXY_LIT("def") >> label<2>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(str == cur);
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, id<1>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "abc");
                return 1;
            }
            LEXY_VERIFY_FN int success(const char* cur, id<2>)
            {
                auto match = lexy::_detail::string_view(str, cur);
                LEXY_VERIFY_CHECK(match == "def");
                return 2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 1);
        auto def = LEXY_VERIFY("def");
        CHECK(def == 2);
    }
}

