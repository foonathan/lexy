// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/grammar.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/noop.hpp>
#include <lexy/dsl/any.hpp>

namespace
{
struct prod
{
    static constexpr auto name  = "prod";
    static constexpr auto rule  = lexy::dsl::any;
    static constexpr auto value = lexy::noop;
};
} // namespace

TEST_CASE("production traits simple")
{
    CHECK(lexy::production_name<prod>() == lexy::_detail::string_view("prod"));

    CHECK(std::is_same_v<const lexy::production_rule<prod>, decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const lexy::production_value<prod>::type, decltype(lexy::noop)>);
}

namespace
{
struct prod_ws
{
    static constexpr auto whitespace = lexy::dsl::any;
};

struct prod_token : lexy::token_production
{};
} // namespace

TEST_CASE("production_whitespace")
{
    CHECK(std::is_same_v<const lexy::production_whitespace<prod_token, prod_ws>, const void>);
    CHECK(
        std::is_same_v<const lexy::production_whitespace<prod, prod_ws>, decltype(lexy::dsl::any)>);
    CHECK(
        std::is_same_v<const lexy::production_whitespace<prod_ws, prod>, decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const lexy::production_whitespace<prod, prod>, const void>);
}

namespace
{
struct prod_depth
{
    static constexpr auto max_recursion_depth = 32;
};
} // namespace

TEST_CASE("max_recursion_depth()")
{
    CHECK(lexy::max_recursion_depth<prod>() == 1024);
    CHECK(lexy::max_recursion_depth<prod_depth>() == 32);
}

