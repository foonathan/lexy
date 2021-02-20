// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>

#include <cstdio>
#include <doctest/doctest.h>

#if defined(__has_include) && __has_include(<memory_resource>)
#    include <memory_resource>
#    define LEXY_HAS_RESOURCE 1
#else
#    define LEXY_HAS_RESOURCE 0
#endif

namespace
{
constexpr auto test_file_name = "lexy-input-file.test.delete-me";

void write_test_data(const char* data)
{
    auto file = std::fopen(test_file_name, "wb");
    std::fputs(data, file);
    std::fclose(file);
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
        CHECK(reader.eof());
    }
    SUBCASE("tiny file")
    {
        write_test_data("abc");

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        CHECK(reader.peek() == 'a');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 'b');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 'c');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == lexy::default_encoding::eof());
        CHECK(reader.eof());
    }
    SUBCASE("small file")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);
        }

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        for (auto i = 0; i != 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            CHECK(!reader.eof());
            reader.bump();
        }

        for (auto i = 0; i != 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            CHECK(!reader.eof());
            reader.bump();
        }

        CHECK(reader.peek() == lexy::default_encoding::eof());
        CHECK(reader.eof());
    }
    SUBCASE("medium file")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 10 * 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 10 * 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);
        }

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        for (auto i = 0; i != 10 * 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            CHECK(!reader.eof());
            reader.bump();
        }

        for (auto i = 0; i != 10 * 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            CHECK(!reader.eof());
            reader.bump();
        }

        CHECK(reader.peek() == lexy::default_encoding::eof());
        CHECK(reader.eof());
    }
    SUBCASE("big file")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 200 * 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 200 * 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);
        }

        auto buffer = lexy::read_file(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        for (auto i = 0; i != 200 * 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            CHECK(!reader.eof());
            reader.bump();
        }

        for (auto i = 0; i != 200 * 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            CHECK(!reader.eof());
            reader.bump();
        }

        CHECK(reader.peek() == lexy::default_encoding::eof());
        CHECK(reader.eof());
    }
#if LEXY_HAS_RESOURCE
    SUBCASE("custom encoding and resource")
    {
        write_test_data("abc");

        auto buffer = lexy::read_file<lexy::ascii_encoding>(test_file_name,
                                                            std::pmr::new_delete_resource());
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        CHECK(reader.peek() == 'a');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 'b');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 'c');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == lexy::ascii_encoding::eof());
        CHECK(reader.eof());
    }
#endif
    SUBCASE("custom encoding and byte order")
    {
        const unsigned char data[] = {0xFF, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x00};
        write_test_data(reinterpret_cast<const char*>(data));

        auto buffer = lexy::read_file<lexy::utf16_encoding>(test_file_name);
        REQUIRE(buffer);

        auto reader = buffer.value().reader();
        CHECK(reader.peek() == 0x2211);
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 0x4433);
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == lexy::utf16_encoding::eof());
        CHECK(reader.eof());
    }

    std::remove(test_file_name);
}

