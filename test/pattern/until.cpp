// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/until.hpp>

#include "verify.hpp"

TEST_CASE("pattern: until")
{
    constexpr auto pattern = until(LEXY_LIT("!"));
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(!empty);
    CHECK(empty.match().empty());

    constexpr auto zero = pattern_matches(pattern, "!");
    CHECK(zero);
    CHECK(zero.match().string_view() == "!");
    constexpr auto one = pattern_matches(pattern, "a!");
    CHECK(one);
    CHECK(one.match().string_view() == "a!");
    constexpr auto two = pattern_matches(pattern, "xy!");
    CHECK(two);
    CHECK(two.match().string_view() == "xy!");

    constexpr auto unterminated = pattern_matches(pattern, "abc");
    CHECK(!unterminated);
    CHECK(unterminated.match().empty());
}

