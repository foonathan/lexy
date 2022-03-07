// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/follow.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

TEST_CASE("dsl::not_followed_by()")
{
    constexpr auto rule = dsl::not_followed_by(LEXY_LIT("abc"), dsl::ascii::alpha);
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());
    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().expected_literal(0, "abc", 1).cancel());
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().expected_literal(0, "abc", 2).cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("abc"));

    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd.status == test_result::fatal_error);
    CHECK(abcd.trace == test_trace().error_token("abc").error(3, 3, "follow restriction").cancel());
}

TEST_CASE("dsl::followed_by()")
{
    constexpr auto rule = dsl::followed_by(LEXY_LIT("abc"), dsl::ascii::alpha);
    CHECK(equivalent_rules(rule, dsl::not_followed_by(LEXY_LIT("abc"), -dsl::ascii::alpha)));
}

