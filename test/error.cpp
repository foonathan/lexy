// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/error.hpp>

#include <doctest.h>

TEST_CASE("production_name")
{
    // Simple test, _detail::type_name does all the heavy lifting.
    struct production
    {};

    constexpr auto name = lexy::production_name(production{});
    CHECK(name == "production");
}

