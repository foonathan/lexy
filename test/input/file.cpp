// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>

#include <doctest.h>
#include <memory_resource>

namespace
{
constexpr auto test_file_name = "lexy-input-file.test.delete-me";

void write_test_data(const char* data)
{
    lexy::_file_handle file(std::fopen(test_file_name, "wb"));
    std::fputs(data, file);
}
} // namespace

TEST_CASE("read_file")
{
    std::remove(test_file_name);

    SUBCASE("non-existing file")
    {
        auto buffer = lexy::read_file(test_file_name);
        CHECK(!buffer);
        CHECK(buffer.error() == lexy::file_error::file_not_found);
    }
    SUBCASE("empty file")
    {
        write_test_data("");

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("small file")
    {
        write_test_data("abc");

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("big file")
    {
        {
            lexy::_file_handle file(std::fopen(test_file_name, "wb"));
            for (auto i = 0; i != 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 1024; ++i)
                std::fputc('b', file);
        }

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        for (auto i = 0; i != 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            reader.bump();
        }

        for (auto i = 0; i != 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            reader.bump();
        }

        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("custom encoding and resource")
    {
        write_test_data("abc");

        auto buffer = lexy::read_file<lexy::ascii_encoding>(test_file_name,
                                                            std::pmr::new_delete_resource());
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::ascii_encoding::eof());
    }

    std::remove(test_file_name);
}
