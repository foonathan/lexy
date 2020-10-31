// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/brackets.hpp>

#include <doctest.h>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/match.hpp>

TEST_CASE("dsl::bracketed")
{
    // We want to make sure that inner does not have a matcher.
    constexpr auto inner = LEXY_LIT("abc") + lexy::dsl::id<0>;

    SUBCASE("round_brackets")
    {
        constexpr auto result     = lexy::dsl::round_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
    }
    SUBCASE("square_brackets")
    {
        constexpr auto result     = lexy::dsl::square_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("[") >> inner + LEXY_LIT("]");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("[abc]"), result + lexy::dsl::eof));
    }
    SUBCASE("curly_brackets")
    {
        constexpr auto result     = lexy::dsl::curly_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("{") >> inner + LEXY_LIT("}");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("{abc}"), result + lexy::dsl::eof));
    }
    SUBCASE("angle_brackets")
    {
        constexpr auto result     = lexy::dsl::angle_bracketed(inner);
        constexpr auto equivalent = LEXY_LIT("<") >> inner + LEXY_LIT(">");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("<abc>"), result + lexy::dsl::eof));
    }
    SUBCASE("parentheses")
    {
        constexpr auto result     = lexy::dsl::parenthesized(inner);
        constexpr auto equivalent = LEXY_LIT("(") >> inner + LEXY_LIT(")");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
    }

    SUBCASE("whitespace")
    {
        constexpr auto result     = lexy::dsl::parenthesized[LEXY_LIT(" ")](inner);
        constexpr auto equivalent = whitespaced(LEXY_LIT("("), LEXY_LIT(" "))
                                    >> inner + whitespaced(LEXY_LIT(")"), LEXY_LIT(" "));
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input(" (abc )"), result + lexy::dsl::eof));
    }

    SUBCASE("opt")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")");

        constexpr auto result     = brackets(o, c).opt(inner);
        constexpr auto equivalent = o >> opt(!c >> inner + c);
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
    }
    SUBCASE("opt - branch")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")") >> lexy::dsl::id<0>;

        constexpr auto result = brackets(o, c).opt(inner);
        constexpr auto equivalent
            = o >> (!c.condition() >> inner + c | lexy::dsl::else_ >> c.then());
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

        CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
        CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
    }
    SUBCASE("list")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")");

        SUBCASE("no sep")
        {
            constexpr auto result     = brackets(o, c).list(inner);
            constexpr auto equivalent = o >> list(!c >> inner);
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(!lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abcabc)"), result + lexy::dsl::eof));
        }
        SUBCASE("sep")
        {
            constexpr auto result     = brackets(o, c).list(inner, sep(LEXY_LIT(",")));
            constexpr auto equivalent = o >> list(peek(!c) >> inner, sep(LEXY_LIT(","))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(!lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(!lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
        SUBCASE("trailing sep")
        {
            constexpr auto result = brackets(o, c).list(inner, trailing_sep(LEXY_LIT(",")));
            constexpr auto equivalent
                = o >> list(peek(!c) >> inner, trailing_sep(LEXY_LIT(","))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(!lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
    }
    SUBCASE("list - branch")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")") >> lexy::dsl::id<0>;

        SUBCASE("no sep")
        {
            constexpr auto result     = brackets(o, c).list(inner);
            constexpr auto equivalent = o >> list(!c.condition() >> inner) + c.then();
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(!lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abcabc)"), result + lexy::dsl::eof));
        }
        SUBCASE("sep")
        {
            constexpr auto result = brackets(o, c).list(inner, sep(LEXY_LIT(",")));
            constexpr auto equivalent
                = o >> list(peek(!c.condition()) >> inner, sep(LEXY_LIT(","))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(!lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(!lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
        SUBCASE("trailing sep")
        {
            constexpr auto result = brackets(o, c).list(inner, trailing_sep(LEXY_LIT(",")));
            constexpr auto equivalent
                = o >> list(peek(!c.condition()) >> inner, trailing_sep(LEXY_LIT(","))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(!lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
    }
    SUBCASE("opt_list")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")");

        SUBCASE("no sep")
        {
            constexpr auto result     = brackets(o, c).opt_list(inner);
            constexpr auto equivalent = o >> opt(list(!c >> inner));
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abcabc)"), result + lexy::dsl::eof));
        }
        SUBCASE("sep")
        {
            constexpr auto result     = brackets(o, c).opt_list(inner, sep(LEXY_LIT(",")));
            constexpr auto equivalent = o >> opt(list(peek(!c) >> inner, sep(LEXY_LIT(",")))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(!lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
        SUBCASE("trailing sep")
        {
            constexpr auto result = brackets(o, c).opt_list(inner, trailing_sep(LEXY_LIT(",")));
            constexpr auto equivalent
                = o >> opt(list(peek(!c) >> inner, trailing_sep(LEXY_LIT(",")))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
    }
    SUBCASE("opt_list - branch")
    {
        constexpr auto o = LEXY_LIT("(");
        constexpr auto c = LEXY_LIT(")") >> lexy::dsl::id<0>;

        SUBCASE("no sep")
        {
            constexpr auto result     = brackets(o, c).opt_list(inner);
            constexpr auto equivalent = o >> opt(list(!c.condition() >> inner)) + c.then();
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abcabc)"), result + lexy::dsl::eof));
        }
        SUBCASE("sep")
        {
            constexpr auto result = brackets(o, c).opt_list(inner, sep(LEXY_LIT(",")));
            constexpr auto equivalent
                = o >> opt(list(peek(!c.condition()) >> inner, sep(LEXY_LIT(",")))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(!lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
        SUBCASE("trailing sep")
        {
            constexpr auto result = brackets(o, c).opt_list(inner, trailing_sep(LEXY_LIT(",")));
            constexpr auto equivalent
                = o >> opt(list(peek(!c.condition()) >> inner, trailing_sep(LEXY_LIT(",")))) + c;
            CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);

            CHECK(lexy::match(lexy::zstring_input("()"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc)"), result + lexy::dsl::eof));
            CHECK(lexy::match(lexy::zstring_input("(abc,abc,)"), result + lexy::dsl::eof));
        }
    }
}

