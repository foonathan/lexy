// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/forward.hpp>

#include <doctest/doctest.h>

TEST_CASE("forward")
{
    auto cb = lexy::forward<int>;
    CHECK(cb(0) == 0);
}

