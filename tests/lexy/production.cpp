// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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

TEST_CASE("production_traits")
{
    using traits = lexy::production_traits<prod>;
    CHECK(traits::name() == "prod");

    CHECK(std::is_same_v<const traits::rule::type, decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const traits::value::type, decltype(lexy::noop)>);
}

