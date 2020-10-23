// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback.hpp>

#include <doctest.h>
#include <lexy/input/string_input.hpp>
#include <memory>
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

TEST_CASE("forward")
{
    auto cb = lexy::construct<int>;
    CHECK(cb(0) == 0);
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

TEST_CASE("new_")
{
    SUBCASE("single")
    {
        auto cb = lexy::new_<int, std::unique_ptr<int>>;
        CHECK(*cb(0) == 0);
    }
    SUBCASE("parens")
    {
        struct type
        {
            int   a;
            float b;

            type(int a, float b) : a(a), b(b) {}
        };

        auto                  cb     = lexy::new_<type, std::unique_ptr<type>>;
        std::unique_ptr<type> result = cb(11, 3.14f);
        CHECK(result->a == 11);
        CHECK(result->b == 3.14f);
    }
    SUBCASE("braces")
    {
        struct type
        {
            int   a;
            float b;
        };

        auto                  cb     = lexy::new_<type, std::unique_ptr<type>>;
        std::unique_ptr<type> result = cb(11, 3.14f);
        CHECK(result->a == 11);
        CHECK(result->b == 3.14f);
    }
}

TEST_CASE("as_list")
{
    SUBCASE("callback")
    {
        std::vector<int> vec = lexy::as_list<std::vector<int>>(1, 2, 3);
        CHECK(vec == std::vector{1, 2, 3});
    }
    SUBCASE("sink")
    {
        auto sink = lexy::as_list<std::vector<std::string>>.sink();
        sink("a");
        sink(std::string("b"));
        sink(1, 'c');
        std::vector<std::string> result = LEXY_MOV(sink).finish();
        CHECK(result == std::vector<std::string>{"a", "b", "c"});
    }
}

TEST_CASE("as_collection")
{
    SUBCASE("callback")
    {
        std::set<int> s = lexy::as_collection<std::set<int>>(1, 2, 3);
        CHECK(s == std::set{1, 2, 3});
    }
    SUBCASE("sink")
    {
        auto sink = lexy::as_collection<std::set<std::string>>.sink();
        sink("a");
        sink(std::string("b"));
        sink(1, 'c');
        std::set<std::string> result = LEXY_MOV(sink).finish();
        CHECK(result == std::set<std::string>{"a", "b", "c"});
    }
}

TEST_CASE("as_string")
{
    lexy::string_input input  = lexy::zstring_input("abc");
    auto               reader = input.reader();

    auto begin = reader.cur();
    reader.bump();
    reader.bump();
    reader.bump();

    const lexy::lexeme lexeme(reader, begin);

    SUBCASE("callback")
    {
        std::string from_ptr_size = lexy::as_string<std::string>("abc", 2);
        CHECK(from_ptr_size == "ab");

        std::string from_lexeme = lexy::as_string<std::string>(lexeme);
        CHECK(from_lexeme == "abc");

        std::string from_lvalue = lexy::as_string<std::string>(from_lexeme);
        CHECK(from_lvalue == "abc");
        std::string from_rvalue = lexy::as_string<std::string>(std::string("test"));
        CHECK(from_rvalue == "test");
    }
    SUBCASE("sink")
    {
        auto sink = lexy::as_string<std::string>.sink();
        sink('a');
        sink("bcd", 2);
        sink(lexeme);
        sink(std::string("hi"));

        std::string result = LEXY_MOV(sink).finish();
        CHECK(result == "abcabchi");
    }
}

TEST_CASE("as_integer")
{
    int no_sign = lexy::as_integer<int>(42);
    CHECK(no_sign == 42);

    int sign = lexy::as_integer<int>(-1, 42);
    CHECK(sign == -42);
}

