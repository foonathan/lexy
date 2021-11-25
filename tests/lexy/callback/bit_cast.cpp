// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/bit_cast.hpp>

#include <doctest/doctest.h>

TEST_CASE("bit_cast")
{
    constexpr auto callback = lexy::bit_cast<unsigned>;
    CHECK(lexy::is_callback<decltype(callback)>);
    CHECK(std::is_same_v<decltype(callback)::return_type, unsigned>);

    CHECK(callback(0u) == 0u);
    CHECK(callback(42u) == 42u);

    CHECK(callback(0) == 0);
    CHECK(callback(42) == 42);
}

