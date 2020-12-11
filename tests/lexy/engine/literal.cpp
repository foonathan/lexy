// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/literal.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>

TEST_CASE("engine_literal")
{
    SUBCASE("a")
    {
        using engine = lexy::engine_literal<LEXY_NTTP_STRING("a")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>("a");
        CHECK(a);
        CHECK(a.count == 1);
        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 1);

        constexpr auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
    }
    SUBCASE("ab")
    {
        using engine = lexy::engine_literal<LEXY_NTTP_STRING("ab")>;
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
        CHECK(abc.count == 2);

        constexpr auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
    }
    SUBCASE("abc")
    {
        using engine = lexy::engine_literal<LEXY_NTTP_STRING("abc")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>("a");
        CHECK(!a);
        CHECK(a.count == 1);
        constexpr auto ab = engine_matches<engine>("ab");
        CHECK(!ab);
        CHECK(ab.count == 2);

        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        constexpr auto abcd = engine_matches<engine>("abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);

        constexpr auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
    }
    SUBCASE("abcd")
    {
        using engine = lexy::engine_literal<LEXY_NTTP_STRING("abcd")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>("");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>("a");
        CHECK(!a);
        CHECK(a.count == 1);
        constexpr auto ab = engine_matches<engine>("ab");
        CHECK(!ab);
        CHECK(ab.count == 2);
        constexpr auto abc = engine_matches<engine>("abc");
        CHECK(!abc);
        CHECK(abc.count == 3);

        constexpr auto abcd = engine_matches<engine>("abcd");
        CHECK(abcd);
        CHECK(abcd.count == 4);
        constexpr auto abcde = engine_matches<engine>("abcde");
        CHECK(abcde);
        CHECK(abcde.count == 4);

        constexpr auto b = engine_matches<engine>("b");
        CHECK(!b);
        CHECK(b.count == 0);
    }

    SUBCASE("abc in UTF-16")
    {
        using engine = lexy::engine_literal<LEXY_NTTP_STRING(u"abc")>;
        CHECK(lexy::engine_is_matcher<engine>);

        constexpr auto empty = engine_matches<engine>(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);

        constexpr auto a = engine_matches<engine>(u"a");
        CHECK(!a);
        CHECK(a.count == 1);
        constexpr auto ab = engine_matches<engine>(u"ab");
        CHECK(!ab);
        CHECK(ab.count == 2);

        constexpr auto abc = engine_matches<engine>(u"abc");
        CHECK(abc);
        CHECK(abc.count == 3);
        constexpr auto abcd = engine_matches<engine>(u"abcd");
        CHECK(abcd);
        CHECK(abcd.count == 3);

        constexpr auto b = engine_matches<engine>(u"b");
        CHECK(!b);
        CHECK(b.count == 0);
    }
}

