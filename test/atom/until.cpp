// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/until.hpp>

#include "verify.hpp"
#include <lexy/dsl/literal.hpp>

TEST_CASE("atom: until")
{
    constexpr auto atom = until(LEXY_LIT("!"));
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.message() == "missing until condition");
    CHECK(empty.error.begin() == empty.input);
    CHECK(empty.error.end() == lexy::_detail::string_view(empty.input).end());

    constexpr auto zero = atom_matches(atom, "!");
    CHECK(zero);
    CHECK(zero.count == 1);
    constexpr auto one = atom_matches(atom, "a!");
    CHECK(one);
    CHECK(one.count == 2);
    constexpr auto two = atom_matches(atom, "xy!");
    CHECK(two);
    CHECK(two.count == 3);

    constexpr auto unterminated = atom_matches(atom, "abc");
    CHECK(!unterminated);
    CHECK(unterminated.count == 3);
    CHECK(unterminated.error.message() == "missing until condition");
    CHECK(unterminated.error.begin() == unterminated.input);
    CHECK(unterminated.error.end() == lexy::_detail::string_view(unterminated.input).end());
}

TEST_CASE("atom: until eof")
{
    constexpr auto atom = until(LEXY_LIT("!")).or_eof();
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(empty);
    CHECK(empty.count == 0);

    constexpr auto zero = atom_matches(atom, "!");
    CHECK(zero);
    CHECK(zero.count == 1);
    constexpr auto one = atom_matches(atom, "a!");
    CHECK(one);
    CHECK(one.count == 2);
    constexpr auto two = atom_matches(atom, "xy!");
    CHECK(two);
    CHECK(two.count == 3);

    constexpr auto unterminated = atom_matches(atom, "abc");
    CHECK(unterminated);
    CHECK(unterminated.count == 3);
}

