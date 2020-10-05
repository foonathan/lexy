// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/branch.hpp>

#include <doctest.h>
#include <lexy/dsl/literal.hpp>

TEST_CASE("dsl::branch()")
{
    SUBCASE("pattern")
    {
        CHECK(lexy::is_branch_rule<decltype(LEXY_LIT("abc"))>);
        constexpr auto result     = branch(LEXY_LIT("abc"));
        constexpr auto equivalent = LEXY_LIT("abc") >> lexy::dsl::success;
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("branch")
    {
        CHECK(lexy::is_branch_rule<decltype(LEXY_LIT("abc") >> LEXY_LIT("def"))>);
        constexpr auto result     = branch(LEXY_LIT("abc") >> LEXY_LIT("def"));
        constexpr auto equivalent = LEXY_LIT("abc") >> LEXY_LIT("def");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
}

TEST_CASE("dsl::_br operator+")
{
    constexpr auto branch = LEXY_LIT("condition") >> LEXY_LIT("then");
    SUBCASE("left")
    {
        constexpr auto result     = LEXY_LIT("prefix") + branch;
        constexpr auto equivalent = LEXY_LIT("prefix") + LEXY_LIT("condition") + LEXY_LIT("then");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("right")
    {
        constexpr auto result     = branch + LEXY_LIT("suffix");
        constexpr auto equivalent = LEXY_LIT("condition") >> LEXY_LIT("then") + LEXY_LIT("suffix");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
    SUBCASE("both")
    {
        constexpr auto result = branch + branch;
        constexpr auto equivalent
            = LEXY_LIT("condition") >> LEXY_LIT("then") + LEXY_LIT("condition") + LEXY_LIT("then");
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
}

TEST_CASE("dsl::_br operator>>")
{
    constexpr auto branch = LEXY_LIT("condition") >> LEXY_LIT("then");

    constexpr auto result     = LEXY_LIT("prefix") >> branch;
    constexpr auto equivalent = LEXY_LIT("prefix") + LEXY_LIT("condition") >> LEXY_LIT("then");
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
}

