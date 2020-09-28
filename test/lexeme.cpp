// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/lexeme.hpp>

#include <doctest.h>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("lexeme")
{
    lexy::string_input input  = lexy::zstring_input("abc");
    auto               reader = input.reader();

    auto begin = reader.cur();
    reader.bump();
    reader.bump();
    reader.bump();

    lexy::lexeme lexeme(reader, begin);
    CHECK(lexeme.begin() == begin);
    CHECK(lexeme.end() == reader.cur());
    CHECK(lexeme.size() == 3);
    CHECK(lexeme.data() == begin);
    CHECK(lexeme[0] == 'a');
}

TEST_CASE("as_string")
{
    lexy::string_input input  = lexy::zstring_input("abc");
    auto               reader = input.reader();

    auto begin = reader.cur();
    reader.bump();
    reader.bump();
    reader.bump();

    lexy::lexeme lexeme(reader, begin);
    std::string  result = lexy::as_string<std::string>(lexeme);
    CHECK(result == "abc");
}

