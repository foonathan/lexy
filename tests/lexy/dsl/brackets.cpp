// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the to-level directory of this distribution.

#include <lexy/dsl/brackets.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/while.hpp>

TEST_CASE("dsl::bracketed")
{
    constexpr auto inner = LEXY_LIT("abc");

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                using return_type = void;

                LEXY_VERIFY_FN void operator()() {}

                LEXY_VERIFY_FN void finish() && {}
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }
        LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
        {
            return -1;
        }
        LEXY_VERIFY_FN int error(test_error<lexy::unexpected_trailing_separator>)
        {
            return -2;
        }
    };

    SUBCASE("round_brackets")
    {
        static constexpr auto rule       = lexy::dsl::round_bracketed(inner);
        constexpr auto        equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto result = LEXY_VERIFY("(abc)");
        CHECK(result == 5);
    }
    SUBCASE("square_brackets")
    {
        static constexpr auto rule       = lexy::dsl::square_bracketed(inner);
        constexpr auto        equivalent = LEXY_LIT("[") >> inner + LEXY_LIT("]");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto result = LEXY_VERIFY("[abc]");
        CHECK(result == 5);
    }
    SUBCASE("curly_brackets")
    {
        static constexpr auto rule       = lexy::dsl::curly_bracketed(inner);
        constexpr auto        equivalent = LEXY_LIT("{") >> inner + LEXY_LIT("}");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto result = LEXY_VERIFY("{abc}");
        CHECK(result == 5);
    }
    SUBCASE("angle_brackets")
    {
        static constexpr auto rule       = lexy::dsl::angle_bracketed(inner);
        constexpr auto        equivalent = LEXY_LIT("<") >> inner + LEXY_LIT(">");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto result = LEXY_VERIFY("<abc>");
        CHECK(result == 5);
    }
    SUBCASE("parentheses")
    {
        static constexpr auto rule       = lexy::dsl::parenthesized(inner);
        constexpr auto        equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto result = LEXY_VERIFY("(abc)");
        CHECK(result == 5);
    }

    SUBCASE(".limit()")
    {
        constexpr auto rule = lexy::dsl::parenthesized.limit(LEXY_LIT("a")).recovery_rule();
        constexpr auto equivalent
            = lexy::dsl::recover(lexy::dsl::parenthesized.close()).limit(LEXY_LIT("a"));
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);
    }

    SUBCASE("try_")
    {
        static constexpr auto rule = lexy::dsl::parenthesized.try_(inner);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero.value == 2);
        CHECK(zero.errors(-1));
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);

        auto partial = LEXY_VERIFY("(ab)");
        CHECK(partial.value == 4);
        CHECK(partial.errors(-1));
        auto invalid = LEXY_VERIFY("(abdef)");
        CHECK(invalid.value == 7);
        CHECK(invalid.errors(-1));
    }
    SUBCASE("while")
    {
        static constexpr auto rule = lexy::dsl::parenthesized.while_(inner);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 2);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abcabc)");
        CHECK(two == 8);

        auto partial = LEXY_VERIFY("(abcab)");
        CHECK(partial.value == 7);
        CHECK(partial.errors(-1));
        auto invalid = LEXY_VERIFY("(abcabdef)");
        CHECK(invalid.value == 10);
        CHECK(invalid.errors(-1));
    }
    SUBCASE("while_one")
    {
        static constexpr auto rule = lexy::dsl::parenthesized.while_one(inner);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abcabc)");
        CHECK(two == 8);

        auto partial = LEXY_VERIFY("(abcab)");
        CHECK(partial.value == 7);
        CHECK(partial.errors(-1));
        auto invalid = LEXY_VERIFY("(abcabdef)");
        CHECK(invalid.value == 10);
        CHECK(invalid.errors(-1));
    }
    SUBCASE("opt")
    {
        static constexpr auto rule = lexy::dsl::parenthesized.opt(inner);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 2);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);

        auto partial = LEXY_VERIFY("(ab)");
        CHECK(partial.value == 4);
        CHECK(partial.errors(-1));
        auto invalid = LEXY_VERIFY("(abdef)");
        CHECK(invalid.value == 7);
        CHECK(invalid.errors(-1));
    }
    SUBCASE("list - no sep")
    {
        static constexpr auto rule = lexy::dsl::parenthesized.list(inner);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abcabc)");
        CHECK(two == 8);
    }
    SUBCASE("list - sep")
    {
        static constexpr auto rule
            = lexy::dsl::parenthesized.list(inner, lexy::dsl::sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abc,abc)");
        CHECK(two == 9);
        auto trailing = LEXY_VERIFY("(abc,abc,)");
        CHECK(trailing == -1);
    }
    SUBCASE("list - trailing sep")
    {
        static constexpr auto rule
            = lexy::dsl::parenthesized.list(inner, lexy::dsl::trailing_sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abc,abc)");
        CHECK(two == 9);
        auto trailing = LEXY_VERIFY("(abc,abc,)");
        CHECK(trailing == 10);
    }
    SUBCASE("list - no trailing sep")
    {
        static constexpr auto rule
            = lexy::dsl::parenthesized.list(inner, lexy::dsl::no_trailing_sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abc,abc)");
        CHECK(two == 9);
        auto trailing = LEXY_VERIFY("(abc,abc,)");
        CHECK(trailing.value == 10);
        CHECK(trailing.errors(-2));
    }
    SUBCASE("opt_list - no sep")
    {
        static constexpr auto rule = lexy::dsl::parenthesized.opt_list(inner);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 2);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abcabc)");
        CHECK(two == 8);
    }
    SUBCASE("opt_list - sep")
    {
        static constexpr auto rule
            = lexy::dsl::parenthesized.opt_list(inner, lexy::dsl::sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 2);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abc,abc)");
        CHECK(two == 9);
        auto trailing = LEXY_VERIFY("(abc,abc,)");
        CHECK(trailing == -1);
    }
    SUBCASE("opt_list - trailing sep")
    {
        static constexpr auto rule
            = lexy::dsl::parenthesized.opt_list(inner, lexy::dsl::trailing_sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 2);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abc,abc)");
        CHECK(two == 9);
        auto trailing = LEXY_VERIFY("(abc,abc,)");
        CHECK(trailing == 10);
    }
    SUBCASE("opt_list - no trailing sep")
    {
        static constexpr auto rule
            = lexy::dsl::parenthesized.opt_list(inner, lexy::dsl::no_trailing_sep(LEXY_LIT(",")));

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 2);
        auto one = LEXY_VERIFY("(abc)");
        CHECK(one == 5);
        auto two = LEXY_VERIFY("(abc,abc)");
        CHECK(two == 9);
        auto trailing = LEXY_VERIFY("(abc,abc,)");
        CHECK(trailing.value == 10);
        CHECK(trailing.errors(-2));
    }
}

