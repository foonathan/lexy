// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/literal.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>

namespace
{
constexpr auto trie_empty = lexy::linear_trie<LEXY_NTTP_STRING("")>;
constexpr auto trie_a     = lexy::linear_trie<LEXY_NTTP_STRING("a")>;
constexpr auto trie_ab    = lexy::linear_trie<LEXY_NTTP_STRING("ab")>;
constexpr auto trie_abc   = lexy::linear_trie<LEXY_NTTP_STRING("abc")>;
constexpr auto trie_abc_u = lexy::linear_trie<LEXY_NTTP_STRING(u"abc")>;
} // namespace

TEST_CASE("engine_literal")
{
    SUBCASE("empty")
    {
        using engine = lexy::engine_literal<trie_empty>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(empty);
        CHECK(empty.count == 0);

        auto a = engine_matches<engine>("a");
        CHECK(a);
        CHECK(a.count == 0);
        auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 0);

        auto b = engine_matches<engine>("b");
        CHECK(b);
        CHECK(b.count == 0);
    }
    SUBCASE("a")
    {
        using engine = lexy::engine_literal<trie_a>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::index_to_error(0));

        auto a = engine_matches<engine>("a");
        CHECK(a);
        CHECK(a.count == 1);
        auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 1);

        auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
        CHECK(b.ec == engine::index_to_error(0));
    }
    SUBCASE("ab")
    {
        using engine = lexy::engine_literal<trie_ab>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::index_to_error(0));

        auto a = engine_matches<engine>("a");
        CHECK(!a);
        CHECK(a.count == 1);
        CHECK(a.ec == engine::index_to_error(1));

        auto ab = engine_matches<engine>("ab");
        CHECK(ab);
        CHECK(ab.count == 2);
        auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 2);

        auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
        CHECK(b.ec == engine::index_to_error(0));
    }
    SUBCASE("abc")
    {
        using engine = lexy::engine_literal<trie_abc>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == engine::index_to_error(0));

        auto a = engine_matches<engine>("a");
        CHECK(!a);
        CHECK(a.count == 1);
        CHECK(a.ec == engine::index_to_error(1));
        auto ab = engine_matches<engine>("ab");
        CHECK(!ab);
        CHECK(ab.count == 2);
        CHECK(ab.ec == engine::index_to_error(2));

        auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        auto abcd = engine_matches<engine>("abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);

        auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
    }
    SUBCASE("abc in UTF-16")
    {
        using engine = lexy::engine_literal<trie_abc_u>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);

        auto a = engine_matches<engine>(u"a");
        CHECK(!a);
        CHECK(a.count == 1);
        auto ab = engine_matches<engine>(u"ab");
        CHECK(!ab);
        CHECK(ab.count == 2);

        auto abc = engine_matches<engine>(u"abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        auto abcd = engine_matches<engine>(u"abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);

        auto b = engine_matches<engine>(u"b");
        CHECK(!b);
        CHECK(b.count == 0);
        CHECK(b.ec == engine::index_to_error(0));
    }
}

