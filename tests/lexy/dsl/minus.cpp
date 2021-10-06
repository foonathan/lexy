// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/minus.hpp>

#include "verify.hpp"
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/until.hpp>

TEST_CASE("dsl::operator-")
{
    constexpr auto token    = dsl::until(dsl::lit_c<'!'>);
    constexpr auto callback = token_callback;

    SUBCASE("basic")
    {
        constexpr auto rule = token - LEXY_LIT("aa!");
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "!", 0).cancel());

        auto zero = LEXY_VERIFY("!");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("!"));
        auto one = LEXY_VERIFY("a!");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().token("a!"));
        auto three = LEXY_VERIFY("aaa!");
        CHECK(three.status == test_result::success);
        CHECK(three.trace == test_trace().token("aaa!"));

        auto two = LEXY_VERIFY("aa!");
        CHECK(two.status == test_result::fatal_error);
        CHECK(two.trace == test_trace().error(0, 3, "minus failure").error_token("aa!").cancel());
    }
    SUBCASE("prefix")
    {
        constexpr auto rule = token - dsl::lit_c<'b'>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "!", 0).cancel());

        auto zero = LEXY_VERIFY("!");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("!"));

        auto a = LEXY_VERIFY("a!");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().token("a!"));

        // The except rule needs to match everything.
        auto b = LEXY_VERIFY("b!");
        CHECK(b.status == test_result::success);
        CHECK(b.trace == test_trace().token("b!"));
    }
    SUBCASE("any")
    {
        constexpr auto rule = token - dsl::token(dsl::lit_c<'b'> + dsl::any);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "!", 0).cancel());

        auto zero = LEXY_VERIFY("!");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("!"));

        auto a = LEXY_VERIFY("a!");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().token("a!"));

        auto b = LEXY_VERIFY("b!");
        CHECK(b.status == test_result::fatal_error);
        CHECK(b.trace == test_trace().error(0, 2, "minus failure").error_token("b!").cancel());
    }
    SUBCASE("multiple subtractions")
    {
        constexpr auto rule = token - LEXY_LIT("a!") - LEXY_LIT("b!");
        CHECK(lexy::is_token_rule<decltype(rule)>);

        CHECK(equivalent_rules(rule, token - LEXY_LIT("a!") / LEXY_LIT("b!")));
    }
}

