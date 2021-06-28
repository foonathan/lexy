// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/composition.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/fold.hpp>
#include <string>
#include <vector>

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
        constexpr auto sink = lexy::fold_inplace<int>(0, [](int& result, int i) { result += i; });
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
        constexpr auto sink = lexy::fold_inplace<int>(0, [](int& result, int i) { result += i; });
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

