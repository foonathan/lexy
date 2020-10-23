// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/delimited.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::delimited()")
{
    constexpr auto cp = lexy::dsl::ascii::character;

    SUBCASE("basic")
    {
        constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"))(cp);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme_for<test_input> lex)
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
                assert(e.message() == "missing delimiter");
                assert(e.begin() == str + 1);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -2;
            }
            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                assert(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
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

        constexpr auto invalid_ascii = rule_matches<callback>(rule, "(ab\xFF");
        CHECK(invalid_ascii == -3);
    }
    SUBCASE("whitespace")
    {
        constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"))[LEXY_LIT(" ")](cp);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme_for<test_input> lex)
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
            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                assert(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
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

        constexpr auto invalid_ascii = rule_matches<callback>(rule, " (ab\xFF");
        CHECK(invalid_ascii == -3);
    }
    SUBCASE("escape")
    {
        constexpr auto escape
            = lexy::dsl::backslash_escape.literal(LEXY_LIT(")"), LEXY_ESCAPE_VALUE("))"));
        constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"))(cp, escape);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    constexpr void operator()(char)
                    {
                        ++count;
                    }
                    constexpr void operator()(const char*, std::size_t length)
                    {
                        count += length;
                    }

                    constexpr int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            constexpr int success(const char* cur, int count)
            {
                assert(cur - str == count + 2);
                return count;
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
            constexpr int error(test_error<lexy::expected_char_class> e)
            {
                assert(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
            constexpr int error(test_error<lexy::invalid_escape_sequence>)
            {
                return -4;
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

        constexpr auto escaped = rule_matches<callback>(rule, "(a\\)b)");
        CHECK(escaped == 4);

        constexpr auto unterminated = rule_matches<callback>(rule, "(abc");
        CHECK(unterminated == -2);

        constexpr auto invalid_ascii = rule_matches<callback>(rule, "(ab\xFF");
        CHECK(invalid_ascii == -3);

        constexpr auto invalid_escape = rule_matches<callback>(rule, "(a\\fb)");
        CHECK(invalid_escape == -4);
    }
    SUBCASE("predefined")
    {
        CHECK(lexy::match(lexy::zstring_input(R"("abc")"), lexy::dsl::quoted(cp) + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input(R"("""abc""")"),
                          lexy::dsl::triple_quoted(cp) + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input(R"('abc')"),
                          lexy::dsl::single_quoted(cp) + lexy::dsl::eof));

        CHECK(
            lexy::match(lexy::zstring_input("`abc`"), lexy::dsl::backticked(cp) + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("``abc``"),
                          lexy::dsl::double_backticked(cp) + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("```abc```"),
                          lexy::dsl::triple_backticked(cp) + lexy::dsl::eof));
    }
}

