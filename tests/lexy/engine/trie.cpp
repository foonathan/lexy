// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
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
    auto parse = [](auto engine, auto str) {
        auto match_result = engine_matches<decltype(engine)>(str);
        auto parse_result = engine_parses<decltype(engine)>(str);

        CHECK(match_result.ec == parse_result.ec);
        CHECK(match_result.count == parse_result.count);
        CHECK(match_result.recovered == parse_result.recovered);

        return parse_result;
    };

    SUBCASE("empty trie")
    {
        using engine = lexy::engine_trie<trie_empty>;
        CHECK(lexy::engine_is_matcher<engine>);
        CHECK(lexy::engine_is_parser<engine>);
        CHECK(lexy::engine_can_fail<engine, lexy::string_input<>>);
        CHECK(!lexy::engine_can_succeed<engine, lexy::string_input<>>);

        auto empty = parse(engine{}, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.value == trie_empty.invalid_value);

        auto abc = parse(engine{}, "abc");
        CHECK(!abc);
        CHECK(abc.count == 0);
        CHECK(abc.value == trie_empty.invalid_value);
    }
    SUBCASE("empty string")
    {
        using engine = lexy::engine_trie<trie_empty_string>;
        CHECK(lexy::engine_is_matcher<engine>);
        CHECK(lexy::engine_is_parser<engine>);
        CHECK(!lexy::engine_can_fail<engine, lexy::string_input<>>);
        CHECK(lexy::engine_can_succeed<engine, lexy::string_input<>>);

        auto empty = parse(engine{}, "");
        CHECK(empty);
        CHECK(empty.count == 0);
        CHECK(empty.value == 0);

        auto abc = parse(engine{}, "abc");
        CHECK(abc);
        CHECK(abc.count == 0);
        CHECK(abc.value == 0);
    }
    SUBCASE("basic")
    {
        using engine = lexy::engine_trie<trie_basic>;
        CHECK(lexy::engine_is_matcher<engine>);
        CHECK(lexy::engine_is_parser<engine>);
        CHECK(lexy::engine_can_fail<engine, lexy::string_input<>>);
        CHECK(lexy::engine_can_succeed<engine, lexy::string_input<>>);

        auto empty = parse(engine{}, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.value == trie_basic.invalid_value);

        auto a = parse(engine{}, "a");
        CHECK(!a);
        CHECK(a.count == 1);
        CHECK(a.value == trie_basic.invalid_value);
        auto ab = parse(engine{}, "ab");
        CHECK(ab);
        CHECK(ab.count == 2);
        CHECK(ab.value == 0);
        auto abc = parse(engine{}, "abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        CHECK(abc.value == 1);
        auto ac = parse(engine{}, "ac");
        CHECK(ac);
        CHECK(ac.count == 2);
        CHECK(ac.value == 2);

        auto b = parse(engine{}, "b");
        CHECK(!b);
        CHECK(b.count == 1);
        CHECK(b.value == trie_basic.invalid_value);
        auto bc = parse(engine{}, "bc");
        CHECK(!bc);
        CHECK(bc.count == 2);
        CHECK(bc.value == trie_basic.invalid_value);
        auto bcd = parse(engine{}, "bcd");
        CHECK(bcd);
        CHECK(bcd.count == 3);
        CHECK(bcd.value == 3);

        auto abd = parse(engine{}, "abd");
        CHECK(abd);
        CHECK(abd.count == 2);
        CHECK(abd.value == 0);
        auto abcd = parse(engine{}, "abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);
        CHECK(abcd.value == 1);
    }
    SUBCASE("completely linear")
    {
        using engine = lexy::engine_trie<trie_linear>;
        CHECK(lexy::engine_is_matcher<engine>);
        CHECK(lexy::engine_is_parser<engine>);
        CHECK(!lexy::engine_can_fail<engine, lexy::string_input<>>);
        CHECK(lexy::engine_can_succeed<engine, lexy::string_input<>>);

        auto empty = parse(engine{}, "");
        CHECK(empty);
        CHECK(empty.count == 0);
        CHECK(empty.value == 0);
        auto a = parse(engine{}, "a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value == 1);
        auto ab = parse(engine{}, "ab");
        CHECK(ab);
        CHECK(ab.count == 2);
        CHECK(ab.value == 2);
        auto abc = parse(engine{}, "abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        CHECK(abc.value == 3);

        auto abcd = parse(engine{}, "abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);
        CHECK(abcd.value == 3);

        auto bcd = parse(engine{}, "bcd");
        CHECK(bcd);
        CHECK(bcd.count == 0);
        CHECK(bcd.value == 0);
    }
    SUBCASE("completely disjoint")
    {
        using engine = lexy::engine_trie<trie_disjoint>;
        CHECK(lexy::engine_is_matcher<engine>);
        CHECK(lexy::engine_is_parser<engine>);
        CHECK(lexy::engine_can_fail<engine, lexy::string_input<>>);
        CHECK(lexy::engine_can_succeed<engine, lexy::string_input<>>);

        auto empty = parse(engine{}, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.value == trie_disjoint.invalid_value);

        auto abc = parse(engine{}, "abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        CHECK(abc.value == 0);
        auto bcd = parse(engine{}, "bcd");
        CHECK(bcd);
        CHECK(bcd.count == 3);
        CHECK(bcd.value == 1);
        auto cde = parse(engine{}, "cde");
        CHECK(cde);
        CHECK(cde.count == 3);
        CHECK(cde.value == 2);
    }
}

