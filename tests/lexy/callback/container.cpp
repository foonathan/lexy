// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/container.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/base.hpp>
#include <set>
#include <string>
#include <vector>

TEST_CASE("as_list")
{
    auto sink = lexy::as_list<std::vector<std::string>>.sink();
    sink("a");
    sink(std::string("b"));
    sink(1, 'c');
    std::vector<std::string> result = LEXY_MOV(sink).finish();
    CHECK(result == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("as_collection")
{
    auto sink = lexy::as_collection<std::set<std::string>>.sink();
    sink("a");
    sink(std::string("b"));
    sink(1, 'c');
    std::set<std::string> result = LEXY_MOV(sink).finish();
    CHECK(result == std::set<std::string>{"a", "b", "c"});
}

TEST_CASE("collect")
{
    SUBCASE("void")
    {
        auto sum      = 0;
        auto callback = lexy::callback([&sum](int i) mutable { sum += i; });

        auto collect = lexy::collect(callback);
        CHECK(lexy::is_sink<decltype(collect)>);

        auto cb = collect.sink();
        cb(1);
        cb(2);
        cb(3);

        std::size_t count = LEXY_MOV(cb).finish();
        CHECK(count == 3);
        CHECK(sum == 6);
    }
    SUBCASE("non-void")
    {
        constexpr auto callback = lexy::callback<int>([](int i) { return 2 * i; });

        constexpr auto collect = lexy::collect<std::vector<int>>(callback);
        CHECK(lexy::is_sink<decltype(collect)>);

        auto cb = collect.sink();
        cb(1);
        cb(2);
        cb(3);

        std::vector<int> result = LEXY_MOV(cb).finish();
        REQUIRE(result.size() == 3);
        CHECK(result[0] == 2);
        CHECK(result[1] == 4);
        CHECK(result[2] == 6);
    }
}

