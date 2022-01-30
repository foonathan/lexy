// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/error.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("error_context")
{
    struct production
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "production";
        }
    };

    auto input   = lexy::zstring_input("abc");
    auto context = lexy::error_context(production{}, input, input.data());
    CHECK(&context.input() == &input);
    CHECK(context.production() == lexy::_detail::string_view("production"));
    CHECK(context.position() == input.data());
}

