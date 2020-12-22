// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/eof.hpp>

#include "verify.hpp"

TEST_CASE("dsl::eof")
{
    SUBCASE("basic")
    {
        constexpr auto atom = lexy::dsl::eof;

        constexpr auto empty = atom_matches(atom, "");
        CHECK(empty);
        CHECK(empty.count == 0);

        constexpr auto non_empty = atom_matches(atom, "abc");
        CHECK(!non_empty);
        CHECK(non_empty.count == 0);
        // CHECK(non_empty.error.position() == non_empty.input);
        // CHECK(non_empty.error.character_class() == "EOF");
    }
    SUBCASE("whitespace")
    {
        constexpr auto result     = lexy::dsl::eof[LEXY_LIT(" ")];
        constexpr auto equivalent = whitespaced(lexy::dsl::eof, LEXY_LIT(" "));
        CHECK(std::is_same_v<decltype(result), decltype(equivalent)>);
    }
}

