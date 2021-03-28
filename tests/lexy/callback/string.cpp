// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/callback/string.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>
#include <string>

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

