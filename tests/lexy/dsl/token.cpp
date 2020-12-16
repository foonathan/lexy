// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/token.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/value.hpp>

TEST_CASE("dsl::token")
{
    constexpr auto atom = token(list(LEXY_LIT("abc") >> lexy::dsl::value_c<0>));
    CHECK(lexy::is_pattern<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    // CHECK(empty.error.message() == "no match");
    // CHECK(empty.error.position() == empty.input);

    constexpr auto one = atom_matches(atom, "abc");
    CHECK(one);
    CHECK(one.count == 3);
    constexpr auto two = atom_matches(atom, "abcabc");
    CHECK(two);
    CHECK(two.count == 6);
    constexpr auto three = atom_matches(atom, "abcabcabc");
    CHECK(three);
    CHECK(three.count == 9);
}

