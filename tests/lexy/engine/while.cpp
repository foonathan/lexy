// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/while.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/literal.hpp>

namespace
{
static constexpr auto trie = lexy::linear_trie<LEXY_NTTP_STRING("ab")>;
}

TEST_CASE("engine_while")
{
    using matcher = lexy::engine_literal<trie>;
    using engine  = lexy::engine_while<matcher>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(empty);
    CHECK(empty.count == 0);

    auto one = engine_matches<engine>("ab");
    CHECK(one);
    CHECK(one.count == 2);
    auto two = engine_matches<engine>("abab");
    CHECK(two);
    CHECK(two.count == 4);
    auto three = engine_matches<engine>("ababab");
    CHECK(three);
    CHECK(three.count == 6);

    auto partial = engine_matches<engine>("aba");
    CHECK(partial);
    CHECK(partial.count == 2);
}

