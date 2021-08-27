// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/lexeme.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("lexeme")
{
    lexy::string_input input  = lexy::zstring_input("abc");
    auto               reader = input.reader();

    auto begin = reader.position();
    reader.bump();
    reader.bump();
    reader.bump();

    lexy::lexeme lexeme(reader, begin);
    CHECK(lexeme.begin() == begin);
    CHECK(lexeme.end() == reader.position());
    CHECK(lexeme.size() == 3);
    CHECK(lexeme.data() == begin);
    CHECK(lexeme[0] == 'a');
}

