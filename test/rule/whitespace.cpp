// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/whitespace.hpp>

#include "verify.hpp"

TEST_CASE("rule: whitespace")
{
    SUBCASE("basic")
    {
        constexpr auto rule = whitespaced(LEXY_LIT("abc"), LEXY_LIT(" "));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                return int(cur - str) - 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);

        constexpr auto space_abc = rule_matches<callback>(rule, " abc");
        CHECK(space_abc == 1);
        constexpr auto space_space_abc = rule_matches<callback>(rule, "  abc");
        CHECK(space_space_abc == 2);
    }
    SUBCASE("capture")
    {
        constexpr auto rule = capture(whitespaced(LEXY_LIT("abc"), LEXY_LIT(" ")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::lexeme_for<test_input> lex)
            {
                assert(lex.string_view() == "abc");
                return int(cur - str) - 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);

        constexpr auto space_abc = rule_matches<callback>(rule, " abc");
        CHECK(space_abc == 1);
        constexpr auto space_space_abc = rule_matches<callback>(rule, "  abc");
        CHECK(space_space_abc == 2);
    }
    SUBCASE("capture<string_view>")
    {
        constexpr auto rule = lexy::dsl::capture<lexy::_detail::string_view>(
            whitespaced(LEXY_LIT("abc"), LEXY_LIT(" ")));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::_detail::string_view lex)
            {
                assert(lex == "abc");
                return int(cur - str) - 3;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                assert(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);

        constexpr auto space_abc = rule_matches<callback>(rule, " abc");
        CHECK(space_abc == 1);
        constexpr auto space_space_abc = rule_matches<callback>(rule, "  abc");
        CHECK(space_space_abc == 2);
    }
}

