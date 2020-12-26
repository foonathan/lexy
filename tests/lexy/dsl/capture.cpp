// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/capture.hpp>

#include "verify.hpp"
#include <lexy/callback.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("dsl::capture()")
{
    SUBCASE("basic")
    {
        static constexpr auto rule = capture(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                LEXY_VERIFY_CHECK(lex.begin() == str);
                LEXY_VERIFY_CHECK(lex.end() == cur);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 0);
    }
    SUBCASE("capture label")
    {
        static constexpr auto rule = capture(lexy::dsl::label<struct lab>);
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> lex,
                                       lexy::label<lab>)
            {
                LEXY_VERIFY_CHECK(str == cur);
                LEXY_VERIFY_CHECK(lex.empty());
                return 0;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto string = LEXY_VERIFY("abc");
        CHECK(string == 0);
    }
    SUBCASE("directly nested")
    {
        static constexpr auto rule = capture(capture(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char*, lexy::lexeme_for<test_input> outer,
                                       lexy::lexeme_for<test_input> inner)
            {
                LEXY_VERIFY_CHECK(lexy::as_string<lexy::_detail::string_view>(outer) == "abc");
                LEXY_VERIFY_CHECK(lexy::as_string<lexy::_detail::string_view>(inner) == "abc");
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 0);
    }
    SUBCASE("indirectly nested")
    {
        static constexpr auto rule
            = capture(LEXY_LIT("(") + capture(LEXY_LIT("abc")) + LEXY_LIT(")"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char*, lexy::lexeme_for<test_input> outer,
                                       lexy::lexeme_for<test_input> inner)
            {
                LEXY_VERIFY_CHECK(lexy::as_string<lexy::_detail::string_view>(outer) == "(abc)");
                LEXY_VERIFY_CHECK(lexy::as_string<lexy::_detail::string_view>(inner) == "abc");
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
            {
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("(abc)");
        CHECK(success == 0);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule = if_(capture(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str);
                return 0;
            }
            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                LEXY_VERIFY_CHECK(lex.begin() == str);
                LEXY_VERIFY_CHECK(lex.end() == cur);
                return 1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 1);
    }
    SUBCASE("whitespace")
    {
        static constexpr auto rule = capture(LEXY_LIT("abc"))[LEXY_LIT(" ")];
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                LEXY_VERIFY_CHECK(lex.end() == cur);
                LEXY_VERIFY_CHECK(lexy::_detail::string_view(lex.data(), lex.size()) == "abc");
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.string() == "abc");
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("abc");
        CHECK(success == 0);

        auto with_space = LEXY_VERIFY("  abc");
        CHECK(with_space == 0);
    }
}

