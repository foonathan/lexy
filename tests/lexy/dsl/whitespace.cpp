// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#define LEXY_IGNORE_DEPRECATED_WHITESPACE
#include <lexy/dsl/whitespace.hpp>

#include "verify.hpp"
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/production.hpp>

namespace
{
struct ws_production
{
    static constexpr auto whitespace = LEXY_LIT(" ");
};
} // namespace

TEST_CASE("dsl::whitespace")
{
    SUBCASE("explicit token")
    {
        static constexpr auto rule = lexy::dsl::whitespace(LEXY_LIT(" "));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto one = LEXY_VERIFY(" ");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("  ");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("   ");
        CHECK(three == 3);
    }
    SUBCASE("explicit branch")
    {
        static constexpr auto rule = lexy::dsl::whitespace(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) / 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "bc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto one = LEXY_VERIFY("abc");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("abcabc");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("abcabcabc");
        CHECK(three == 3);

        auto partial = LEXY_VERIFY("ab");
        CHECK(partial == -1);
    }
    SUBCASE("explicit choice")
    {
        static constexpr auto rule
            = lexy::dsl::whitespace(LEXY_LIT("a") >> LEXY_LIT("bc") | LEXY_LIT("123"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) / 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "bc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto one = LEXY_VERIFY("abc");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("123abc");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("abc123abc");
        CHECK(three == 3);

        auto partial_branch = LEXY_VERIFY("ab");
        CHECK(partial_branch == -1);
        auto partial_token = LEXY_VERIFY("12");
        CHECK(partial_token == 0);
    }

    SUBCASE("explicit whitespace operators")
    {
        static constexpr auto base = lexy::dsl::whitespace(LEXY_LIT("abc"));

        static constexpr auto choice_right = base | LEXY_LIT("123");
        static constexpr auto choice_right_eq
            = lexy::dsl::whitespace(LEXY_LIT("abc") | LEXY_LIT("123"));
        CHECK(std::is_same_v<decltype(choice_right), decltype(choice_right_eq)>);

        static constexpr auto choice_left = LEXY_LIT("123") | base;
        static constexpr auto choice_left_eq
            = lexy::dsl::whitespace(LEXY_LIT("123") | LEXY_LIT("abc"));
        CHECK(std::is_same_v<decltype(choice_left), decltype(choice_left_eq)>);

        static constexpr auto alt_right = base / LEXY_LIT("123");
        static constexpr auto alt_right_eq
            = lexy::dsl::whitespace(LEXY_LIT("abc") / LEXY_LIT("123"));
        CHECK(std::is_same_v<decltype(alt_right), decltype(alt_right_eq)>);

        static constexpr auto alt_left = LEXY_LIT("123") / base;
        static constexpr auto alt_left_eq
            = lexy::dsl::whitespace(LEXY_LIT("123") / LEXY_LIT("abc"));
        CHECK(std::is_same_v<decltype(alt_left), decltype(alt_left_eq)>);
    }

    SUBCASE("automatic whitespace - none")
    {
        static constexpr auto rule = lexy::dsl::whitespace;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);
        auto spaces = LEXY_VERIFY("  ");
        CHECK(spaces == 0);
    }
    SUBCASE("automatic whitespace - token")
    {
        static constexpr auto rule = lexy::dsl::whitespace;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str);
            }
        };

        auto empty = LEXY_VERIFY_PRODUCTION(ws_production, "");
        CHECK(empty == 0);

        auto one = LEXY_VERIFY_PRODUCTION(ws_production, " ");
        CHECK(one == 1);
        auto two = LEXY_VERIFY_PRODUCTION(ws_production, "  ");
        CHECK(two == 2);
        auto three = LEXY_VERIFY_PRODUCTION(ws_production, "   ");
        CHECK(three == 3);
    }
}

TEST_CASE("dsl::no_whitespace()")
{
    SUBCASE("token")
    {
        static constexpr auto rule = no_whitespace(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY_PRODUCTION(ws_production, "");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY_PRODUCTION(ws_production, "abc");
        CHECK(abc == 0);
        auto abc_space = LEXY_VERIFY_PRODUCTION(ws_production, "abc   ");
        CHECK(abc_space == 3);

        auto inner_space = LEXY_VERIFY_PRODUCTION(ws_production, "a bc");
        CHECK(inner_space == -1);
    }
    SUBCASE("sequence")
    {
        static constexpr auto rule = no_whitespace(LEXY_LIT("a") + LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
            {
                return -1;
            }
        };

        auto empty = LEXY_VERIFY_PRODUCTION(ws_production, "");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY_PRODUCTION(ws_production, "abc");
        CHECK(abc == 0);
        auto abc_space = LEXY_VERIFY_PRODUCTION(ws_production, "abc   ");
        CHECK(abc_space == 3);

        auto inner_space = LEXY_VERIFY_PRODUCTION(ws_production, "a bc");
        CHECK(inner_space == -1);
    }
}

namespace
{
struct token_production : lexy::token_production
{
    static constexpr auto rule = LEXY_LIT("a") + LEXY_LIT("bc");
};
} // namespace

TEST_CASE("automatic whitespace")
{
    SUBCASE("tokens")
    {
        static constexpr auto rule = LEXY_LIT("a") + LEXY_LIT("bc");
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
            {
                return -1;
            }
        };

        auto empty = LEXY_VERIFY_PRODUCTION(ws_production, "");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY_PRODUCTION(ws_production, "abc");
        CHECK(abc == 0);
        auto abc_space = LEXY_VERIFY_PRODUCTION(ws_production, "abc   ");
        CHECK(abc_space == 3);

        auto inner_space = LEXY_VERIFY_PRODUCTION(ws_production, "a bc");
        CHECK(inner_space == 1);
    }
    SUBCASE("token production")
    {
        static constexpr auto rule = lexy::dsl::p<token_production>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(token_production)
            {
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, int)
            {
                return int(cur - str) - 3;
            }

            LEXY_VERIFY_FN int error(token_production, test_error<lexy::expected_literal>)
            {
                return -1;
            }
        };

        auto empty = LEXY_VERIFY_PRODUCTION(ws_production, "");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY_PRODUCTION(ws_production, "abc");
        CHECK(abc == 0);
        auto abc_space = LEXY_VERIFY_PRODUCTION(ws_production, "abc   ");
        CHECK(abc_space == 3);

        auto inner_space = LEXY_VERIFY_PRODUCTION(ws_production, "a bc");
        CHECK(inner_space == -1);
    }
}

TEST_CASE("dsl::whitespaced()")
{
    SUBCASE("simple")
    {
        static constexpr auto rule = whitespaced(LEXY_LIT("abc"), LEXY_LIT(" "));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 0);

        auto space_abc = LEXY_VERIFY(" abc");
        CHECK(space_abc == 1);
        auto space_space_abc = LEXY_VERIFY("  abc");
        CHECK(space_space_abc == 2);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule_
            = whitespaced(LEXY_LIT("abc") >> lexy::dsl::id<0>, LEXY_LIT(" "));
        CHECK(lexy::is_rule<decltype(rule_)>);
        CHECK(lexy::is_branch<decltype(rule_)>);

        static constexpr auto rule = if_(rule_);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str);
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::id<0>)
            {
                return int(cur - str);
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc == 3);

        auto space_abc = LEXY_VERIFY(" abc");
        CHECK(space_abc == 4);
        auto space_space_abc = LEXY_VERIFY("  abc");
        CHECK(space_space_abc == 5);
    }
}

