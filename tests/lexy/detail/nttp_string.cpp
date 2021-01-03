// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/_detail/nttp_string.hpp>

#include <doctest/doctest.h>

TEST_CASE("LEXY_NTTP_STRING")
{
    using string = LEXY_NTTP_STRING("abc");
    CHECK(std::is_same_v<typename string::char_type, char>);
    CHECK(string::get() == "abc");
    CHECK(string::template get<wchar_t>() == L"abc");

    using wstring = LEXY_NTTP_STRING(L"abc");
    CHECK(std::is_same_v<typename wstring::char_type, wchar_t>);
    CHECK(wstring::get() == L"abc");
    CHECK(wstring::template get<char>() == "abc");

#if LEXY_HAS_NTTP
    using lit_string = lexy::_detail::type_string<lexy::_detail::string_literal("abc")>;
    CHECK(lit_string::get() == "abc");
#endif
}

