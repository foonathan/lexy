// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/alternative.hpp>

#include "verify.hpp"
#include <lexy/dsl/label.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("pattern: alternative")
{
    constexpr auto pattern = LEXY_LIT("abc") / LEXY_LIT("a") / LEXY_LIT("ab") / LEXY_LIT("def");
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(!empty);

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(abc);
    CHECK(abc.match().string_view() == "abc");

    constexpr auto a = pattern_matches(pattern, "a");
    CHECK(a);
    CHECK(a.match().string_view() == "a");

    constexpr auto ab = pattern_matches(pattern, "ab");
    CHECK(ab);
    CHECK(ab.match().string_view() == "a");

    constexpr auto def = pattern_matches(pattern, "def");
    CHECK(def);
    CHECK(def.match().string_view() == "def");
}

