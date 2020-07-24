// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/pattern/sequence.hpp>

#include "verify.hpp"

TEST_CASE("pattern: sequence")
{
    constexpr auto pattern = LEXY_LIT("abc") + LEXY_LIT("d") + LEXY_LIT("ef");
    CHECK(lexy::is_pattern<decltype(pattern)>);

    constexpr auto empty = pattern_matches(pattern, "");
    CHECK(!empty);

    constexpr auto first_partial = pattern_matches(pattern, "ab");
    CHECK(!first_partial);
    constexpr auto first = pattern_matches(pattern, "abc");
    CHECK(!first);
    constexpr auto second = pattern_matches(pattern, "abcd");
    CHECK(!second);

    constexpr auto third = pattern_matches(pattern, "abcdef");
    CHECK(third);
    CHECK(third.id() == 0);
    CHECK(third.match().string_view() == "abcdef");
}

