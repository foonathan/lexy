// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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

TEST_CASE("dsl::capture_token()")
{
    SUBCASE("as rule")
    {
        constexpr auto rule = dsl::capture_token(LEXY_LIT("abc"));
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
        constexpr auto rule = dsl::if_(dsl::capture_token(LEXY_LIT("abc")));
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

TEST_CASE("dsl::capture()")
{
    SUBCASE("token")
    {
        constexpr auto rule = dsl::capture(LEXY_LIT("abc"));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char* begin, lexy::string_lexeme<> lex) {
            CHECK(lex.begin() == begin);
            CHECK(lex.size() >= 3);
            CHECK(lex[0] == 'a');
            CHECK(lex[1] == 'b');
            CHECK(lex[2] == 'c');

            if (lex.size() > 3)
                for (auto i = 3u; i < lex.size(); ++i)
                    CHECK(lex[i] == '.');

            return int(lex.size() - 3);
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
        CHECK(abc_ws.value == 3);
        CHECK(abc_ws.trace == test_trace().literal("abc").whitespace("..."));
    }
    SUBCASE("rule")
    {
        constexpr auto rule
            = dsl::capture(LEXY_LIT("a") + dsl::position + dsl::try_(LEXY_LIT("bc")));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback
            = [](const char* begin, lexy::string_lexeme<> lex, const char* pos) {
                  CHECK(lex.begin() == begin);
                  CHECK(lex.size() >= 1);
                  CHECK(lex[0] == 'a');
                  if (lex.size() >= 2)
                      CHECK(lex[1] == 'b');
                  if (lex.size() >= 3)
                      CHECK(lex[2] == 'c');

                  CHECK(pos == begin + 1);

                  return 0;
              };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.value == 0);
        CHECK(
            ab.trace
            == test_trace().literal("a").position().error_token("b").expected_literal(1, "bc", 1));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("a").position().literal("bc"));
    }
    SUBCASE("directly nested")
    {
        constexpr auto rule = dsl::capture(dsl::capture(LEXY_LIT("abc")));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char* begin, lexy::string_lexeme<> lex_outer,
                                     lexy::string_lexeme<> lex_inner) {
            CHECK(lex_outer.begin() == begin);
            CHECK(lex_outer.size() == 3);
            CHECK(lex_outer[0] == 'a');
            CHECK(lex_outer[1] == 'b');
            CHECK(lex_outer[2] == 'c');

            CHECK(lex_inner.begin() == lex_outer.begin());
            CHECK(lex_inner.end() == lex_outer.end());

            return 0;
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("abc"));
    }
    SUBCASE("indirectly nested")
    {
        constexpr auto rule
            = dsl::capture(LEXY_LIT("a") + dsl::capture(LEXY_LIT("b")) + LEXY_LIT("c"));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = [](const char* begin, lexy::string_lexeme<> lex_outer,
                                     lexy::string_lexeme<> lex_inner) {
            CHECK(lex_outer.begin() == begin);
            CHECK(lex_outer.size() == 3);
            CHECK(lex_outer[0] == 'a');
            CHECK(lex_outer[1] == 'b');
            CHECK(lex_outer[2] == 'c');

            CHECK(lex_inner.begin() == lex_outer.begin() + 1);
            CHECK(lex_inner.end() == lex_outer.end() - 1);

            return 0;
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("a").literal("b").literal("c"));
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
    }
}

