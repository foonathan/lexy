// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/base.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("partial_reader()")
{
    auto input = lexy::zstring_input("abc");
    auto end   = input.end() - 1;

    auto partial = lexy::partial_reader(input.reader(), end);
    CHECK(partial.cur() == input.begin());
    CHECK(partial.peek() == 'a');
    CHECK(!partial.eof());

    partial.bump();
    CHECK(partial.peek() == 'b');
    CHECK(!partial.eof());

    partial.bump();
    CHECK(partial.peek() == lexy::default_encoding::eof());
    CHECK(partial.eof());
}

