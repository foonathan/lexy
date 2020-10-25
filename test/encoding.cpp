// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/encoding.hpp>

#include <doctest.h>

TEST_CASE("wchar_t encoding")
{
    if constexpr (sizeof(wchar_t) == sizeof(char16_t))
        CHECK(!!lexy::utf16_encoding::is_secondary_char_type<wchar_t>);
    else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
        CHECK(!!lexy::utf32_encoding::is_secondary_char_type<wchar_t>);
}

