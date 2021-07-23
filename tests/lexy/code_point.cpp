// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/code_point.hpp>

#include <cctype>
#include <doctest/doctest.h>

TEST_CASE("code_point")
{
    // Note: we're only testing the BMP.

    for (auto i = 0u; i <= 0x7F; ++i)
    {
        lexy::code_point cp(i);
        CHECK(cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(cp.is_control() == !!std::iscntrl(static_cast<int>(cp.value())));
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());

        CHECK(cp.is_scalar());
    }
    for (auto i = 0x80u; i <= 0x9F; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());

        CHECK(cp.is_scalar());
    }
    for (auto i = 0xA0u; i <= 0xFF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());

        CHECK(cp.is_scalar());
    }

    // 0x0100 - 0xD7FF are normal

    for (auto i = 0xD800u; i <= 0xDFFF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());

        CHECK(!cp.is_scalar());
    }
    for (auto i = 0xE000u; i <= 0xF8FF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(cp.is_private_use());
        CHECK(!cp.is_noncharacter());

        CHECK(cp.is_scalar());
    }

    // 0xF900 - 0xFDCF are normal

    for (auto i = 0xFDD0u; i <= 0xFDEF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(cp.is_noncharacter());

        CHECK(cp.is_scalar());
    }

    // 0xFDF0 - 0xFFFD are normal

    for (auto i = 0xFFFEu; i <= 0xFFFF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(cp.is_noncharacter());

        CHECK(cp.is_scalar());
    }
}

