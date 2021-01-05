// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

// For the tests that explicitly use the deprecated members below.
#define LEXY_IGNORE_DEPRECATED_LIST

#include <lexy/production.hpp>

#include <doctest/doctest.h>
#include <lexy/callback.hpp>
#include <lexy/dsl/any.hpp>

namespace
{
struct prod
{
    static constexpr auto rule  = lexy::dsl::any;
    static constexpr auto value = lexy::noop;
};
} // namespace

TEST_CASE("production traits")
{
    CHECK(lexy::production_name<prod>() == "prod");

    CHECK(std::is_same_v<const lexy::production_rule<prod>, decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const lexy::production_value<prod>::type, decltype(lexy::noop)>);
}

namespace
{
struct prod_list
{
    static constexpr auto list = lexy::noop;
};
struct prod_list_value
{
    static constexpr auto list  = lexy::noop;
    static constexpr auto value = lexy::noop;
};
} // namespace

TEST_CASE("production traits deprecated")
{
    SUBCASE("list")
    {
        using traits = lexy::production_value<prod_list>;
        CHECK(std::is_same_v<const traits::type, decltype(lexy::noop)>);
    }
    SUBCASE("list + value")
    {
        using traits = lexy::production_value<prod_list_value>;
        CHECK(std::is_same_v<traits::type, decltype(lexy::noop >> lexy::noop)>);
    }
}

