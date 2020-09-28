// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/sign.hpp>

#include "verify.hpp"
#include <lexy/dsl/eof.hpp>

TEST_CASE("rule: plus_sign")
{
    constexpr auto rule = lexy::dsl::plus_sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char*, int i)
        {
            return i;
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            assert(e.character_class() == lexy::_detail::string_view("EOF"));
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == +1);
    constexpr auto plus = rule_matches<callback>(rule, "+");
    CHECK(plus == +1);
    constexpr auto minus = rule_matches<callback>(rule, "-");
    CHECK(minus == 0);
}

TEST_CASE("rule: minus_sign")
{
    constexpr auto rule = lexy::dsl::minus_sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char*, int i)
        {
            return i;
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            assert(e.character_class() == lexy::_detail::string_view("EOF"));
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == +1);
    constexpr auto plus = rule_matches<callback>(rule, "+");
    CHECK(plus == 0);
    constexpr auto minus = rule_matches<callback>(rule, "-");
    CHECK(minus == -1);
}

TEST_CASE("rule: sign")
{
    constexpr auto rule = lexy::dsl::sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char*, int i)
        {
            return i;
        }

        constexpr int error(test_error<lexy::expected_char_class> e)
        {
            assert(e.character_class() == lexy::_detail::string_view("EOF"));
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == +1);
    constexpr auto plus = rule_matches<callback>(rule, "+");
    CHECK(plus == +1);
    constexpr auto minus = rule_matches<callback>(rule, "-");
    CHECK(minus == -1);
}

