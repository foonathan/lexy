// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/error.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("error_context")
{
    struct production
    {};

    auto input   = lexy::zstring_input("abc");
    auto context = lexy::error_context(production{}, input, input.begin());
    CHECK(&context.input() == &input);
    CHECK(context.production() == "production");
    CHECK(context.position() == input.begin());
}

