// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/code_point.hpp>

#include "verify.hpp"
#include <doctest.h>
#include <lexy/input/string_input.hpp>
#include <lexy/parse.hpp>

TEST_CASE("dsl::code_point")
{
    static constexpr auto atom = lexy::dsl::code_point;
    CHECK(lexy::is_pattern<decltype(atom)>);

    static constexpr auto match = [](auto encoding, auto str) LEXY_CONSTEVAL {
        auto input  = lexy::zstring_input<decltype(encoding)>(str);
        auto reader = input.reader();

        auto result = atom.match(reader);
        if (result)
            return int(reader.cur() - input.begin());

        auto error = atom.error(reader, input.begin());
        CONSTEXPR_CHECK(error.position() == input.begin());
        return -int(reader.cur() - input.begin()) - 1;
    };

    // Empty string.
    CHECK(match(lexy::ascii_encoding{}, "") == -1);
    CHECK(match(lexy::utf8_encoding{}, u8"") == -1);
    CHECK(match(lexy::utf16_encoding{}, u"") == -1);
    CHECK(match(lexy::utf32_encoding{}, U"") == -1);

    SUBCASE("well-formed input")
    {
        // ASCII character.
        CHECK(match(lexy::ascii_encoding{}, "a") == 1);
        CHECK(match(lexy::utf8_encoding{}, u8"a") == 1);
        CHECK(match(lexy::utf16_encoding{}, u"a") == 1);
        CHECK(match(lexy::utf32_encoding{}, U"a") == 1);

        // BOM character.
        CHECK(match(lexy::ascii_encoding{}, "\u00E4") == -1);
        CHECK(match(lexy::utf8_encoding{}, u8"\u00E4") == 2);
        CHECK(match(lexy::utf16_encoding{}, u"\u00E4") == 1);
        CHECK(match(lexy::utf32_encoding{}, U"\u00E4") == 1);

        // Three byte UTF-8 character.
        CHECK(match(lexy::ascii_encoding{}, "\u20AC") == -1);
        CHECK(match(lexy::utf8_encoding{}, u8"\u20AC") == 3);
        CHECK(match(lexy::utf16_encoding{}, u"\u20AC") == 1);
        CHECK(match(lexy::utf32_encoding{}, U"\u20AC") == 1);

        // Four byte UTF-8 character.
        CHECK(match(lexy::ascii_encoding{}, "\U0001F642") == -1);
        CHECK(match(lexy::utf8_encoding{}, u8"\U0001F642") == 4);
        CHECK(match(lexy::utf16_encoding{}, u"\U0001F642") == 2);
        CHECK(match(lexy::utf32_encoding{}, U"\U0001F642") == 1);
    }
    SUBCASE("ill-formed UTF-8")
    {
        auto match_utf8 = [](auto... cs) {
            // We use the second 0 to silence a bogus warning about array bounds on GCC 8.
            LEXY_CHAR8_T array[] = {LEXY_CHAR8_T(cs)..., 0, 0};
            return match(lexy::utf8_encoding{}, array);
        };

        // Missing first continuation byte.
        CHECK(match_utf8(0b1101'0000) == -2);
        CHECK(match_utf8(0b1110'1000) == -2);
        CHECK(match_utf8(0b1111'0100) == -2);
        // Missing second continuation byte.
        CHECK(match_utf8(0b1110'1000, 0b1000'0001) == -3);
        CHECK(match_utf8(0b1111'0100, 0b1000'0001) == -3);
        // Missing third continuation byte.
        CHECK(match_utf8(0b1111'0100, 0b1000'0001, 0b1000'0001) == -4);

        // Invalid first continuation byte.
        CHECK(match_utf8(0b1101'0000, 0b1111) == -2);
        CHECK(match_utf8(0b1110'1000, 0b1111) == -2);
        CHECK(match_utf8(0b1111'0100, 0b1111) == -2);
        // Invalid second continuation byte.
        CHECK(match_utf8(0b1110'1000, 0b1000'0001, 0b1111) == -3);
        CHECK(match_utf8(0b1111'0100, 0b1000'0001, 0b1111) == -3);
        // Invalid third continuation byte.
        CHECK(match_utf8(0b1111'0100, 0b1000'0001, 0b1000'0001, 0b1111) == -4);

        // Surrogate.
        CHECK(match_utf8(0b1110'1101, 0b1011'1111, 0b1011'1111) == -3);
        // Out of range code point.
        CHECK(match_utf8(0b1111'0111, 0b1011'1111, 0b1011'1111, 0b1011'1111) == -4);

        // Begins with continuation bytes.
        CHECK(match_utf8(0b1000'0001) == -1);
        // Begins with invalid byte.
        for (auto byte = 0xF8; byte <= 0xFF; ++byte)
            CHECK(match_utf8(byte) == -1);

        // Two byte overlong sequence.
        CHECK(match_utf8(0xC0, 0x84) == -1);
        CHECK(match_utf8(0xC1, 0x84) == -1);

        // Three byte overlong sequence.
        CHECK(match_utf8(0xE0, 0x80, 0x80) == -2);
        CHECK(match_utf8(0xE0, 0x84, 0x80) == -2);

        // Four byte overlong sequence.
        CHECK(match_utf8(0xF0, 0x80, 0x80, 0x80) == -2);
        CHECK(match_utf8(0xF0, 0x89, 0x80, 0x80) == -2);
    }
    SUBCASE("ill-formed UTF-16")
    {
        auto match_utf16 = [](auto... cs) {
            char16_t array[] = {char16_t(cs)..., 0};
            return match(lexy::utf16_encoding{}, array);
        };

        // Missing continuation.
        CHECK(match_utf16(0xD876) == -2);
        // Invalid continuation.
        CHECK(match_utf16(0xD876, 0x3000) == -2);

        // Begins with continuation.
        CHECK(match_utf16(0xDF00) == -1);
    }
    SUBCASE("ill-formed UTF-32")
    {
        auto match_utf32 = [](auto... cs) {
            char32_t array[] = {char32_t(cs)..., 0};
            return match(lexy::utf32_encoding{}, array);
        };

        // Surrogate.
        CHECK(match_utf32(0xD900) == -1);
        // Overflow.
        CHECK(match_utf32(0x20'FFFF) == -1);
    }
}

namespace
{
struct production
{
    static constexpr auto rule  = lexy::dsl::code_point.capture();
    static constexpr auto value = lexy::forward<lexy::code_point>;
};
} // namespace

TEST_CASE("dsl::code_point.capture()")
{
    constexpr auto parse = [](auto encoding, auto str) LEXY_CONSTEVAL {
        auto input  = lexy::zstring_input<decltype(encoding)>(str);
        auto result = lexy::parse<production>(input, lexy::noop);
        if (result)
            return result.value();
        else
            return lexy::code_point();
    };

    // Just very basic tests necessary, same logic as above.

    CHECK(parse(lexy::ascii_encoding{}, "a") == lexy::code_point('a'));
    CHECK(parse(lexy::utf8_encoding{}, u8"a") == lexy::code_point('a'));
    CHECK(parse(lexy::utf16_encoding{}, u"a") == lexy::code_point('a'));
    CHECK(parse(lexy::utf32_encoding{}, U"a") == lexy::code_point('a'));

    CHECK(parse(lexy::ascii_encoding{}, "\u00E4") == lexy::code_point());
    CHECK(parse(lexy::utf8_encoding{}, u8"\u00E4") == lexy::code_point(0x00E4));
    CHECK(parse(lexy::utf16_encoding{}, u"\u00E4") == lexy::code_point(0x00E4));
    CHECK(parse(lexy::utf32_encoding{}, U"\u00E4") == lexy::code_point(0x00E4));

    CHECK(parse(lexy::ascii_encoding{}, "\u20AC") == lexy::code_point());
    CHECK(parse(lexy::utf8_encoding{}, u8"\u20AC") == lexy::code_point(0x20AC));
    CHECK(parse(lexy::utf16_encoding{}, u"\u20AC") == lexy::code_point(0x20AC));
    CHECK(parse(lexy::utf32_encoding{}, U"\u20AC") == lexy::code_point(0x20AC));

    CHECK(parse(lexy::ascii_encoding{}, "\U0001F642") == lexy::code_point());
    CHECK(parse(lexy::utf8_encoding{}, u8"\U0001F642") == lexy::code_point(0x1F642));
    CHECK(parse(lexy::utf16_encoding{}, u"\U0001F642") == lexy::code_point(0x1F642));
    CHECK(parse(lexy::utf32_encoding{}, U"\U0001F642") == lexy::code_point(0x1F642));
}

