// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/capture.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/recover.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("dsl::capture()")
{
    SUBCASE("as rule")
    {
        constexpr auto rule = dsl::capture(LEXY_LIT("abc"));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char* begin, lexy::string_lexeme<> lex) {
            CHECK(lex.begin() == begin);
            CHECK(lex.size() == 3);
            CHECK(lex[0] == 'a');
            CHECK(lex[1] == 'b');
            CHECK(lex[2] == 'c');

            return 0;
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("abc"));

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto abc_ws = LEXY_VERIFY_P(production, "abc...");
        CHECK(abc_ws.status == test_result::success);
        CHECK(abc_ws.value == 0);
        CHECK(abc_ws.trace == test_trace().literal("abc").whitespace("..."));
    }

    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(dsl::capture(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = lexy::callback<int>([](const char*) { return 0; },
                                  [](const char* begin, lexy::string_lexeme<> lex) {
                                      CHECK(lex.begin() == begin);
                                      CHECK(lex.size() == 3);
                                      CHECK(lex[0] == 'a');
                                      CHECK(lex[1] == 'b');
                                      CHECK(lex[2] == 'c');

                                      return 1;
                                  });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc"));

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto abc_ws = LEXY_VERIFY_P(production, "abc...");
        CHECK(abc_ws.status == test_result::success);
        CHECK(abc_ws.value == 1);
        CHECK(abc_ws.trace == test_trace().literal("abc").whitespace("..."));
    }
}

