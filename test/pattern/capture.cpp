// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/capture.hpp>

#include "verify.hpp"
#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/sequence.hpp>

TEST_CASE("pattern: capture")
{
    SUBCASE("basic")
    {
        constexpr auto pattern = capture(LEXY_LIT("abc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto success = pattern_matches(pattern, "abc");
        CHECK(success);
        CHECK(success.id() == 0);
        CHECK(success.match().string_view() == "abc");
        CHECK(success.capture_count() == 1);
        CHECK(success.capture(0).string_view() == "abc");

        constexpr auto additional = pattern_matches(pattern, "abcde");
        CHECK(additional);
        CHECK(additional.id() == 0);
        CHECK(additional.match().string_view() == "abc");
        CHECK(additional.capture_count() == 1);
        CHECK(additional.capture(0).string_view() == "abc");
    }
    SUBCASE("directly nested")
    {
        constexpr auto pattern = capture(capture(LEXY_LIT("abc")));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto success = pattern_matches(pattern, "abc");
        CHECK(success);
        CHECK(success.id() == 0);
        CHECK(success.match().string_view() == "abc");
        CHECK(success.capture_count() == 2);
        CHECK(success.capture(0).string_view() == "abc");
        CHECK(success.capture(1).string_view() == "abc");
    }
    SUBCASE("indirectly nested")
    {
        constexpr auto pattern = capture(LEXY_LIT("(") + capture(LEXY_LIT("abc")) + LEXY_LIT(")"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto success = pattern_matches(pattern, "(abc)");
        CHECK(success);
        CHECK(success.id() == 0);
        CHECK(success.match().string_view() == "(abc)");
        CHECK(success.capture_count() == 2);
        CHECK(success.capture(0).string_view() == "(abc)");
        CHECK(success.capture(1).string_view() == "abc");
    }
    SUBCASE("alternative")
    {
        constexpr auto pattern = capture(LEXY_LIT("abc")) / capture(LEXY_LIT("def"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.id() == 0);
        CHECK(abc.match().string_view() == "abc");
        CHECK(abc.capture_count() == 1);
        CHECK(abc.capture(0).string_view() == "abc");

        constexpr auto def = pattern_matches(pattern, "def");
        CHECK(def);
        CHECK(def.id() == 0);
        CHECK(def.match().string_view() == "def");
        CHECK(def.capture_count() == 1);
        CHECK(def.capture(0).string_view() == "def");
    }
}

