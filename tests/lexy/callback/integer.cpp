// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/integer.hpp>

#include <doctest/doctest.h>

TEST_CASE("as_integer")
{
    int no_sign = lexy::as_integer<int>(42);
    CHECK(no_sign == 42);

    int sign = lexy::as_integer<int>(-1, 42);
    CHECK(sign == -42);
}

