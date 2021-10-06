// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/error.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/if.hpp>

namespace
{
struct my_error
{
    static constexpr auto name()
    {
        return "my error";
    }
};
} // namespace

TEST_CASE("dsl::error")
{
    constexpr auto err = dsl::error<my_error>;
    CHECK(lexy::is_unconditional_branch_rule<decltype(err)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = err;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().error(0, 0, "my error").cancel());
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = err | LEXY_LIT("abc");

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().error(0, 0, "my error").cancel());
    }
}

TEST_CASE("dsl::error(rule)")
{
    constexpr auto err = dsl::error<my_error>(LEXY_LIT("abc"));
    CHECK(lexy::is_unconditional_branch_rule<decltype(err)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = err;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().error(0, 1, "my error").cancel());
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().error(0, 2, "my error").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().error(0, 3, "my error").cancel());
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = err | LEXY_LIT("123");

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().error(0, 1, "my error").cancel());
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().error(0, 2, "my error").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().error(0, 3, "my error").cancel());
    }
}

TEST_CASE("dsl::must().error")
{
    constexpr auto must = dsl::must(LEXY_LIT("a") | LEXY_LIT("b") | LEXY_LIT("c")).error<my_error>;
    CHECK(lexy::is_branch_rule<decltype(must)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = must;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().token("a"));
        auto b = LEXY_VERIFY("b");
        CHECK(b.status == test_result::success);
        CHECK(b.trace == test_trace().token("b"));
        auto c = LEXY_VERIFY("c");
        CHECK(c.status == test_result::success);
        CHECK(c.trace == test_trace().token("c"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(must);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().token("a"));
        auto b = LEXY_VERIFY("b");
        CHECK(b.status == test_result::success);
        CHECK(b.trace == test_trace().token("b"));
        auto c = LEXY_VERIFY("c");
        CHECK(c.status == test_result::success);
        CHECK(c.trace == test_trace().token("c"));
    }
}

TEST_CASE("dsl::must().error(rule)")
{
    constexpr auto must
        = dsl::must(LEXY_LIT("a") | LEXY_LIT("b") | LEXY_LIT("c")).error<my_error>(LEXY_LIT("123"));
    CHECK(lexy::is_branch_rule<decltype(must)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = must;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto with_range = LEXY_VERIFY("123");
        CHECK(with_range.status == test_result::fatal_error);
        CHECK(with_range.trace == test_trace().error(0, 3, "my error").cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().token("a"));
        auto b = LEXY_VERIFY("b");
        CHECK(b.status == test_result::success);
        CHECK(b.trace == test_trace().token("b"));
        auto c = LEXY_VERIFY("c");
        CHECK(c.status == test_result::success);
        CHECK(c.trace == test_trace().token("c"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(must);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto with_range = LEXY_VERIFY("123");
        CHECK(with_range.status == test_result::success);
        CHECK(with_range.trace == test_trace());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().token("a"));
        auto b = LEXY_VERIFY("b");
        CHECK(b.status == test_result::success);
        CHECK(b.trace == test_trace().token("b"));
        auto c = LEXY_VERIFY("c");
        CHECK(c.status == test_result::success);
        CHECK(c.trace == test_trace().token("c"));
    }
}

