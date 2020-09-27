// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/whitespace.hpp>

#include "verify.hpp"
#include <lexy/dsl/literal.hpp>

TEST_CASE("pattern: whitespace")
{
    constexpr auto pattern = whitespaced(LEXY_LIT("abc"), LEXY_LIT(" "));
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(!empty);

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(abc);
    CHECK(abc.match().string_view() == "abc");

    constexpr auto space_abc = pattern_matches(pattern, " abc");
    CHECK(space_abc);
    CHECK(space_abc.match().string_view() == " abc");

    constexpr auto space_space_abc = pattern_matches(pattern, "  abc");
    CHECK(space_space_abc);
    CHECK(space_space_abc.match().string_view() == "  abc");
}

