// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback.hpp>

#include <doctest.h>

namespace
{
int test_fn(std::nullptr_t)
{
    return 0;
}
} // namespace

TEST_CASE("callback")
{
    SUBCASE("basic")
    {
        auto callback = lexy::callback<int>([](int i) { return 2 * i; },
                                            [](const char* ptr) { return *ptr; }, &test_fn);

        CHECK(std::is_same_v<typename decltype(callback)::return_type, int>);

        CHECK(callback(11) == 22);
        CHECK(callback("abc") == 'a');
        CHECK(callback(nullptr) == 0);
    }
    SUBCASE("match all case")
    {
        auto callback = lexy::callback<int>([](const auto&... args) { return sizeof...(args); });
        CHECK(callback() == 0);
        CHECK(callback(1) == 1);
        CHECK(callback(1, 2, 3) == 3);
    }
}

TEST_CASE("noop")
{
    lexy::noop();
    lexy::noop(1, 2, 3);
}

TEST_CASE("construct")
{
    SUBCASE("single")
    {
        auto cb = lexy::construct<int>;
        CHECK(cb(0) == 0);
    }
    SUBCASE("parens")
    {
        struct type
        {
            int   a;
            float b;

            type(int a, float b) : a(a), b(b) {}
        };

        auto cb     = lexy::construct<type>;
        auto result = cb(11, 3.14f);
        CHECK(result.a == 11);
        CHECK(result.b == 3.14f);
    }
    SUBCASE("braces")
    {
        struct type
        {
            int   a;
            float b;
        };

        auto cb     = lexy::construct<type>;
        auto result = cb(11, 3.14f);
        CHECK(result.a == 11);
        CHECK(result.b == 3.14f);
    }
}
