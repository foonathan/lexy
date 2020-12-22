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

        constexpr auto empty = atom_matches(atom, "");
        CHECK(!empty);
        CHECK(empty.count == 0);
        // CHECK(empty.error.position() == empty.input);
        // CHECK(empty.error.character_class() == "argv-separator");

        constexpr auto non_empty = atom_matches(atom, "abc");
        CHECK(!non_empty);
        CHECK(non_empty.count == 0);
        // CHECK(non_empty.error.position() == non_empty.input);
        // CHECK(non_empty.error.character_class() == "argv-separator");
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
        auto             reader = input.reader();

        using engine = decltype(lexy::dsl::argv_separator)::token_engine;
        CHECK(!lexy::engine_try_match<engine>(reader));
        CHECK(reader.peek() == 'a');
        reader.bump();

        CHECK(!lexy::engine_try_match<engine>(reader));
        CHECK(reader.peek() == 'b');
        reader.bump();

        CHECK(!lexy::engine_try_match<engine>(reader));
        CHECK(reader.peek() == 'c');
        reader.bump();

        CHECK(lexy::engine_try_match<engine>(reader));
        CHECK(reader.peek() == 'd');
    }
}

