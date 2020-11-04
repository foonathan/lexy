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

    SUBCASE("pattern")
    {
        constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"))(cp);
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

                    constexpr void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += lex.size();
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
                assert(e.begin() == str + 1);
                assert(e.end() == lexy::_detail::string_view(str).end());
                assert(e.message() == "missing delimiter");
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
    SUBCASE("branch")
    {
        constexpr auto rule = delimited(LEXY_LIT("(") >> lexy::dsl::value_c<0>,
                                        LEXY_LIT(")") >> lexy::dsl::value_c<1>)(cp);
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

                    constexpr void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += lex.size();
                    }

                    constexpr int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            constexpr int success(const char* cur, int open, int count, int close)
            {
                assert(cur - str == count + 2);
                assert(open == 0);
                assert(close == 1);
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
                assert(e.begin() == str + 1);
                assert(e.end() == lexy::_detail::string_view(str).end());
                assert(e.message() == "missing delimiter");
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
}

TEST_CASE("predefined dsl::delimited")
{
    constexpr auto cp = lexy::dsl::ascii::character;

    CHECK(lexy::match(lexy::zstring_input(R"("abc")"), lexy::dsl::quoted(cp) + lexy::dsl::eof));
    CHECK(lexy::match(lexy::zstring_input(R"("""abc""")"),
                      lexy::dsl::triple_quoted(cp) + lexy::dsl::eof));
    CHECK(lexy::match(lexy::zstring_input(R"('abc')"),
                      lexy::dsl::single_quoted(cp) + lexy::dsl::eof));

    CHECK(lexy::match(lexy::zstring_input("`abc`"), lexy::dsl::backticked(cp) + lexy::dsl::eof));
    CHECK(lexy::match(lexy::zstring_input("``abc``"),
                      lexy::dsl::double_backticked(cp) + lexy::dsl::eof));
    CHECK(lexy::match(lexy::zstring_input("```abc```"),
                      lexy::dsl::triple_backticked(cp) + lexy::dsl::eof));
}

TEST_CASE("dsl::delimited with escape")
{
    constexpr auto cp = lexy::dsl::ascii::character;
    constexpr auto rule
        = delimited(LEXY_LIT("("), LEXY_LIT(")"))(cp, lexy::dsl::escape(LEXY_LIT("$"))
                                                          .capture(lexy::dsl::ascii::character));
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

                constexpr void operator()(lexy::lexeme_for<test_input> lex)
                {
                    count += lex.size();
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
            assert(cur[-1] == ')');
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
            assert(e.begin() == str + 1);
            assert(e.end() == lexy::_detail::string_view(str).end());
            assert(e.message() == "missing delimiter");
            return -2;
        }
        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            assert(e.character_class() == "ASCII");
            return -3;
        }
        constexpr int error(test_error<lexy::invalid_escape_sequence> e)
        {
            assert(e.message() == "invalid escape sequence");
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

    constexpr auto unterminated = rule_matches<callback>(rule, "(abc");
    CHECK(unterminated == -2);

    constexpr auto invalid_ascii = rule_matches<callback>(rule, "(ab\xFF");
    CHECK(invalid_ascii == -3);

    constexpr auto escape = rule_matches<callback>(rule, "(a$bc$))");
    CHECK(escape == 4);
}

TEST_CASE("dsl::escape")
{
    constexpr auto escape = lexy::dsl::escape(LEXY_LIT("$"));
    SUBCASE(".rule()")
    {
        constexpr auto rule = escape.rule(LEXY_LIT("abc") >> lexy::dsl::value_c<0>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, int i)
            {
                assert(cur == str + 4);
                return i;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == str);
                assert(e.string() == "$");
                return -1;
            }
            constexpr int error(test_error<lexy::invalid_escape_sequence> e)
            {
                assert(e.position() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "$abc");
        CHECK(abc == 0);

        constexpr auto invalid = rule_matches<callback>(rule, "$ab");
        CHECK(invalid == -2);
    }
    SUBCASE("multiple rules")
    {
        constexpr auto rule = escape.rule(LEXY_LIT("a") >> lexy::dsl::value_c<1>)
                                  .rule(LEXY_LIT("b") >> lexy::dsl::value_c<2>)
                                  .rule(lexy::dsl::else_ >> lexy::dsl::value_c<0>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char*, int i)
            {
                return i;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == str);
                assert(e.string() == "$");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "$a");
        CHECK(a == 1);
        constexpr auto b = rule_matches<callback>(rule, "$b");
        CHECK(b == 2);

        constexpr auto invalid = rule_matches<callback>(rule, "$c");
        CHECK(invalid == 0);
    }
    SUBCASE(".capture()")
    {
        constexpr auto rule = escape.capture(lexy::dsl::ascii::character);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                assert(cur == str + 2);
                return *lex.begin();
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == str);
                assert(e.string() == "$");
                return -1;
            }
            constexpr int error(test_error<lexy::invalid_escape_sequence> e)
            {
                assert(e.position() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "$a");
        CHECK(a == 'a');
        constexpr auto b = rule_matches<callback>(rule, "$b");
        CHECK(b == 'b');

        constexpr auto invalid = rule_matches<callback>(rule, "$\xFF");
        CHECK(invalid == -2);
    }
#if LEXY_HAS_NTTP
    SUBCASE(".lit_c()")
    {
        constexpr auto rule = escape.lit<"a">();
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, char c)
            {
                assert(cur == str + 2);
                return c;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == str);
                assert(e.string() == "$");
                return -1;
            }
            constexpr int error(test_error<lexy::invalid_escape_sequence> e)
            {
                assert(e.position() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "$a");
        CHECK(a == 'a');

        constexpr auto invalid = rule_matches<callback>(rule, "$b");
        CHECK(invalid == -2);
    }
#endif
    SUBCASE(".lit_c()")
    {
        constexpr auto rule = escape.lit_c<'a'>();
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, char c)
            {
                assert(cur == str + 2);
                return c;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.position() == str);
                assert(e.string() == "$");
                return -1;
            }
            constexpr int error(test_error<lexy::invalid_escape_sequence> e)
            {
                assert(e.position() == str + 1);
                return -2;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto a = rule_matches<callback>(rule, "$a");
        CHECK(a == 'a');

        constexpr auto invalid = rule_matches<callback>(rule, "$b");
        CHECK(invalid == -2);
    }
}

TEST_CASE("predefined escapes")
{
    CHECK(lexy::match(lexy::zstring_input("\\"), lexy::dsl::backslash_escape));
    CHECK(lexy::match(lexy::zstring_input("$"), lexy::dsl::dollar_escape));
}

