// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/parse.hpp>

#include <doctest.h>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/while.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
//=== AST ===//
struct string_pair
{
    lexy::string_lexeme<> a;
    lexy::string_lexeme<> b;
};

//=== grammar ===//
struct string_p
{
    static constexpr auto rule = capture(while_(lexy::dsl::ascii::alnum));

    static constexpr auto value
        = lexy::callback<lexy::string_lexeme<>>([](lexy::string_lexeme<> lex) { return lex; });
};

struct string_pair_p
{
    static constexpr auto rule = LEXY_LIT("(") + lexy::dsl::p<string_p> + LEXY_LIT(",")
                                 + lexy::dsl::p<string_p> + LEXY_LIT(")");

    static constexpr auto value
        = lexy::callback<string_pair>([](lexy::string_lexeme<> a, lexy::string_lexeme<> b) {
              return string_pair{a, b};
          });
};

using prod = string_pair_p;
} // namespace

TEST_CASE("parse")
{
    constexpr auto empty = lexy::parse<prod>(lexy::zstring_input(""));
    CHECK(!empty);

    constexpr auto abc_abc = lexy::parse<prod>(lexy::zstring_input("(abc,abc)"));
    CHECK(abc_abc);
    CHECK(abc_abc.value().a.string_view() == "abc");
    CHECK(abc_abc.value().b.string_view() == "abc");

    constexpr auto abc_123 = lexy::parse<prod>(lexy::zstring_input("(abc,123)"));
    CHECK(abc_123);
    CHECK(abc_123.value().a.string_view() == "abc");
    CHECK(abc_123.value().b.string_view() == "123");
}

