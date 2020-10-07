// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/condition.hpp>

#include "verify.hpp"
#include <lexy/dsl/branch.hpp>

TEST_CASE("pattern: if_")
{
    constexpr auto pattern = if_(LEXY_LIT("abc")) >> lexy::dsl::success;
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(!empty);

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(abc);
    CHECK(abc.match().empty());
}

TEST_CASE("pattern: unless")
{
    constexpr auto pattern = unless(LEXY_LIT("abc")) >> lexy::dsl::success;
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(empty);
    CHECK(empty.match().empty());

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(!abc);
}

TEST_CASE("pattern: not")
{
    constexpr auto pattern = !LEXY_LIT("abc") >> lexy::dsl::success;
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(empty);
    CHECK(empty.match().empty());

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(!abc);
    CHECK(abc.match().string_view() == "abc");
}

