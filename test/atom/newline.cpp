// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/atom/newline.hpp>

#include "verify.hpp"

TEST_CASE("atom: newline")
{
    constexpr auto atom = lexy::dsl::newline;
    CHECK(lexy::is_atom<decltype(atom)>);

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.position() == empty.input);

    constexpr auto nl = atom_matches(atom, "\n");
    CHECK(nl);
    CHECK(nl.count == 1);

    constexpr auto cr = atom_matches(atom, "\r");
    CHECK(!cr);
    CHECK(cr.count == 1);
    CHECK(cr.error.position() == cr.input);
    constexpr auto cr_nl = atom_matches(atom, "\r\n");
    CHECK(cr_nl);
    CHECK(cr_nl.count == 2);

    constexpr auto extra_cr_nl = atom_matches(atom, "\n\r\n");
    CHECK(extra_cr_nl);
    CHECK(extra_cr_nl.count == 1);
    constexpr auto extra_nl = atom_matches(atom, "\r\n\n");
    CHECK(extra_nl);
    CHECK(extra_nl.count == 2);
}

