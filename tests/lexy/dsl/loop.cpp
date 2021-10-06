// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/loop.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/recover.hpp>

TEST_CASE("dsl::loop()")
{
    constexpr auto rule
        = dsl::loop(LEXY_LIT("a") >> dsl::try_(LEXY_LIT("bc")) | LEXY_LIT("!") >> dsl::break_);
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "exhausted choice").cancel());

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("!"));
    auto one = LEXY_VERIFY("abc!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().token("a").token("bc").token("!"));
    auto two = LEXY_VERIFY("abcabc!");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().token("a").token("bc").token("a").token("bc").token("!"));

    auto recover = LEXY_VERIFY("aabc!");
    auto recover_trace
        = test_trace().token("a").expected_literal(1, "bc", 0).token("a").token("bc").token("!");
    CHECK(recover.status == test_result::recovered_error);
    CHECK(recover.trace == recover_trace);

    auto unterminated       = LEXY_VERIFY("abcabc");
    auto unterminated_trace = test_trace()
                                  .token("a")
                                  .token("bc")
                                  .token("a")
                                  .token("bc")
                                  .error(6, 6, "exhausted choice")
                                  .cancel();
    CHECK(unterminated.status == test_result::fatal_error);
    CHECK(unterminated.trace == unterminated_trace);
}

TEST_CASE("dsl::while_()")
{
    constexpr auto rule = dsl::while_(LEXY_LIT("a") >> dsl::try_(LEXY_LIT("bc")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace());

    auto one = LEXY_VERIFY("abc");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().token("a").token("bc"));
    auto two = LEXY_VERIFY("abcabc");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().token("a").token("bc").token("a").token("bc"));
    auto three = LEXY_VERIFY("abcabcabc");
    CHECK(three.status == test_result::success);
    CHECK(three.trace
          == test_trace().token("a").token("bc").token("a").token("bc").token("a").token("bc"));

    auto recovered = LEXY_VERIFY("aabc");
    CHECK(recovered.status == test_result::recovered_error);
    CHECK(recovered.trace
          == test_trace().token("a").expected_literal(1, "bc", 0).token("a").token("bc"));
}

TEST_CASE("dsl::while_one()")
{
    constexpr auto rule = dsl::while_one(LEXY_LIT("a") >> LEXY_LIT("bc"));
    CHECK(lexy::is_branch_rule<decltype(rule)>);

    constexpr auto equivalent
        = LEXY_LIT("a") >> LEXY_LIT("bc") + dsl::while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
    CHECK(equivalent_rules(rule, equivalent));
}

TEST_CASE("dsl::do_while()")
{
    SUBCASE("branch")
    {
        constexpr auto rule = dsl::do_while(LEXY_LIT("bc"), LEXY_LIT("a"));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto equivalent = LEXY_LIT("bc") >> dsl::while_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(equivalent_rules(rule, equivalent));
    }
    SUBCASE("non-branch")
    {
        constexpr auto rule = dsl::do_while(dsl::while_(LEXY_LIT("bc")), LEXY_LIT("a"));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto equivalent = dsl::while_(LEXY_LIT("bc"))
                                    + dsl::while_(LEXY_LIT("a") >> dsl::while_(LEXY_LIT("bc")));
        CHECK(equivalent_rules(rule, equivalent));
    }
}

