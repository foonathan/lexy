// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/label.hpp>

#include "verify.hpp"

TEST_CASE("pattern: id")
{
    constexpr auto pattern = lexy::dsl::id<1>;
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(empty);
    CHECK(empty.id() == 1);
    CHECK(empty.match().empty());

    constexpr auto string = pattern_matches(pattern, "abc");
    CHECK(string);
    CHECK(string.id() == 1);
    CHECK(string.match().empty());
}

