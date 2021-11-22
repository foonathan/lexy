// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/lookahead.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/until.hpp>

TEST_CASE("dsl::lookahead()")
{
    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    constexpr auto condition = dsl::lookahead(LEXY_LIT("."), LEXY_LIT("!"));
    CHECK(lexy::is_branch_rule<decltype(condition)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = condition;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace == test_trace().error(0, 0, "lookahead failure"));

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing.status == test_result::recovered_error);
        CHECK(nothing.trace == test_trace().backtracked("abc").error(0, 0, "lookahead failure"));
        auto nothing_limit = LEXY_VERIFY("abc!def");
        CHECK(nothing_limit.status == test_result::recovered_error);
        CHECK(nothing_limit.trace
              == test_trace().backtracked("abc!").error(0, 0, "lookahead failure"));

        auto something = LEXY_VERIFY("abc.");
        CHECK(something.status == test_result::success);
        CHECK(something.trace == test_trace().backtracked("abc."));
        auto something_limit = LEXY_VERIFY("abc.def!ghi");
        CHECK(something_limit.status == test_result::success);
        CHECK(something_limit.trace == test_trace().backtracked("abc."));

        auto limit_something = LEXY_VERIFY("abc!def.");
        CHECK(limit_something.status == test_result::recovered_error);
        CHECK(limit_something.trace
              == test_trace().backtracked("abc!").error(0, 0, "lookahead failure"));
    }
    SUBCASE("as rule with .error")
    {
        constexpr auto rule = condition.error<my_error>;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error"));

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing.status == test_result::recovered_error);
        CHECK(nothing.trace == test_trace().backtracked("abc").error(0, 0, "my error"));
        auto nothing_limit = LEXY_VERIFY("abc!def");
        CHECK(nothing_limit.status == test_result::recovered_error);
        CHECK(nothing_limit.trace == test_trace().backtracked("abc!").error(0, 0, "my error"));

        auto something = LEXY_VERIFY("abc.");
        CHECK(something.status == test_result::success);
        CHECK(something.trace == test_trace().backtracked("abc."));
        auto something_limit = LEXY_VERIFY("abc.def!ghi");
        CHECK(something_limit.status == test_result::success);
        CHECK(something_limit.trace == test_trace().backtracked("abc."));

        auto limit_something = LEXY_VERIFY("abc!def.");
        CHECK(limit_something.status == test_result::recovered_error);
        CHECK(limit_something.trace == test_trace().backtracked("abc!").error(0, 0, "my error"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(condition >> dsl::until(LEXY_LIT(".")));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing.status == test_result::success);
        CHECK(nothing.trace == test_trace().backtracked("abc"));
        auto nothing_limit = LEXY_VERIFY("abc!def");
        CHECK(nothing_limit.status == test_result::success);
        CHECK(nothing_limit.trace == test_trace().backtracked("abc!"));

        auto something = LEXY_VERIFY("abc.");
        CHECK(something.status == test_result::success);
        CHECK(something.trace == test_trace().backtracked("abc.").token("any", "abc."));
        auto something_limit = LEXY_VERIFY("abc.def!ghi");
        CHECK(something_limit.status == test_result::success);
        CHECK(something_limit.trace == test_trace().backtracked("abc.").token("any", "abc."));

        auto limit_something = LEXY_VERIFY("abc!def.");
        CHECK(limit_something.status == test_result::success);
        CHECK(limit_something.trace == test_trace().backtracked("abc!"));
    }
}

