// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/lexeme.hpp>

#include <doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("lexeme")
{
    lexy::string_input input = lexy::zstring_input("abc");

    auto begin = input.cur();
    input.bump();
    input.bump();
    input.bump();

    lexy::lexeme lexeme(input, begin);
    CHECK(lexeme.begin() == begin);
    CHECK(lexeme.end() == input.cur());
    CHECK(lexeme.size() == 3);
    CHECK(lexeme.data() == begin);
    CHECK(lexeme[0] == 'a');
}

