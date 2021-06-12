// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/punctuator.hpp>

#include <doctest/doctest.h>

namespace
{
template <typename Punct, typename Lit>
constexpr bool is_punct(Punct, Lit)
{
    return std::is_base_of_v<Lit, Punct>;
}
} // namespace

TEST_CASE("dsl punctuators")
{
    CHECK(is_punct(lexy::dsl::period, LEXY_LIT(".")));
    CHECK(is_punct(lexy::dsl::comma, LEXY_LIT(",")));
    CHECK(is_punct(lexy::dsl::colon, LEXY_LIT(":")));
    CHECK(is_punct(lexy::dsl::semicolon, LEXY_LIT(";")));

    CHECK(is_punct(lexy::dsl::exclamation_mark, LEXY_LIT("!")));
    CHECK(is_punct(lexy::dsl::question_mark, LEXY_LIT("?")));

    CHECK(is_punct(lexy::dsl::hyphen, LEXY_LIT("-")));
    CHECK(is_punct(lexy::dsl::slash, LEXY_LIT("/")));
    CHECK(is_punct(lexy::dsl::backslash, LEXY_LIT("\\")));
    CHECK(is_punct(lexy::dsl::apostrophe, LEXY_LIT("'")));
    CHECK(is_punct(lexy::dsl::ampersand, LEXY_LIT("&")));
    CHECK(is_punct(lexy::dsl::caret, LEXY_LIT("^")));
    CHECK(is_punct(lexy::dsl::asterisk, LEXY_LIT("*")));
    CHECK(is_punct(lexy::dsl::tilde, LEXY_LIT("~")));

    CHECK(is_punct(lexy::dsl::hash_sign, LEXY_LIT("#")));
    CHECK(is_punct(lexy::dsl::dollar_sign, LEXY_LIT("$")));
    CHECK(is_punct(lexy::dsl::at_sign, LEXY_LIT("@")));
    CHECK(is_punct(lexy::dsl::percent_sign, LEXY_LIT("%")));
    CHECK(is_punct(lexy::dsl::equal_sign, LEXY_LIT("=")));
}

