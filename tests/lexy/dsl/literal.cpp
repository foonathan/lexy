// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/literal.hpp>

#include "verify.hpp"

TEST_CASE("dsl::lit")
{
    constexpr auto callback = token_callback;

    SUBCASE("ASCII")
    {
        constexpr auto rule = LEXY_LIT("abc");
        CHECK(lexy::is_token_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
        CHECK(equivalent_rules(rule, dsl::lit<"abc">));
#endif

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().token("abc"));
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::success);
        CHECK(abcd.trace == test_trace().token("abc"));

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().expected_literal(0, "abc", 1).error_token("a").cancel());
        auto ad = LEXY_VERIFY("ad");
        CHECK(ad.status == test_result::fatal_error);
        CHECK(ad.trace == test_trace().expected_literal(0, "abc", 1).error_token("a").cancel());

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().expected_literal(0, "abc", 2).error_token("ab").cancel());
        auto abd = LEXY_VERIFY("abd");
        CHECK(abd.status == test_result::fatal_error);
        CHECK(abd.trace == test_trace().expected_literal(0, "abc", 2).error_token("ab").cancel());

        auto ABC = LEXY_VERIFY("ABC");
        CHECK(ABC.status == test_result::fatal_error);
        CHECK(ABC.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto utf16 = LEXY_VERIFY(u"abc");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().token("abc"));
    }
    SUBCASE("UTF-16, but only in ASCII")
    {
        constexpr auto rule = LEXY_LIT(u"abc");
        CHECK(lexy::is_token_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
        CHECK(equivalent_rules(rule, dsl::lit<u"abc">));
#endif

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().token("abc"));

        auto utf16 = LEXY_VERIFY(u"abc");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().token("abc"));
    }
    SUBCASE("UTF-16, non ASCII")
    {
        constexpr auto rule = LEXY_LIT(u"äöü");
        CHECK(lexy::is_token_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
        CHECK(equivalent_rules(rule, dsl::lit<u"äöü">));
#endif

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "\\u00E4\\u00F6\\u00FC", 0).cancel());

        auto umlaute = LEXY_VERIFY(u"äöü");
        CHECK(umlaute.status == test_result::success);
        CHECK(umlaute.trace == test_trace().token("\\u00E4\\u00F6\\u00FC"));
    }
}

TEST_CASE("dsl::lit_c")
{
    constexpr auto rule = dsl::lit_c<'a'>;
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, LEXY_LIT("a")));
}

