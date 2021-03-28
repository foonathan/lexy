// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/noop.hpp>

#include <doctest/doctest.h>

TEST_CASE("noop")
{
    SUBCASE("callback")
    {
        lexy::noop();
        lexy::noop(1, 2, 3);
    }
    SUBCASE("sink")
    {
        auto sink = lexy::noop.sink();
        sink(1, 2, 3);
        sink(1, 2, 3);
        LEXY_MOV(sink).finish();
    }
}

