// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/fold.hpp>

#include <doctest/doctest.h>

TEST_CASE("fold")
{
    constexpr auto cb = lexy::fold<int>({}, [](int lhs, auto rhs) { return lhs + int(rhs); });

    SUBCASE("callback")
    {
        CHECK(cb() == 0);
        CHECK(cb(1) == 1);
        CHECK(cb(1, 2) == 3);
        CHECK(cb(1, 2, 3.14f) == 6);
    }
    SUBCASE("sink")
    {
        auto sink_cb = cb.sink();
        sink_cb(1);
        sink_cb(2);
        sink_cb(3.14f);
        CHECK(LEXY_MOV(sink_cb).finish() == 6);
    }
}

TEST_CASE("fold_inplace")
{
    constexpr auto cb = lexy::fold_inplace<int>({}, [](int& lhs, auto rhs) { lhs += int(rhs); });

    SUBCASE("callback")
    {
        CHECK(cb() == 0);
        CHECK(cb(1) == 1);
        CHECK(cb(1, 2) == 3);
        CHECK(cb(1, 2, 3.14f) == 6);
    }
    SUBCASE("sink")
    {
        auto sink_cb = cb.sink();
        sink_cb(1);
        sink_cb(2);
        sink_cb(3.14f);
        CHECK(LEXY_MOV(sink_cb).finish() == 6);
    }
}

TEST_CASE("count")
{
    constexpr auto cb = lexy::count;

    SUBCASE("callback")
    {
        CHECK(cb() == 0);
        CHECK(cb(1) == 1);
        CHECK(cb(1, nullptr) == 2);
        CHECK(cb(1, nullptr, 3.14f) == 3);
    }
    SUBCASE("sink")
    {
        auto sink_cb = cb.sink();
        sink_cb(1);
        sink_cb(nullptr, "hello");
        sink_cb(3.14f, 42);
        CHECK(LEXY_MOV(sink_cb).finish() == 3);
    }
}

