// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback.hpp>

#include <doctest/doctest.h>
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

TEST_CASE("callback compose")
{
    constexpr auto a = lexy::callback<int>([](int i) { return 2 * i; });
    constexpr auto b
        = lexy::callback<std::string>([](int i) { return std::string(std::size_t(i), 'a'); });
    constexpr auto c
        = lexy::callback<std::size_t>([](const std::string& str) { return str.size(); });

    constexpr auto composed = a | b | c;
    CHECK(composed(0) == 0);
    CHECK(composed(8) == 16);
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

TEST_CASE("as_aggregate")
{
    struct agg
    {
        int         i;
        float       f;
        const char* str;
    };
    using member_i   = lexy::make_member_ptr<&agg::i>;
    using member_f   = lexy::make_member_ptr<&agg::f>;
    using member_str = lexy::make_member_ptr<&agg::str>;

    static constexpr auto callback = lexy::as_aggregate<agg>;
    SUBCASE("callback")
    {
        constexpr auto result = callback(member_f{}, 3.14f, member_str{}, "hello", member_i{}, 42);
        CHECK(result.i == 42);
        CHECK(result.f == 3.14f);
        CHECK(*result.str == 'h');

        constexpr auto result2 = callback(agg(result), member_f{}, 2.71f, member_i{}, 11);
        CHECK(result2.i == 11);
        CHECK(result2.f == 2.71f);
        CHECK(*result2.str == 'h');
    }
    SUBCASE("sink")
    {
        constexpr auto result = [] {
            auto sink = callback.sink();
            sink(member_i{}, 11);
            sink(member_str{}, "hello");
            sink(member_f{}, 3.14f);
            sink(member_i{}, 42);
            return LEXY_MOV(sink).finish();
        }();
        CHECK(result.i == 42);
        CHECK(result.f == 3.14f);
        CHECK(*result.str == 'h');
    }
}

TEST_CASE("as_string")
{
    auto char_lexeme = [] {
        auto input  = lexy::zstring_input("abc");
        auto reader = input.reader();

        auto begin = reader.cur();
        reader.bump();
        reader.bump();
        reader.bump();

        return lexy::lexeme(reader, begin);
    }();
    auto uchar_lexeme = [] {
        auto input  = lexy::zstring_input<lexy::raw_encoding>("abc");
        auto reader = input.reader();

        auto begin = reader.cur();
        reader.bump();
        reader.bump();
        reader.bump();

        return lexy::lexeme(reader, begin);
    }();

    SUBCASE("callback")
    {
        std::string from_ptr_size = lexy::as_string<std::string>("abc", 2);
        CHECK(from_ptr_size == "ab");

        std::string from_char_lexeme = lexy::as_string<std::string>(char_lexeme);
        CHECK(from_char_lexeme == "abc");
        std::string from_uchar_lexeme = lexy::as_string<std::string>(uchar_lexeme);
        CHECK(from_uchar_lexeme == "abc");

        std::string from_lvalue = lexy::as_string<std::string>(from_char_lexeme);
        CHECK(from_lvalue == "abc");
        std::string from_rvalue = lexy::as_string<std::string>(std::string("test"));
        CHECK(from_rvalue == "test");

        std::string from_ascii_cp
            = lexy::as_string<std::string, lexy::ascii_encoding>(lexy::code_point('a'));
        CHECK(from_ascii_cp == "a");
        std::string from_unicode_cp
            = lexy::as_string<std::string, lexy::utf8_encoding>(lexy::code_point(0x00E4));
        CHECK(from_unicode_cp == "\u00E4");
    }
    SUBCASE("sink")
    {
        auto sink = lexy::as_string<std::string, lexy::utf8_encoding>.sink();
        sink('a');
        sink("bcd", 2);
        sink(char_lexeme);
        sink(uchar_lexeme);
        sink(std::string("hi"));
        sink(lexy::code_point('a'));
        sink(lexy::code_point(0x00E4));

        std::string result = LEXY_MOV(sink).finish();
        CHECK(result == "abcabcabchia\u00E4");
    }
}

TEST_CASE("as_integer")
{
    int no_sign = lexy::as_integer<int>(42);
    CHECK(no_sign == 42);

    int sign = lexy::as_integer<int>(-1, 42);
    CHECK(sign == -42);
}

