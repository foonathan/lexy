// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/punctuator.hpp>

#include <doctest/doctest.h>

TEST_CASE("dsl punctuators")
{
    constexpr auto period    = LEXY_LIT(".");
    constexpr auto comma     = LEXY_LIT(",");
    constexpr auto colon     = LEXY_LIT(":");
    constexpr auto semicolon = LEXY_LIT(";");

    CHECK(std::is_same_v<const decltype(period), decltype(lexy::dsl::period)>);
    CHECK(std::is_same_v<const decltype(comma), decltype(lexy::dsl::comma)>);
    CHECK(std::is_same_v<const decltype(colon), decltype(lexy::dsl::colon)>);
    CHECK(std::is_same_v<const decltype(semicolon), decltype(lexy::dsl::semicolon)>);

    constexpr auto hyphen     = LEXY_LIT("-");
    constexpr auto slash      = LEXY_LIT("/");
    constexpr auto backslash  = LEXY_LIT("\\");
    constexpr auto apostrophe = LEXY_LIT("'");

    CHECK(std::is_same_v<const decltype(hyphen), decltype(lexy::dsl::hyphen)>);
    CHECK(std::is_same_v<const decltype(slash), decltype(lexy::dsl::slash)>);
    CHECK(std::is_same_v<const decltype(backslash), decltype(lexy::dsl::backslash)>);
    CHECK(std::is_same_v<const decltype(apostrophe), decltype(lexy::dsl::apostrophe)>);

    constexpr auto hash_sign   = LEXY_LIT("#");
    constexpr auto dollar_sign = LEXY_LIT("$");
    constexpr auto at_sign     = LEXY_LIT("@");

    CHECK(std::is_same_v<const decltype(hash_sign), decltype(lexy::dsl::hash_sign)>);
    CHECK(std::is_same_v<const decltype(dollar_sign), decltype(lexy::dsl::dollar_sign)>);
    CHECK(std::is_same_v<const decltype(at_sign), decltype(lexy::dsl::at_sign)>);
}
