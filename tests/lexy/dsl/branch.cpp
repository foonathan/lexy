// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/branch.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::operator>>")
{
    constexpr auto rule = LEXY_LIT("a") >> lexy::dsl::label<struct lab>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::label<lab>)
        {
            CONSTEXPR_CHECK(str + 1 == cur);
            return 0;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.string() == "a");
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto success = rule_matches<callback>(rule, "a");
    CHECK(success == 0);
}

TEST_CASE("dsl::_br operator+")
{
    constexpr auto branch = LEXY_LIT("condition") >> LEXY_LIT("then");

    SUBCASE("rule + branch")
    {
        constexpr auto result     = LEXY_LIT("prefix") + branch;
        constexpr auto equivalent = LEXY_LIT("prefix") + LEXY_LIT("condition") + LEXY_LIT("then");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("sequence + branch")
    {
        constexpr auto result = LEXY_LIT("a") + LEXY_LIT("b") + branch;
        constexpr auto equivalent
            = LEXY_LIT("a") + LEXY_LIT("b") + LEXY_LIT("condition") + LEXY_LIT("then");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }

    SUBCASE("branch + rule")
    {
        constexpr auto result     = branch + LEXY_LIT("suffix");
        constexpr auto equivalent = LEXY_LIT("condition") >> LEXY_LIT("then") + LEXY_LIT("suffix");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("branch + sequence")
    {
        constexpr auto result = branch + LEXY_LIT("a") + LEXY_LIT("b");
        constexpr auto equivalent
            = LEXY_LIT("condition") >> LEXY_LIT("then") + LEXY_LIT("a") + LEXY_LIT("b");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }

    SUBCASE("branch + branch")
    {
        constexpr auto result = branch + branch;
        constexpr auto equivalent
            = LEXY_LIT("condition") >> LEXY_LIT("then") + LEXY_LIT("condition") + LEXY_LIT("then");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
}

