// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/string_input.hpp>

#include <doctest.h>

TEST_CASE("string_input")
{
    static const char str[] = {'a', 'b', 'c', '\0'};
    struct view
    {
        using char_type = char;

        auto data() const
        {
            return str;
        }

        std::size_t size() const
        {
            return 3;
        }
    };

    SUBCASE("basic")
    {
        lexy::string_input<> input;
        CHECK(input.cur() == nullptr);
        CHECK(input.peek() == lexy::default_encoding::eof());

        SUBCASE("range ctor")
        {
            input = lexy::string_input(str, str + 3);
        }
        SUBCASE("ptr + size ctor")
        {
            input = lexy::string_input(str, 3);
        }
        SUBCASE("view")
        {
            input = lexy::string_input(view());
        }
        SUBCASE("zstring")
        {
            input = lexy::zstring_input(str);
        }

        CHECK(input.cur() == str);
        CHECK(input.peek() == 'a');

        input.bump();
        CHECK(input.cur() == str + 1);
        CHECK(input.peek() == 'b');

        input.bump();
        CHECK(input.cur() == str + 2);
        CHECK(input.peek() == 'c');

        input.bump();
        CHECK(input.cur() == str + 3);
        CHECK(input.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("converting ctor")
    {
        lexy::string_input<lexy::raw_encoding> input;

        SUBCASE("range ctor")
        {
            input = lexy::string_input<lexy::raw_encoding>(str, str + 3);
        }
        SUBCASE("ptr + size ctor")
        {
            input = lexy::string_input<lexy::raw_encoding>(str, 3);
        }
        SUBCASE("view")
        {
            input = lexy::string_input<lexy::raw_encoding>(view());
        }
        SUBCASE("zstring")
        {
            input = lexy::zstring_input<lexy::raw_encoding>(str);
        }

        CHECK(input.cur() == reinterpret_cast<const unsigned char*>(str));
    }
}

