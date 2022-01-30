// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/forward.hpp>

#include <doctest/doctest.h>

TEST_CASE("forward")
{
    auto cb = lexy::forward<int>;
    CHECK(cb(0) == 0);

    auto cb_void = lexy::forward<void>;
    cb_void();
}

