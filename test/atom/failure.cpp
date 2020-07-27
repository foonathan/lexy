// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/failure.hpp>

#include "verify.hpp"

TEST_CASE("atom: failure")
{
    struct my_tag
    {};

    constexpr auto atom = lexy::dsl::failure<my_tag>;
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.position() == empty.input);

    constexpr auto non_empty = atom_matches(atom, "abc");
    CHECK(!non_empty);
    CHECK(non_empty.count == 0);
    CHECK(non_empty.error.position() == non_empty.input);
}

