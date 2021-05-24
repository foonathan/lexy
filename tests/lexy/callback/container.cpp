// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/container.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/base.hpp>
#include <set>
#include <string>
#include <vector>

namespace
{
template <typename T>
struct my_allocator : std::allocator<T>
{
    template <typename U>
    using rebind = my_allocator<U>;

    my_allocator(int i)
    {
        CHECK(i == 42);
    }
    template <typename U>
    my_allocator(my_allocator<U>)
    {}
};
} // namespace

TEST_CASE("as_list")
{
    SUBCASE("default")
    {
        constexpr auto sink = lexy::as_list<std::vector<std::string>>;
        auto           cb   = sink.sink();
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        std::vector<std::string> result = LEXY_MOV(cb).finish();
        CHECK(result == std::vector<std::string>{"a", "b", "c"});
    }
    SUBCASE("allocator")
    {
        constexpr auto sink = lexy::as_list<std::vector<std::string, my_allocator<std::string>>>;
        auto           cb   = sink.sink(42);
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({"a", "b", "c"}, 42));
    }
}

TEST_CASE("as_collection")
{
    SUBCASE("default")
    {
        constexpr auto sink = lexy::as_collection<std::set<std::string>>;
        auto           cb   = sink.sink();
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        std::set<std::string> result = LEXY_MOV(cb).finish();
        CHECK(result == std::set<std::string>{"a", "b", "c"});
    }
    SUBCASE("allocator")
    {
        constexpr auto sink
            = lexy::as_collection<std::set<std::string, std::less<>, my_allocator<std::string>>>;
        auto cb = sink.sink(42);
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({"a", "b", "c"}, 42));
    }
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
        CHECK(result == std::vector<int>{2, 4, 6});
    }
    SUBCASE("non-void with allocator")
    {
        constexpr auto callback = lexy::callback<int>([](int i) { return 2 * i; });

        constexpr auto collect = lexy::collect<std::vector<int, my_allocator<int>>>(callback);

        auto cb = collect.sink(42);
        cb(1);
        cb(2);
        cb(3);

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({2, 4, 6}, 42));
    }
}

