// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/null_input.hpp>

#include <doctest/doctest.h>

TEST_CASE("null_input")
{
    lexy::null_input<> input;

    auto reader = input.reader();
    CHECK(reader.cur() == nullptr);
    CHECK(reader.peek() == lexy::default_encoding::eof());
    CHECK(reader.eof());
}

