// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/base.hpp>

#include "verify.hpp"

TEST_CASE("pattern: atom")
{
    constexpr auto pattern = LEXY_LIT("abc");
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(!empty);

    constexpr auto abc = pattern_matches(pattern, "abc");
    CHECK(abc);
    CHECK(abc.match().string_view() == "abc");
}

