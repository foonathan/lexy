// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/identifier.hpp>

#include "verify.hpp"
#include <cctype>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("dsl::identifier(leading, trailing).pattern()")
{
    constexpr auto rule = dsl::identifier(dsl::ascii::upper, dsl::ascii::lower).pattern();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::identifier(dsl::ascii::upper, dsl::ascii::lower)
                                     .reserve(LEXY_LIT("Abc"))
                                     .pattern()));

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.upper").cancel());

    auto A = LEXY_VERIFY("A");
    CHECK(A.status == test_result::success);
    CHECK(A.trace == test_trace().token("identifier", "A"));

    auto Abc = LEXY_VERIFY("Abc");
    CHECK(Abc.status == test_result::success);
    CHECK(Abc.trace == test_trace().token("identifier", "Abc"));

    auto Abc123 = LEXY_VERIFY("Abc123");
    CHECK(Abc123.status == test_result::success);
    CHECK(Abc123.trace == test_trace().token("identifier", "Abc"));
}

TEST_CASE("dsl::identifier(leading, trailing)")
{
    constexpr auto id = dsl::identifier(dsl::ascii::upper, dsl::ascii::lower);
    CHECK(lexy::is_branch_rule<decltype(id)>);

    CHECK(equivalent_rules(id.leading_pattern(), dsl::ascii::upper));
    CHECK(equivalent_rules(id.trailing_pattern(), dsl::ascii::lower));

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char* begin, lexy::string_lexeme<> lex) {
                                  CHECK(lex.begin() == begin);
                                  CHECK(lex.size() >= 1);

                                  CHECK(std::isupper(lex[0]));
                                  for (auto iter = lex.begin(); ++iter != lex.end();)
                                      CHECK(std::islower(*iter));

                                  return 1;
                              });

    SUBCASE("basic")
    {
        constexpr auto rule = id;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.upper").cancel());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace == test_trace().token("identifier", "Ab"));
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 1);
        CHECK(Abc.trace == test_trace().token("identifier", "Abc"));
        auto Abc123 = LEXY_VERIFY("Abc123");
        CHECK(Abc123.status == test_result::success);
        CHECK(Abc123.value == 1);
        CHECK(Abc123.trace == test_trace().token("identifier", "Abc"));
    }
    SUBCASE("with whitespace")
    {
        struct production : test_production_for<decltype(id)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.upper").cancel());

        auto A = LEXY_VERIFY_P(production, "A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto Ab = LEXY_VERIFY_P(production, "Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace == test_trace().token("identifier", "Ab"));
        auto Abc = LEXY_VERIFY_P(production, "Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 1);
        CHECK(Abc.trace == test_trace().token("identifier", "Abc"));
        auto Abc123 = LEXY_VERIFY_P(production, "Abc123");
        CHECK(Abc123.status == test_result::success);
        CHECK(Abc123.value == 1);
        CHECK(Abc123.trace == test_trace().token("identifier", "Abc"));

        auto whitespace = LEXY_VERIFY_P(production, "Abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 1);
        CHECK(whitespace.trace == test_trace().token("identifier", "Abc").whitespace("..."));
    }

    SUBCASE(".reserve()")
    {
        constexpr auto rule
            = id.reserve(LEXY_LIT("Ab"), LEXY_KEYWORD("Abc", id)).reserve(LEXY_LIT("Int"));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.upper").cancel());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto Abcd = LEXY_VERIFY("Abcd");
        CHECK(Abcd.status == test_result::success);
        CHECK(Abcd.value == 1);
        CHECK(Abcd.trace == test_trace().token("identifier", "Abcd"));
        auto Abcd123 = LEXY_VERIFY("Abcd123");
        CHECK(Abcd123.status == test_result::success);
        CHECK(Abcd123.value == 1);
        CHECK(Abcd123.trace == test_trace().token("identifier", "Abcd"));

        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::recovered_error);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace
              == test_trace().token("identifier", "Ab").error(0, 2, "reserved identifier"));
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::recovered_error);
        CHECK(Abc.value == 1);
        CHECK(Abc.trace
              == test_trace().token("identifier", "Abc").error(0, 3, "reserved identifier"));
        auto Int = LEXY_VERIFY("Int");
        CHECK(Int.status == test_result::recovered_error);
        CHECK(Int.value == 1);
        CHECK(Int.trace
              == test_trace().token("identifier", "Int").error(0, 3, "reserved identifier"));
    }
    SUBCASE(".reserve_prefix()")
    {
        constexpr auto rule = id.reserve_prefix(LEXY_LIT("Ab"));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.upper").cancel());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto B = LEXY_VERIFY("B");
        CHECK(B.status == test_result::success);
        CHECK(B.value == 1);
        CHECK(B.trace == test_trace().token("identifier", "B"));

        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::recovered_error);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace
              == test_trace().token("identifier", "Ab").error(0, 2, "reserved identifier"));
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::recovered_error);
        CHECK(Abc.value == 1);
        CHECK(Abc.trace
              == test_trace().token("identifier", "Abc").error(0, 3, "reserved identifier"));
        auto Abcd = LEXY_VERIFY("Abcd");
        CHECK(Abcd.status == test_result::recovered_error);
        CHECK(Abcd.value == 1);
        CHECK(Abcd.trace
              == test_trace().token("identifier", "Abcd").error(0, 4, "reserved identifier"));
    }
    SUBCASE(".reserve_containing()")
    {
        constexpr auto rule = id.reserve_containing(LEXY_LIT("b"));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.upper").cancel());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto Acd = LEXY_VERIFY("Acd");
        CHECK(Acd.status == test_result::success);
        CHECK(Acd.value == 1);
        CHECK(Acd.trace == test_trace().token("identifier", "Acd"));

        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::recovered_error);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace
              == test_trace().token("identifier", "Ab").error(0, 2, "reserved identifier"));
        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::recovered_error);
        CHECK(Abc.value == 1);
        CHECK(Abc.trace
              == test_trace().token("identifier", "Abc").error(0, 3, "reserved identifier"));
        auto Abcd = LEXY_VERIFY("Abcd");
        CHECK(Abcd.status == test_result::recovered_error);
        CHECK(Abcd.value == 1);
        CHECK(Abcd.trace
              == test_trace().token("identifier", "Abcd").error(0, 4, "reserved identifier"));
    }

    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(id.reserve(LEXY_LIT("Abc")));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::success);
        CHECK(A.value == 1);
        CHECK(A.trace == test_trace().token("identifier", "A"));
        auto Ab = LEXY_VERIFY("Ab");
        CHECK(Ab.status == test_result::success);
        CHECK(Ab.value == 1);
        CHECK(Ab.trace == test_trace().token("identifier", "Ab"));

        auto Abc = LEXY_VERIFY("Abc");
        CHECK(Abc.status == test_result::success);
        CHECK(Abc.value == 0);
        CHECK(Abc.trace == test_trace());
    }
}

TEST_CASE("dsl::identifier(pattern)")
{
    constexpr auto rule = dsl::identifier(dsl::ascii::alpha);
    CHECK(lexy::is_branch_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::identifier(dsl::ascii::alpha, dsl::ascii::alpha)));
}

TEST_CASE("dsl::keyword")
{
    constexpr auto id = dsl::identifier(dsl::ascii::alpha).reserve(LEXY_LIT("foo"));

    constexpr auto rule = LEXY_KEYWORD("Int", id);
    CHECK(lexy::is_token_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(equivalent_rules(rule, dsl::keyword<"Int">(id)));
#endif

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_keyword(0, 0, "Int").cancel());
    auto I = LEXY_VERIFY("I");
    CHECK(I.status == test_result::fatal_error);
    CHECK(I.trace == test_trace().expected_keyword(0, 1, "Int").cancel());
    auto In = LEXY_VERIFY("In");
    CHECK(In.status == test_result::fatal_error);
    CHECK(In.trace == test_trace().expected_keyword(0, 2, "Int").cancel());

    auto Int = LEXY_VERIFY("Int");
    CHECK(Int.status == test_result::success);
    CHECK(Int.trace == test_trace().token("Int"));

    auto Integer = LEXY_VERIFY("Integer");
    CHECK(Integer.status == test_result::fatal_error);
    CHECK(Integer.trace == test_trace().expected_keyword(0, 7, "Int").error_token("Int").cancel());
}

