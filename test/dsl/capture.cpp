// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/capture.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/sequence.hpp>
#include <string>

TEST_CASE("dsl::capture()")
{
    SUBCASE("basic")
    {
        constexpr auto rule = capture(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                assert(lex.begin() == str);
                assert(lex.end() == cur);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 0);
    }
    SUBCASE("capture label")
    {
        constexpr auto rule = capture(lexy::dsl::label<struct lab>);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme_for<test_input> lex,
                                  lexy::label<lab>)
            {
                assert(str == cur);
                assert(lex.empty());
                return 0;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 0);
    }
    SUBCASE("directly nested")
    {
        constexpr auto rule = capture(capture(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char*, lexy::lexeme_for<test_input> outer,
                                  lexy::lexeme_for<test_input> inner)
            {
                assert(lexy::_detail::string_view(outer) == "abc");
                assert(lexy::_detail::string_view(inner) == "abc");
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 0);
    }
    SUBCASE("indirectly nested")
    {
        constexpr auto rule = capture(LEXY_LIT("(") + capture(LEXY_LIT("abc")) + LEXY_LIT(")"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char*, lexy::lexeme_for<test_input> outer,
                                  lexy::lexeme_for<test_input> inner)
            {
                assert(lexy::_detail::string_view(outer) == "(abc)");
                assert(lexy::_detail::string_view(inner) == "abc");
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal>)
            {
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto success = rule_matches<callback>(rule, "(abc)");
        CHECK(success == 0);
    }
    SUBCASE("as string")
    {
        constexpr auto rule = lexy::dsl::capture<std::string>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            int success(const char*, std::string lex)
            {
                assert(lex == "abc");
                return 0;
            }

            int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        const auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        const auto success = rule_matches<callback>(rule, "abc");
        CHECK(success == 0);
    }
}

