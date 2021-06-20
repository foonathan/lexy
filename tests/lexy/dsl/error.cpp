// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/error.hpp>

#include "verify.hpp"
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::error")
{
    struct tag;

    SUBCASE("no range")
    {
        static constexpr auto rule = lexy::dsl::error<tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == -1);
    }
    SUBCASE("range")
    {
        static constexpr auto rule = lexy::dsl::error<tag>(LEXY_LIT("ab") + LEXY_LIT("c"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                if (e.begin() != e.end())
                {
                    LEXY_VERIFY_CHECK(e.begin() == str);
                    LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                    return -1;
                }
                else
                {
                    LEXY_VERIFY_CHECK(e.position() == str);
                    return -2;
                }
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -2);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == -1);
    }
}

TEST_CASE("dsl::must()")
{
    SUBCASE("basic")
    {
        static constexpr auto rule = must(LEXY_LIT("a") >> LEXY_LIT("bc")).error<struct tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur - str == 3);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("bc"));
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 0);

        auto condition = LEXY_VERIFY("a");
        CHECK(condition == -2);
        auto partial = LEXY_VERIFY("ab");
        CHECK(partial == -2);

        auto bad = LEXY_VERIFY("123");
        CHECK(bad == -1);
    }
    SUBCASE("with range")
    {
        static constexpr auto rule
            = must(LEXY_LIT("a") >> LEXY_LIT("bc")).error<struct tag>(lexy::dsl::any);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur - str == 3);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<tag> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("bc"));
                return -2;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 0);

        auto condition = LEXY_VERIFY("a");
        CHECK(condition == -2);
        auto partial = LEXY_VERIFY("ab");
        CHECK(partial == -2);

        auto bad = LEXY_VERIFY("123");
        CHECK(bad == -1);
    }

    SUBCASE("as branch")
    {
        static constexpr auto rule = if_(must(LEXY_LIT("a") >> LEXY_LIT("bc")).error<struct tag>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                if (cur == str)
                    return 0;
                else
                {
                    LEXY_VERIFY_CHECK(cur - str == 3);
                    return 1;
                }
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("bc"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 1);

        auto condition = LEXY_VERIFY("a");
        CHECK(condition == -1);
        auto partial = LEXY_VERIFY("ab");
        CHECK(partial == -1);

        auto bad = LEXY_VERIFY("123");
        CHECK(bad == 0);
    }
}

