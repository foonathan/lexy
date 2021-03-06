// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/branch.hpp>

#include "verify.hpp"

TEST_CASE("dsl::operator>>")
{
    static constexpr auto rule = LEXY_LIT("a") >> label<0>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur, id<0>)
        {
            LEXY_VERIFY_CHECK(str + 1 == cur);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("a"));
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto success = LEXY_VERIFY("a");
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

