// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/pattern/base.hpp>

#include "verify.hpp"

TEST_CASE("pattern_match")
{
    constexpr auto result
        = lexy::pattern_match(lexy::zstring_input("abc"), [] { return LEXY_LIT("abc"); });
    CHECK(result);
    CHECK(result.id() == 0);
    CHECK(result.match().string_view() == "abc");
}

