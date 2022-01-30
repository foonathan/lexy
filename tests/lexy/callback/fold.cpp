// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/fold.hpp>

#include <doctest/doctest.h>

TEST_CASE("fold")
{
    constexpr auto cb = lexy::fold<int>({}, [](int lhs, auto rhs) { return lhs + int(rhs); });

    auto sink_cb = cb.sink();
    sink_cb(1);
    sink_cb(2);
    sink_cb(3.14f);
    CHECK(LEXY_MOV(sink_cb).finish() == 6);
}

TEST_CASE("fold_inplace")
{
    constexpr auto cb = lexy::fold_inplace<int>({}, [](int& lhs, auto rhs) { lhs += int(rhs); });

    auto sink_cb = cb.sink();
    sink_cb(1);
    sink_cb(2);
    sink_cb(3.14f);
    CHECK(LEXY_MOV(sink_cb).finish() == 6);
}

TEST_CASE("count")
{
    constexpr auto cb = lexy::count;

    auto sink_cb = cb.sink();
    sink_cb(1);
    sink_cb(nullptr, "hello");
    sink_cb(3.14f, 42);
    CHECK(LEXY_MOV(sink_cb).finish() == 3);
}

