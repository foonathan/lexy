// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/encoding.hpp>

#include <doctest.h>
#include <lexy/input/string_input.hpp>
#include <string>

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
        using char_type   = typename decltype(encoding)::char_type;
        using string_type = std::basic_string<char_type>;

        char_type buffer[4] = {};
        auto      count     = encoding.encode_code_point(cp, buffer, 4);
        return string_type(buffer, count);
    };

    SUBCASE("ASCII")
    {
        // ASCII is always guaranteed to round-trip.
        for (auto c = 0u; c <= 0x7F; ++c)
        {
            auto cp = lexy::code_point(c);
            CHECK(cp.is_valid());
            CHECK(cp.is_ascii());

            CHECK(encode(lexy::default_encoding{}, cp)[0] == c);
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
            if (!cp.is_valid())
                continue; // Exclude surrogates.
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

TEST_CASE("decode code point")
{
    auto decode = [](auto encoding, auto str, std::size_t length) {
        auto input  = lexy::string_input<decltype(encoding)>(str, length);
        auto reader = input.reader();

        typename decltype(encoding)::code_point_decoder decoder;
        auto                                            size = decoder.init(reader.peek());
        if (size == -1)
            return lexy::code_point();

        for (auto i = 0; i < size; ++i)
        {
            reader.bump();
            if (!decoder.next(reader.peek()))
                return lexy::code_point();
        }

        return LEXY_MOV(decoder).finish();
    };

    SUBCASE("empty")
    {
        CHECK(!decode(lexy::default_encoding{}, nullptr, 0).is_valid());
        CHECK(!decode(lexy::ascii_encoding{}, nullptr, 0).is_valid());
        CHECK(!decode(lexy::utf8_encoding{}, nullptr, 0).is_valid());
        CHECK(!decode(lexy::utf16_encoding{}, nullptr, 0).is_valid());
        CHECK(!decode(lexy::utf32_encoding{}, nullptr, 0).is_valid());
    }
    SUBCASE("ASCII")
    {
        // ASCII is always guaranteed to round-trip.
        for (auto c = 0u; c <= 0x7F; ++c)
        {
            char     str[]   = {char(c)};
            char16_t str16[] = {char16_t(c)};
            char32_t str32[] = {char32_t(c)};
            auto     cp      = lexy::code_point(c);

            CHECK(decode(lexy::default_encoding{}, str, 1) == cp);
            CHECK(decode(lexy::ascii_encoding{}, str, 1) == cp);
            CHECK(decode(lexy::utf8_encoding{}, str, 1) == cp);
            CHECK(decode(lexy::utf16_encoding{}, str16, 1) == cp);
            CHECK(decode(lexy::utf32_encoding{}, str32, 1) == cp);
        }
    }
    SUBCASE("BMP")
    {
        // BMP is guaranteed to round-trip for UTF-16 and UTF-32.
        for (auto c = 0u; c <= 0xFFFF; ++c)
        {
            auto cp = lexy::code_point(c);
            if (!cp.is_valid())
                continue; // Exclude surrogates.

            char16_t str16[] = {char16_t(c)};
            char32_t str32[] = {char32_t(c)};

            CHECK(decode(lexy::utf16_encoding{}, str16, 1) == cp);
            CHECK(decode(lexy::utf32_encoding{}, str32, 1) == cp);
        }
    }

    SUBCASE("UTF-8 multi-unit sequences")
    {
        // ä - 2 bytes
        CHECK(decode(lexy::utf8_encoding{}, u8"\u00E4", 2) == lexy::code_point(0x00E4));
        // € - 3 bytes
        CHECK(decode(lexy::utf8_encoding{}, u8"\u20AC", 3) == lexy::code_point(0x20AC));
        // 🙂 - 4 bytes
        CHECK(decode(lexy::utf8_encoding{}, u8"\U0001F642", 4) == lexy::code_point(0x1'F642));
    }
    SUBCASE("UTF-16 multi-unit sequences")
    {
        // €
        CHECK(decode(lexy::utf16_encoding{}, u"\u20AC", 1) == lexy::code_point(0x20AC));
        // 🙂
        CHECK(decode(lexy::utf16_encoding{}, u"\U0001F642", 2) == lexy::code_point(0x1'F642));
    }
    SUBCASE("UTF-32 selected characters")
    {
        // €
        CHECK(decode(lexy::utf32_encoding{}, U"\u20AC", 1) == lexy::code_point(0x20AC));
        // 🙂
        CHECK(decode(lexy::utf32_encoding{}, U"\U0001F642", 1) == lexy::code_point(0x1'F642));
    }
}

