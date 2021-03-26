// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/encoding.hpp>

#include <doctest/doctest.h>
#include <lexy/callback.hpp>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("encoding deduction")
{
    auto char_input = lexy::zstring_input("str");
    CHECK(std::is_same_v<decltype(char_input)::encoding, lexy::default_encoding>);

#if LEXY_HAS_CHAR8_T
    auto char8_input = lexy::zstring_input(u8"str");
    CHECK(std::is_same_v<decltype(char8_input)::encoding, lexy::utf8_encoding>);
#endif
    auto char16_input = lexy::zstring_input(u"str");
    CHECK(std::is_same_v<decltype(char16_input)::encoding, lexy::utf16_encoding>);
    auto char32_input = lexy::zstring_input(U"str");
    CHECK(std::is_same_v<decltype(char32_input)::encoding, lexy::utf32_encoding>);

    constexpr unsigned char uchar_str[] = {0};
    auto                    uchar_input = lexy::zstring_input(uchar_str);
    CHECK(std::is_same_v<decltype(uchar_input)::encoding, lexy::raw_encoding>);

    constexpr std::byte byte_str[] = {std::byte(0)};
    auto                byte_input = lexy::zstring_input(byte_str);
    CHECK(std::is_same_v<decltype(byte_input)::encoding, lexy::raw_encoding>);
}

TEST_CASE("wchar_t encoding")
{
    if constexpr (sizeof(wchar_t) == sizeof(char16_t))
        CHECK(!!lexy::utf16_encoding::is_secondary_char_type<wchar_t>);
    else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
        CHECK(!!lexy::utf32_encoding::is_secondary_char_type<wchar_t>);
}

TEST_CASE("encode code point")
{
    auto encode = [](auto encoding, lexy::code_point cp) {
        if constexpr (std::is_same_v<decltype(encoding), lexy::utf8_encoding> && !LEXY_HAS_CHAR8_T)
        {
            using string_type = std::string;
            return lexy::as_string<string_type, decltype(encoding)>(cp);
        }
        else
        {
            using char_type   = typename decltype(encoding)::char_type;
            using string_type = std::basic_string<char_type>;
            return lexy::as_string<string_type, decltype(encoding)>(cp);
        }
    };

    SUBCASE("ASCII")
    {
        // ASCII is always guaranteed to round-trip.
        for (auto c = 0u; c <= 0x7F; ++c)
        {
            auto cp = lexy::code_point(c);
            CHECK(cp.is_valid());
            CHECK(cp.is_ascii());

            CHECK(encode(lexy::ascii_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf8_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf16_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf32_encoding{}, cp)[0] == c);
        }
    }
    SUBCASE("BMP")
    {
        // BMP is guaranteed to round-trip for UTF-16 and UTF-32.
        for (auto c = 0u; c <= 0xFFFF; ++c)
        {
            auto cp = lexy::code_point(c);
            if (cp.is_surrogate())
                continue;
            CHECK(cp.is_bmp());

            CHECK(encode(lexy::utf16_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf32_encoding{}, cp)[0] == c);
        }
    }

    SUBCASE("UTF-8 multi-unit sequences")
    {
        // ä - 2 bytes
        CHECK(encode(lexy::utf8_encoding{}, lexy::code_point(0x00E4)) == u8"\u00E4");
        // € - 3 bytes
        CHECK(encode(lexy::utf8_encoding{}, lexy::code_point(0x20AC)) == u8"\u20AC");
        // 🙂 - 4 bytes
        CHECK(encode(lexy::utf8_encoding{}, lexy::code_point(0x1'F642)) == u8"\U0001F642");
    }
    SUBCASE("UTF-16 multi-unit sequences")
    {
        // €
        CHECK(encode(lexy::utf16_encoding{}, lexy::code_point(0x20AC)) == u"\u20AC");
        // 🙂
        CHECK(encode(lexy::utf16_encoding{}, lexy::code_point(0x1'F642)) == u"\U0001F642");
    }
    SUBCASE("UTF-32 selected characters")
    {
        // €
        CHECK(encode(lexy::utf32_encoding{}, lexy::code_point(0x20AC)) == U"\u20AC");
        // 🙂
        CHECK(encode(lexy::utf32_encoding{}, lexy::code_point(0x1'F642)) == U"\U0001F642");
    }
}

