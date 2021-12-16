// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/alternative.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/literal.hpp>

TEST_CASE("dsl::operator/")
{
    constexpr auto callback = token_callback;

    SUBCASE("literals only")
    {
        constexpr auto rule = LEXY_LIT("a") / LEXY_LIT("ab") / LEXY_LIT("abc") / LEXY_LIT("def");
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted alternatives").cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().literal("a"));
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().literal("ab"));
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto def = LEXY_VERIFY("def");
        CHECK(def.status == test_result::success);
        CHECK(def.trace == test_trace().literal("def"));

        auto aa = LEXY_VERIFY("a");
        CHECK(aa.status == test_result::success);
        CHECK(aa.trace == test_trace().literal("a"));

        auto ABC = LEXY_VERIFY("ABC");
        CHECK(ABC.status == test_result::fatal_error);
        CHECK(ABC.trace == test_trace().error(0, 0, "exhausted alternatives").cancel());
    }
    SUBCASE("non-literals only")
    {
        constexpr auto rule = dsl::ascii::alnum / dsl::identifier(dsl::ascii::lower).pattern()
                              / dsl::token(dsl::ascii::upper + dsl::ascii::upper);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted alternatives").cancel());

        auto digit = LEXY_VERIFY("1");
        CHECK(digit.status == test_result::success);
        CHECK(digit.trace == test_trace().token("1"));
        auto lower = LEXY_VERIFY("a");
        CHECK(lower.status == test_result::success);
        CHECK(lower.trace == test_trace().token("a"));
        auto upper = LEXY_VERIFY("A");
        CHECK(upper.status == test_result::success);
        CHECK(upper.trace == test_trace().token("A"));

        auto three_lower = LEXY_VERIFY("abc");
        CHECK(three_lower.status == test_result::success);
        CHECK(three_lower.trace == test_trace().token("abc"));
        auto two_upper = LEXY_VERIFY("XY");
        CHECK(two_upper.status == test_result::success);
        CHECK(two_upper.trace == test_trace().token("XY"));

        auto two_digit = LEXY_VERIFY("11");
        CHECK(two_digit.status == test_result::success);
        CHECK(two_digit.trace == test_trace().token("1"));
        auto three_upper = LEXY_VERIFY("XYZ");
        CHECK(three_upper.status == test_result::success);
        CHECK(three_upper.trace == test_trace().token("XY"));
    }
    SUBCASE("mixed")
    {
        constexpr auto rule = LEXY_LIT("12") / dsl::ascii::alnum
                              / dsl::identifier(dsl::ascii::lower).pattern() / LEXY_LIT("abc")
                              / dsl::token(dsl::ascii::upper + dsl::ascii::upper) / LEXY_LIT("123");
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted alternatives").cancel());

        auto digit = LEXY_VERIFY("1");
        CHECK(digit.status == test_result::success);
        CHECK(digit.trace == test_trace().token("1"));
        auto lower = LEXY_VERIFY("a");
        CHECK(lower.status == test_result::success);
        CHECK(lower.trace == test_trace().token("a"));
        auto upper = LEXY_VERIFY("A");
        CHECK(upper.status == test_result::success);
        CHECK(upper.trace == test_trace().token("A"));
        auto one_two = LEXY_VERIFY("12");
        CHECK(one_two.status == test_result::success);
        CHECK(one_two.trace == test_trace().token("12"));
        auto one_two_three = LEXY_VERIFY("123");
        CHECK(one_two_three.status == test_result::success);
        CHECK(one_two_three.trace == test_trace().token("123"));
        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().token("abc"));

        auto four_lower = LEXY_VERIFY("abcd");
        CHECK(four_lower.status == test_result::success);
        CHECK(four_lower.trace == test_trace().token("abcd"));
        auto two_upper = LEXY_VERIFY("XY");
        CHECK(two_upper.status == test_result::success);
        CHECK(two_upper.trace == test_trace().token("XY"));

        auto two_digit = LEXY_VERIFY("11");
        CHECK(two_digit.status == test_result::success);
        CHECK(two_digit.trace == test_trace().token("1"));
        auto three_upper = LEXY_VERIFY("XYZ");
        CHECK(three_upper.status == test_result::success);
        CHECK(three_upper.trace == test_trace().token("XY"));
    }

    SUBCASE("code point")
    {
        constexpr auto rule
            = dsl::lit_cp<'a'> / dsl::lit_cp<'b'> / dsl::lit_cp<0x12345> / dsl::lit_cp<0x12346>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(u"");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted alternatives").cancel());

        auto a = LEXY_VERIFY(u"a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().literal("a"));
        auto b = LEXY_VERIFY(u"b");
        CHECK(b.status == test_result::success);
        CHECK(b.trace == test_trace().literal("b"));

        auto first = LEXY_VERIFY(u"\U00012345");
        CHECK(first.status == test_result::success);
        CHECK(first.trace == test_trace().literal("\\U00012345"));
        auto second = LEXY_VERIFY(u"\U00012346");
        CHECK(second.status == test_result::success);
        CHECK(second.trace == test_trace().literal("\\U00012346"));
    }
    SUBCASE("null bytes")
    {
        constexpr auto rule = dsl::lit_b<0> / dsl::lit_b<0, 0> / dsl::lit_b<'a', 0, 'b'>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted alternatives").cancel());

        auto null = LEXY_VERIFY("\0", 1);
        CHECK(null.status == test_result::success);
        CHECK(null.trace == test_trace().literal("\\0"));
        auto double_null = LEXY_VERIFY("\0\0", 2);
        CHECK(double_null.status == test_result::success);
        CHECK(double_null.trace == test_trace().literal("\\0\\0"));

        auto a_null_b = LEXY_VERIFY("a\0b", 3);
        CHECK(a_null_b.status == test_result::success);
        CHECK(a_null_b.trace == test_trace().literal("a\\0b"));
    }
}

