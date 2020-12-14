// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/code_point.hpp>

#include "verify.hpp"
#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>
#include <lexy/parse.hpp>

TEST_CASE("dsl::code_point")
{
    static constexpr auto atom = lexy::dsl::code_point;
    CHECK(lexy::is_pattern<decltype(atom)>);

    // TODO: do some basic tests that the correct engine is used
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

    CHECK(parse(lexy::ascii_encoding{}, "a").value() == 'a');
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

