// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/trie.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>

namespace
{
constexpr auto trie_empty        = lexy::trie<char>;
constexpr auto trie_empty_string = lexy::trie<char, LEXY_NTTP_STRING("")>;
constexpr auto trie_basic        = lexy::trie<char, LEXY_NTTP_STRING("ab"), LEXY_NTTP_STRING("abc"),
                                       LEXY_NTTP_STRING("ac"), LEXY_NTTP_STRING("bcd")>;
constexpr auto trie_linear       = lexy::trie<char, LEXY_NTTP_STRING(""), LEXY_NTTP_STRING("a"),
                                        LEXY_NTTP_STRING("ab"), LEXY_NTTP_STRING("abc")>;
constexpr auto trie_disjoint
    = lexy::trie<char, LEXY_NTTP_STRING("abc"), LEXY_NTTP_STRING("bcd"), LEXY_NTTP_STRING("cde")>;
} // namespace

TEST_CASE("engine_trie")
{
    SUBCASE("empty trie")
    {
        using engine = lexy::engine_trie<trie_empty>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(!abc);
        CHECK(abc.count == 0);
    }
    SUBCASE("empty string")
    {
        using engine = lexy::engine_trie<trie_empty_string>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(empty);
        CHECK(empty.count == 0);

        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 0);
    }
    SUBCASE("basic")
    {
        using engine = lexy::engine_trie<trie_basic>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>("a");
        CHECK(!a);
        CHECK(a.count == 1);
        constexpr auto ab = engine_matches<engine>("ab");
        CHECK(ab);
        CHECK(ab.count == 2);
        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        constexpr auto ac = engine_matches<engine>("ac");
        CHECK(ac);
        CHECK(ac.count == 2);

        constexpr auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 1);
        constexpr auto bc = engine_matches<engine>("bc");
        CHECK(!bc);
        CHECK(bc.count == 2);
        auto bcd = engine_matches<engine>("bcd");
        CHECK(bcd);
        CHECK(bcd.count == 3);

        constexpr auto abd = engine_matches<engine>("abd");
        CHECK(abd);
        CHECK(abd.count == 2);
        constexpr auto abcd = engine_matches<engine>("abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);
    }
    SUBCASE("completely linear")
    {
        using engine = lexy::engine_trie<trie_linear>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(empty);
        CHECK(empty.count == 0);
        constexpr auto a = engine_matches<engine>("a");
        CHECK(a);
        CHECK(a.count == 1);
        constexpr auto ab = engine_matches<engine>("ab");
        CHECK(ab);
        CHECK(ab.count == 2);
        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 3);

        constexpr auto abcd = engine_matches<engine>("abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);

        constexpr auto bcd = engine_matches<engine>("bcd");
        CHECK(bcd);
        CHECK(bcd.count == 0);
    }
    SUBCASE("completely disjoint")
    {
        using engine = lexy::engine_trie<trie_disjoint>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        constexpr auto bcd = engine_matches<engine>("bcd");
        CHECK(bcd);
        CHECK(bcd.count == 3);
        constexpr auto cde = engine_matches<engine>("cde");
        CHECK(cde);
        CHECK(cde.count == 3);
    }
}

