// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/argv_input.hpp>

#include "verify.hpp"

TEST_CASE("atom: argv_separator")
{
    SUBCASE("non-argv_input")
    {
        constexpr auto atom = lexy::dsl::argv_separator;
        CHECK(lexy::is_atom<decltype(atom)>);

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.error.position() == empty.input);
        CHECK(empty.error.character_class() == "argv-separator");

        constexpr auto non_empty = atom_matches(atom, "abc");
        CHECK(!non_empty);
        CHECK(non_empty.count == 0);
        CHECK(non_empty.error.position() == non_empty.input);
        CHECK(non_empty.error.character_class() == "argv-separator");
    }
    SUBCASE("argv_input")
    {
        char program[] = "IGNORED";
        char first[]   = "abc";
        char second[]  = "de";
        char third[]   = "f";

        char* argv[] = {program, first, second, third, nullptr};
        int   argc   = 4;

        lexy::argv_input input(argc, argv);

        CHECK(!lexy::dsl::argv_separator.match(input));
        CHECK(input.peek() == 'a');
        input.bump();

        CHECK(!lexy::dsl::argv_separator.match(input));
        CHECK(input.peek() == 'b');
        input.bump();

        CHECK(!lexy::dsl::argv_separator.match(input));
        CHECK(input.peek() == 'c');
        input.bump();

        CHECK(lexy::dsl::argv_separator.match(input));
        CHECK(input.peek() == 'd');
    }
}

