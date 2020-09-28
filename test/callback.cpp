// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback.hpp>

#include <doctest.h>
#include <set>
#include <string>
#include <vector>

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
        constexpr auto callback
            = lexy::callback<int>([](int i) { return 2 * i; }, [](const char* ptr) { return *ptr; },
                                  &test_fn);

        CHECK(std::is_same_v<typename decltype(callback)::return_type, int>);

        CHECK(callback(11) == 22);
        CHECK(callback("abc") == 'a');
        CHECK(callback(nullptr) == 0);
    }
    SUBCASE("match all case")
    {
        constexpr auto callback
            = lexy::callback<int>([](const auto&... args) { return sizeof...(args); });
        CHECK(callback() == 0);
        CHECK(callback(1) == 1);
        CHECK(callback(1, 2, 3) == 3);
    }
}

TEST_CASE("sink")
{
    constexpr auto sink = lexy::sink<int>([](int& result, int i) { result += i; },
                                          [](int& result, const char* ptr) { result += *ptr; });
    auto           cb   = sink.sink();
    cb(4);
    cb("abc");

    int result = LEXY_MOV(cb).finish();
    CHECK(result == 4 + 'a');
}

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

TEST_CASE("list")
{
    SUBCASE("callback")
    {
        std::vector<int> vec = lexy::list<std::vector<int>>(1, 2, 3);
        CHECK(vec == std::vector{1, 2, 3});
    }
    SUBCASE("sink")
    {
        auto sink = lexy::list<std::vector<std::string>>.sink();
        sink("a");
        sink(std::string("b"));
        sink(1, 'c');
        std::vector<std::string> result = LEXY_MOV(sink).finish();
        CHECK(result == std::vector<std::string>{"a", "b", "c"});
    }
}

TEST_CASE("collection")
{
    SUBCASE("callback")
    {
        std::set<int> s = lexy::collection<std::set<int>>(1, 2, 3);
        CHECK(s == std::set{1, 2, 3});
    }
    SUBCASE("sink")
    {
        auto sink = lexy::collection<std::set<std::string>>.sink();
        sink("a");
        sink(std::string("b"));
        sink(1, 'c');
        std::set<std::string> result = LEXY_MOV(sink).finish();
        CHECK(result == std::set<std::string>{"a", "b", "c"});
    }
}

