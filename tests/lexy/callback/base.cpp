// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/base.hpp>

#include <doctest/doctest.h>
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

        CHECK(lexy::is_callback<decltype(callback)>);
        CHECK(std::is_same_v<typename decltype(callback)::return_type, int>);

        CHECK(lexy::is_callback_for<decltype(callback), int>);
        CHECK(callback(11) == 22);

        CHECK(lexy::is_callback_for<decltype(callback), const char*>);
        CHECK(callback("abc") == 'a');

        CHECK(lexy::is_callback_for<decltype(callback), std::nullptr_t>);
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
    SUBCASE("member ptr")
    {
        struct foo
        {
            int member;

            int fn(int i) const
            {
                return i;
            }
        };
        foo obj{42};

        constexpr auto callback = lexy::callback<int>(&foo::fn, &foo::member);
        CHECK(callback(foo(), 4) == 4);
        CHECK(callback(&obj) == 42);
    }
    SUBCASE("with state")
    {
        constexpr auto callback = lexy::callback<int>([i = 42](int arg) { return arg + i; });
        CHECK(callback(0) == 42);
        CHECK(callback(11) == 53);
    }
}

TEST_CASE("sink")
{
    constexpr auto sink = lexy::sink<int>([](int& result, int i) { result += i; },
                                          [](int& result, const char* ptr) { result += *ptr; });
    CHECK(lexy::is_sink<decltype(sink)>);

    auto cb = sink.sink();
    cb(4);
    cb("abc");

    int result = LEXY_MOV(cb).finish();
    CHECK(result == 4 + 'a');
}

TEST_CASE("callback compose")
{
    SUBCASE("callbacks")
    {
        constexpr auto a = lexy::callback<int>([](int i) { return 2 * i; });
        constexpr auto b
            = lexy::callback<std::string>([](int i) { return std::string(std::size_t(i), 'a'); });
        constexpr auto c = lexy::callback<std::size_t>(&std::string::length);

        constexpr auto composed = a | b | c;
        CHECK(composed(0) == 0);
        CHECK(composed(8) == 16);
    }
    SUBCASE("sink and callback")
    {
        constexpr auto sink = lexy::sink<int>([](int& result, int i) { result += i; });
        constexpr auto cb   = lexy::callback<std::string>([](int i) { return std::to_string(i); });

        constexpr auto composed = sink >> cb;

        auto s = sink.sink();
        s(1);
        s(2);
        s(3);
        auto result = composed(LEXY_MOV(s).finish());
        CHECK(result == "6");
    }
    SUBCASE("sink and two callback")
    {
        constexpr auto sink = lexy::sink<int>([](int& result, int i) { result += i; });
        constexpr auto cb_a = lexy::callback<std::string>([](int i) { return std::to_string(i); });
        constexpr auto cb_b = lexy::callback<std::size_t>(&std::string::length);

        constexpr auto composed = sink >> cb_a | cb_b;

        auto s = sink.sink();
        s(1);
        s(2);
        s(3);
        auto result = composed(LEXY_MOV(s).finish());
        CHECK(result == 1);
    }
}

