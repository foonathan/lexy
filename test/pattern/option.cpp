// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/option.hpp>

#include "verify.hpp"

TEST_CASE("pattern: option")
{
    constexpr auto pattern = opt(LEXY_LIT("abc"));
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(empty);
    CHECK(empty.id() == 0);
    CHECK(empty.match().empty());

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(abc);
    CHECK(abc.id() == 0);
    CHECK(abc.match().string_view() == "abc");

    constexpr auto ab = pattern_matches(pattern, "ab");
    CHECK(ab);
    CHECK(ab.id() == 0);
    CHECK(ab.match().empty());
}

