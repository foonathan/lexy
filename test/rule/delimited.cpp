// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/delimited.hpp>

#include "verify.hpp"
#include <lexy/dsl/eof.hpp>
#include <lexy/match.hpp>

TEST_CASE("rule: delimited")
{
    SUBCASE("basic")
    {
        constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme<test_input> lex)
            {
                assert(lex.begin() == str + 1);
                assert(lex.end() == cur - 1);
                return int(lex.size());
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "(");
                assert(e.position() == str);
                return -1;
            }
            constexpr int error(test_error<lexy::missing_delimiter> e)
            {
                assert(e.position() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = rule_matches<callback>(rule, "()");
        CHECK(zero == 0);
        constexpr auto one = rule_matches<callback>(rule, "(a)");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, "(ab)");
        CHECK(two == 2);
        constexpr auto three = rule_matches<callback>(rule, "(abc)");
        CHECK(three == 3);

        constexpr auto unterminated = rule_matches<callback>(rule, "(abc");
        CHECK(unterminated == -2);
    }
    SUBCASE("whitespace")
    {
        constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"))[LEXY_LIT(" ")];
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme<test_input> lex)
            {
                assert(lex.begin() == str + 2);
                assert(lex.end() == cur - 1);
                return int(lex.size());
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "(" || e.string() == " ");
                assert(e.position() == str);
                return -1;
            }
            constexpr int error(test_error<lexy::missing_delimiter> e)
            {
                assert(e.position() == str + 2);
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto zero = rule_matches<callback>(rule, " ()");
        CHECK(zero == 0);
        constexpr auto one = rule_matches<callback>(rule, " (a)");
        CHECK(one == 1);
        constexpr auto two = rule_matches<callback>(rule, " (ab)");
        CHECK(two == 2);
        constexpr auto three = rule_matches<callback>(rule, " (abc)");
        CHECK(three == 3);

        constexpr auto unterminated = rule_matches<callback>(rule, " (abc");
        CHECK(unterminated == -2);
    }
    SUBCASE("predefined")
    {
        CHECK(lexy::match(lexy::zstring_input(R"("abc")"), lexy::dsl::quoted + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input(R"("""abc""")"),
                          lexy::dsl::triple_quoted + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input(R"('abc')"),
                          lexy::dsl::single_quoted + lexy::dsl::eof));

        CHECK(lexy::match(lexy::zstring_input("`abc`"), lexy::dsl::backticked + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("``abc``"),
                          lexy::dsl::double_backticked + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("```abc```"),
                          lexy::dsl::triple_backticked + lexy::dsl::eof));
    }
}

