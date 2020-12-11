// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/char_class.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>

TEST_CASE("engine_char_set")
{
    SUBCASE("{'a'}")
    {
        using engine = lexy::engine_char_set<LEXY_NTTP_STRING("a")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>("aaa");
        CHECK(a);
        CHECK(a.count == 1);

        constexpr auto b = engine_matches<engine>("bbb");
        CHECK(!b);
        CHECK(b.count == 0);

        constexpr auto c = engine_matches<engine>("ccc");
        CHECK(!c);
        CHECK(c.count == 0);

        constexpr auto d = engine_matches<engine>("ddd");
        CHECK(!d);
        CHECK(d.count == 0);
    }
    SUBCASE("{'a', 'b', 'c'}")
    {
        using engine = lexy::engine_char_set<LEXY_NTTP_STRING("abc")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>("aaa");
        CHECK(a);
        CHECK(a.count == 1);

        constexpr auto b = engine_matches<engine>("bbb");
        CHECK(b);
        CHECK(b.count == 1);

        constexpr auto c = engine_matches<engine>("ccc");
        CHECK(c);
        CHECK(c.count == 1);

        constexpr auto d = engine_matches<engine>("ddd");
        CHECK(!d);
        CHECK(d.count == 0);
    }

    SUBCASE("{'a', 'b', 'c'} in UTF-16")
    {
        using engine = lexy::engine_char_set<LEXY_NTTP_STRING(u"abc")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>(u"aaa");
        CHECK(a);
        CHECK(a.count == 1);

        constexpr auto b = engine_matches<engine>(u"bbb");
        CHECK(b);
        CHECK(b.count == 1);

        constexpr auto c = engine_matches<engine>(u"ccc");
        CHECK(c);
        CHECK(c.count == 1);

        constexpr auto d = engine_matches<engine>(u"ddd");
        CHECK(!d);
        CHECK(d.count == 0);
    }
}

TEST_CASE("engine_char_range")
{
    using engine = lexy::engine_char_range<'0', '9'>;
    CHECK(lexy::engine_is_matcher<engine>);

    constexpr auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);

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
        INFO(non_digit);
        const char str[] = {non_digit, non_digit, non_digit, '\0'};

        auto result = engine_matches<engine>(str);
        CHECK(!result);
        CHECK(result.count == 0);
    }
}

