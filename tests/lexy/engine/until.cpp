// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/until.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/literal.hpp>

namespace
{
constexpr auto trie_ab = lexy::linear_trie<LEXY_NTTP_STRING("ab")>;
}

TEST_CASE("engine_until")
{
    using condition = lexy::engine_literal<trie_ab>;
    using engine    = lexy::engine_until<condition>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == condition::index_to_error(0));

    auto zero = engine_matches<engine>("ab");
    CHECK(zero);
    CHECK(zero.count == 2);
    auto one = engine_matches<engine>("-ab");
    CHECK(one);
    CHECK(one.count == 3);
    auto two = engine_matches<engine>("-+ab");
    CHECK(two);
    CHECK(two.count == 4);
    auto three = engine_matches<engine>("+-+ab");
    CHECK(three);
    CHECK(three.count == 5);

    auto partial = engine_matches<engine>("+a+ab");
    CHECK(partial);
    CHECK(partial.count == 5);

    auto partial_end = engine_matches<engine>("+a+a");
    CHECK(!partial_end);
    CHECK(partial_end.count == 4);
    CHECK(partial_end.ec == condition::index_to_error(0));

    auto unterminated = engine_matches<engine>("++");
    CHECK(!unterminated);
    CHECK(unterminated.count == 2);
    CHECK(unterminated.ec == condition::index_to_error(0));
}

TEST_CASE("engine_until_eof")
{
    using condition = lexy::engine_literal<trie_ab>;
    using engine    = lexy::engine_until_eof<condition>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(empty);
    CHECK(empty.count == 0);

    auto zero = engine_matches<engine>("ab");
    CHECK(zero);
    CHECK(zero.count == 2);
    auto one = engine_matches<engine>("-ab");
    CHECK(one);
    CHECK(one.count == 3);
    auto two = engine_matches<engine>("-+ab");
    CHECK(two);
    CHECK(two.count == 4);
    auto three = engine_matches<engine>("+-+ab");
    CHECK(three);
    CHECK(three.count == 5);

    auto partial = engine_matches<engine>("+a+ab");
    CHECK(partial);
    CHECK(partial.count == 5);

    auto partial_end = engine_matches<engine>("+a+a");
    CHECK(partial_end);
    CHECK(partial_end.count == 4);
}

