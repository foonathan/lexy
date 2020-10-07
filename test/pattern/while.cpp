// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/while.hpp>

#include "verify.hpp"
#include <lexy/dsl/condition.hpp>

TEST_CASE("pattern: while")
{
    SUBCASE("simple")
    {
        constexpr auto pattern = while_(LEXY_LIT("ab"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match().empty());

        constexpr auto ab = pattern_matches(pattern, "ab");
        CHECK(ab);
        CHECK(ab.match().string_view() == "ab");

        constexpr auto abab = pattern_matches(pattern, "abab");
        CHECK(abab);
        CHECK(abab.match().string_view() == "abab");

        constexpr auto ababab = pattern_matches(pattern, "ababab");
        CHECK(ababab);
        CHECK(ababab.match().string_view() == "ababab");
    }
    SUBCASE("branch")
    {
        constexpr auto pattern = while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(!a);
        CHECK(a.match().empty());

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match().string_view() == "abc");
        constexpr auto abcabc = pattern_matches(pattern, "abcabc");
        CHECK(abcabc);
        CHECK(abcabc.match().string_view() == "abcabc");

        constexpr auto abcabca = pattern_matches(pattern, "abcabca");
        CHECK(!abcabca);
        CHECK(abcabca.match().empty());
    }
    SUBCASE("negative")
    {
        constexpr auto pattern = while_(!LEXY_LIT("a") >> LEXY_LIT("b"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto a = pattern_matches(pattern, "a");
        CHECK(a);
        CHECK(a.match().string_view() == "a");
        constexpr auto ba = pattern_matches(pattern, "ba");
        CHECK(ba);
        CHECK(ba.match().string_view() == "ba");
        constexpr auto bba = pattern_matches(pattern, "bba");
        CHECK(bba);
        CHECK(bba.match().string_view() == "bba");

        constexpr auto bb = pattern_matches(pattern, "bb");
        CHECK(!bb);
    }
}

