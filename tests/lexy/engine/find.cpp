// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/find.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/literal.hpp>

namespace
{
constexpr auto trie_ab    = lexy::linear_trie<LEXY_NTTP_STRING("ab")>;
constexpr auto trie_limit = lexy::linear_trie<LEXY_NTTP_STRING("!")>;
} // namespace

TEST_CASE("engine_find")
{
    using condition = lexy::engine_literal<trie_ab>;
    using engine    = lexy::engine_find<condition>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == engine::error_code::not_found);

    auto zero = engine_matches<engine>("ab");
    CHECK(zero);
    CHECK(zero.count == 0);
    auto one = engine_matches<engine>("-ab");
    CHECK(one);
    CHECK(one.count == 1);
    auto two = engine_matches<engine>("-+ab");
    CHECK(two);
    CHECK(two.count == 2);
    auto three = engine_matches<engine>("+-+ab");
    CHECK(three);
    CHECK(three.count == 3);

    auto partial = engine_matches<engine>("+a+ab");
    CHECK(partial);
    CHECK(partial.count == 3);

    auto partial_end = engine_matches<engine>("+a+a");
    CHECK(!partial_end);
    CHECK(partial_end.count == 4);
    CHECK(partial_end.ec == engine::error_code::not_found);

    auto unterminated = engine_matches<engine>("++");
    CHECK(!unterminated);
    CHECK(unterminated.count == 2);
    CHECK(unterminated.ec == engine::error_code::not_found);
}

TEST_CASE("engine_find_before")
{
    using condition = lexy::engine_literal<trie_ab>;
    using limit     = lexy::engine_literal<trie_limit>;
    using engine    = lexy::engine_find_before<condition, limit>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == engine::error_code::not_found_eof);

    auto zero = engine_matches<engine>("ab");
    CHECK(zero);
    CHECK(zero.count == 0);
    auto one = engine_matches<engine>("-ab");
    CHECK(one);
    CHECK(one.count == 1);
    auto two = engine_matches<engine>("-+ab");
    CHECK(two);
    CHECK(two.count == 2);
    auto three = engine_matches<engine>("+-+ab");
    CHECK(three);
    CHECK(three.count == 3);

    auto partial = engine_matches<engine>("+a+ab");
    CHECK(partial);
    CHECK(partial.count == 3);

    auto partial_end = engine_matches<engine>("+a+a");
    CHECK(!partial_end);
    CHECK(partial_end.count == 4);
    CHECK(partial_end.ec == engine::error_code::not_found_eof);

    auto unterminated = engine_matches<engine>("++");
    CHECK(!unterminated);
    CHECK(unterminated.count == 2);
    CHECK(unterminated.ec == engine::error_code::not_found_eof);

    auto limited = engine_matches<engine>("++!ab");
    CHECK(!limited);
    CHECK(limited.count == 2);
    CHECK(limited.ec == engine::error_code::not_found_limit);
}

