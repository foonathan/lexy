// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/string.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("_detail::encode_code_point")
{
    // We test it indirectly by going through lexy::as_string for convenience.
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

TEST_CASE("as_string")
{
    auto char_lexeme = [] {
        auto input  = lexy::zstring_input("abc");
        auto reader = input.reader();

        auto begin = reader.cur();
        reader.bump();
        reader.bump();
        reader.bump();

        return lexy::lexeme(reader, begin);
    }();
    auto uchar_lexeme = [] {
        auto input  = lexy::zstring_input<lexy::byte_encoding>("abc");
        auto reader = input.reader();

        auto begin = reader.cur();
        reader.bump();
        reader.bump();
        reader.bump();

        return lexy::lexeme(reader, begin);
    }();

    SUBCASE("callback")
    {
        std::string from_ptr_size = lexy::as_string<std::string>("abc", 2);
        CHECK(from_ptr_size == "ab");

        std::string from_char_lexeme = lexy::as_string<std::string>(char_lexeme);
        CHECK(from_char_lexeme == "abc");
        std::string from_uchar_lexeme = lexy::as_string<std::string>(uchar_lexeme);
        CHECK(from_uchar_lexeme == "abc");

        std::string from_lvalue = lexy::as_string<std::string>(from_char_lexeme);
        CHECK(from_lvalue == "abc");
        std::string from_rvalue = lexy::as_string<std::string>(std::string("test"));
        CHECK(from_rvalue == "test");

        std::string from_ascii_cp
            = lexy::as_string<std::string, lexy::ascii_encoding>(lexy::code_point('a'));
        CHECK(from_ascii_cp == "a");
        std::string from_unicode_cp
            = lexy::as_string<std::string, lexy::utf8_encoding>(lexy::code_point(0x00E4));
        CHECK(from_unicode_cp == "\u00E4");
    }
    SUBCASE("sink")
    {
        auto sink = lexy::as_string<std::string, lexy::utf8_encoding>.sink();
        sink('a');
        sink("bcd", 2);
        sink(char_lexeme);
        sink(uchar_lexeme);
        sink(std::string("hi"));
        sink(lexy::code_point('a'));
        sink(lexy::code_point(0x00E4));

        std::string result = LEXY_MOV(sink).finish();
        CHECK(result == "abcabcabchia\u00E4");
    }
}

