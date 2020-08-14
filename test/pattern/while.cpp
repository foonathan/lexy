// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include "verify.hpp"

TEST_CASE("pattern: while")
{
    constexpr auto pattern = while_(LEXY_LIT("ab"));
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(empty);
    CHECK(empty.match().empty());

    constexpr auto a = pattern_matches(pattern, "a");
    CHECK(a);
    CHECK(a.match().empty());

    constexpr auto ab = pattern_matches(pattern, "ab");
    CHECK(ab);
    CHECK(ab.match().string_view() == "ab");

    constexpr auto abab = pattern_matches(pattern, "abab");
    CHECK(abab);
    CHECK(abab.match().string_view() == "abab");

    constexpr auto ababab = pattern_matches(pattern, "ababab");
    CHECK(ababab);
    CHECK(ababab.match().string_view() == "ababab");
}

