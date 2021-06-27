// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/string_input.hpp>

#include <doctest/doctest.h>

TEST_CASE("string_input")
{
    static const char str[] = {'a', 'b', 'c', '\0'};
    struct view
    {
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
        CHECK(input.data() == nullptr);
        CHECK(input.size() == 0u);
        CHECK(input.reader().cur() == nullptr);
        CHECK(input.reader().peek() == lexy::default_encoding::eof());
        CHECK(input.reader().eof());

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
        CHECK(input.data() == str);
        CHECK(input.size() == 3);

        auto reader = input.reader();
        CHECK(reader.cur() == str);
        CHECK(reader.peek() == 'a');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.cur() == str + 1);
        CHECK(reader.peek() == 'b');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.cur() == str + 2);
        CHECK(reader.peek() == 'c');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.cur() == str + 3);
        CHECK(reader.peek() == lexy::default_encoding::eof());
        CHECK(reader.eof());
    }
    SUBCASE("converting ctor")
    {
        lexy::string_input<lexy::byte_encoding> input;

        SUBCASE("range ctor")
        {
            input = lexy::string_input<lexy::byte_encoding>(str, str + 3);
        }
        SUBCASE("ptr + size ctor")
        {
            input = lexy::string_input<lexy::byte_encoding>(str, 3);
        }
        SUBCASE("view")
        {
            input = lexy::string_input<lexy::byte_encoding>(view());
        }
        SUBCASE("zstring")
        {
            input = lexy::zstring_input<lexy::byte_encoding>(str);
        }

        CHECK(input.data() == reinterpret_cast<const unsigned char*>(str));
        CHECK(input.size() == 3);
    }
}

