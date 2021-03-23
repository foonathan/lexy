// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/char_class.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>

TEST_CASE("engine_char_range")
{
    using engine = lexy::engine_char_range<'0', '9'>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.recovered == 0);

    for (auto digit = '0'; digit <= '9'; ++digit)
    {
        INFO(digit);
        const char str[] = {digit, digit, digit, '\0'};

        auto result = engine_matches<engine>(str);
        CHECK(result);
        CHECK(result.count == 1);
    }

    for (auto non_digit : "abcdefgh")
    {
        if (non_digit == '\0')
            continue;
        const char str[] = {non_digit, non_digit, non_digit, '\0'};

        auto result = engine_matches<engine>(str);
        CHECK(!result);
        CHECK(result.count == 0);
        CHECK(result.recovered == 1);
    }
}

namespace
{
constexpr auto trie_a     = lexy::shallow_trie<LEXY_NTTP_STRING("a")>;
constexpr auto trie_abc   = lexy::shallow_trie<LEXY_NTTP_STRING("abc")>;
constexpr auto trie_abc_u = lexy::shallow_trie<LEXY_NTTP_STRING(u"abc")>;
} // namespace

TEST_CASE("engine_char_set")
{
    SUBCASE("{'a'}")
    {
        using engine = lexy::engine_char_set<trie_a>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.recovered == 0);

        auto a = engine_matches<engine>("aaa");
        CHECK(a);
        CHECK(a.count == 1);

        auto b = engine_matches<engine>("bbb");
        CHECK(!b);
        CHECK(b.count == 0);
        CHECK(b.recovered == 1);

        auto c = engine_matches<engine>("ccc");
        CHECK(!c);
        CHECK(c.count == 0);
        CHECK(c.recovered == 1);

        auto d = engine_matches<engine>("ddd");
        CHECK(!d);
        CHECK(d.count == 0);
    }
    SUBCASE("{'a', 'b', 'c'}")
    {
        using engine = lexy::engine_char_set<trie_abc>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.recovered == 0);

        auto a = engine_matches<engine>("aaa");
        CHECK(a);
        CHECK(a.count == 1);

        auto b = engine_matches<engine>("bbb");
        CHECK(b);
        CHECK(b.count == 1);

        auto c = engine_matches<engine>("ccc");
        CHECK(c);
        CHECK(c.count == 1);

        auto d = engine_matches<engine>("ddd");
        CHECK(!d);
        CHECK(d.count == 0);
        CHECK(d.recovered == 1);
    }

    SUBCASE("{'a', 'b', 'c'} in UTF-16")
    {
        using engine = lexy::engine_char_set<trie_abc_u>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.recovered == 0);

        auto a = engine_matches<engine>(u"aaa");
        CHECK(a);
        CHECK(a.count == 1);

        auto b = engine_matches<engine>(u"bbb");
        CHECK(b);
        CHECK(b.count == 1);

        auto c = engine_matches<engine>(u"ccc");
        CHECK(c);
        CHECK(c.count == 1);

        auto d = engine_matches<engine>(u"ddd");
        CHECK(!d);
        CHECK(d.count == 0);
        CHECK(d.recovered == 1);
    }
}

namespace
{
constexpr auto table = [] {
    lexy::ascii_table<3> table;

    for (char c = 0; c < 0x20; ++c)
        table.insert(c, 0);
    for (char c = 0x20; c < 0x40; ++c)
        table.insert(c, 1);

    for (char c = 0x40; c < 0x60; ++c)
    {
        table.insert(c, 0);
        table.insert(c, 2);
    }

    return table;
}();
}

TEST_CASE("engine_ascii_table")
{
    SUBCASE("table interface")
    {
        for (char c = 0; c < 0x20; ++c)
        {
            CHECK(table.contains<lexy::default_encoding, 0>(c));
            CHECK(!table.contains<lexy::default_encoding, 1>(c));
            CHECK(!table.contains<lexy::default_encoding, 2>(c));
        }

        for (char c = 0x20; c < 0x40; ++c)
        {
            CHECK(!table.contains<lexy::default_encoding, 0>(c));
            CHECK(table.contains<lexy::default_encoding, 1>(c));
            CHECK(!table.contains<lexy::default_encoding, 2>(c));
        }

        for (char c = 0x40; c < 0x60; ++c)
        {
            CHECK(table.contains<lexy::default_encoding, 0>(c));
            CHECK(!table.contains<lexy::default_encoding, 1>(c));
            CHECK(table.contains<lexy::default_encoding, 2>(c));
        }

        for (auto c = 0x60; c < 0xFF; ++c)
        {
            CHECK(!table.contains<lexy::default_encoding, 0>(c));
            CHECK(!table.contains<lexy::default_encoding, 1>(c));
            CHECK(!table.contains<lexy::default_encoding, 2>(c));
        }
    }

    SUBCASE("category 1")
    {
        using engine = lexy::engine_ascii_table<table, 1>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.recovered == 0);

        for (auto c = 0x00; c < 0x20; ++c)
        {
            const char str[] = {char(c), char(c), char(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(!result);
            CHECK(result.count == 0);
            CHECK(result.recovered == 1);
        }

        for (auto c = 0x20; c < 0x40; ++c)
        {
            const char str[] = {char(c), char(c), char(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(result);
            CHECK(result.count == 1);
        }

        for (auto c = 0x40; c < 0xFF; ++c)
        {
            const char str[] = {char(c), char(c), char(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(!result);
            CHECK(result.count == 0);
            CHECK(result.recovered == 1);
        }
    }
    SUBCASE("category 1 or 2")
    {
        using engine = lexy::engine_ascii_table<table, 1, 2>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.recovered == 0);

        for (auto c = 0x00; c < 0x20; ++c)
        {
            const char str[] = {char(c), char(c), char(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(!result);
            CHECK(result.count == 0);
            CHECK(result.recovered == 1);
        }

        for (auto c = 0x20; c < 0x60; ++c)
        {
            const char str[] = {char(c), char(c), char(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(result);
            CHECK(result.count == 1);
        }

        for (auto c = 0x60; c < 0xFF; ++c)
        {
            const char str[] = {char(c), char(c), char(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(!result);
            CHECK(result.count == 0);
            CHECK(result.recovered == 1);
        }
    }

    SUBCASE("category 1 in UTF-16")
    {
        using engine = lexy::engine_ascii_table<table, 1>;
        CHECK(lexy::engine_is_matcher<engine>);

        auto empty = engine_matches<engine>(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.recovered == 0);

        for (auto c = 0x00; c < 0x20; ++c)
        {
            const char16_t str[] = {char16_t(c), char16_t(c), char16_t(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(!result);
            CHECK(result.count == 0);
            CHECK(result.recovered == 1);
        }

        for (auto c = 0x20; c < 0x40; ++c)
        {
            const char16_t str[] = {char16_t(c), char16_t(c), char16_t(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(result);
            CHECK(result.count == 1);
        }

        for (auto c = 0x40; c < 0xFFFF; ++c)
        {
            const char16_t str[] = {char16_t(c), char16_t(c), char16_t(c)};

            auto result = engine_matches<engine>(str, 3);
            CHECK(!result);
            CHECK(result.count == 0);
            CHECK(result.recovered == 1);
        }
    }
}

