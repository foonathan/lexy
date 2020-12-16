// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/minus.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/failure.hpp>
#include <lexy/engine/literal.hpp>
#include <lexy/engine/until.hpp>

namespace
{
constexpr auto condition_trie = lexy::linear_trie<LEXY_NTTP_STRING("!")>;
constexpr auto trie_a         = lexy::linear_trie<LEXY_NTTP_STRING("a!")>;
constexpr auto trie_bc        = lexy::linear_trie<LEXY_NTTP_STRING("bc!")>;
} // namespace

TEST_CASE("engine_minus")
{
    using condition = lexy::engine_literal<condition_trie>;
    using until     = lexy::engine_until<condition>;

    using except_a  = lexy::engine_literal<trie_a>;
    using except_bc = lexy::engine_literal<trie_bc>;

    using engine = lexy::engine_minus<until, except_a, except_bc>;
    CHECK(lexy::engine_is_matcher<engine>);

    constexpr auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == engine::error_from_matcher(condition::index_to_error(0)));

    constexpr auto zero = engine_matches<engine>("!");
    CHECK(zero);
    CHECK(zero.count == 1);
    constexpr auto cde = engine_matches<engine>("cde!");
    CHECK(cde);
    CHECK(cde.count == 4);

    constexpr auto a = engine_matches<engine>("a!");
    CHECK(!a);
    CHECK(a.count == 2);
    CHECK(a.ec == engine::error_code::minus_failure);
    constexpr auto bc = engine_matches<engine>("bc!");
    CHECK(!bc);
    CHECK(bc.count == 3);
    CHECK(bc.ec == engine::error_code::minus_failure);

    constexpr auto abc = engine_matches<engine>("abc!");
    CHECK(abc);
    CHECK(abc.count == 4);
    constexpr auto bcd = engine_matches<engine>("bcd!");
    CHECK(bcd);
    CHECK(bcd.count == 4);
}

