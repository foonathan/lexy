// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/eof.hpp>

#include <doctest.h>
#include <lexy/dsl/literal.hpp>

TEST_CASE("eof whitespace")
{
    constexpr auto result     = lexy::dsl::eof[LEXY_LIT(" ")];
    constexpr auto equivalent = LEXY_LIT(" ") + lexy::dsl::eof;
    CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
}

