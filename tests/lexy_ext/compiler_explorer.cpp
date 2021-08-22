// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy_ext/compiler_explorer.hpp>

#include <doctest/doctest.h>

TEST_CASE("read_stdin")
{
    constexpr auto test_file_name = "lexy_ext-compiler_explorer.test.delete-me";

    // Here, we'll reassociate stdin with our test file.
    // This means that we'll permantently loose stdin, but that's okay -- unit tests don't need it.
    auto write_stdin = [](const char* data) {
        auto file = std::fopen(test_file_name, "wb");
        std::fputs(data, file);
        std::fclose(file);

        auto result = std::freopen(test_file_name, "rb", stdin);
        REQUIRE(result == stdin);
    };

    std::remove(test_file_name);

    SUBCASE("empty")
    {
        write_stdin("");

        auto input  = lexy_ext::compiler_explorer_input();
        auto reader = input.reader();
        CHECK(reader.peek() == lexy::utf8_encoding::eof());
        CHECK(reader.eof());
    }
    SUBCASE("small")
    {
        write_stdin("abc");

        auto input  = lexy_ext::compiler_explorer_input();
        auto reader = input.reader();
        CHECK(reader.peek() == 'a');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 'b');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == 'c');
        CHECK(!reader.eof());

        reader.bump();
        CHECK(reader.peek() == lexy::utf8_encoding::eof());
        CHECK(reader.eof());
    }
    SUBCASE("big")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);

            auto result = std::freopen(test_file_name, "rb", stdin);
            REQUIRE(result == stdin);
        }

        auto input  = lexy_ext::compiler_explorer_input();
        auto reader = input.reader();

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

        CHECK(reader.peek() == lexy::utf8_encoding::eof());
        CHECK(reader.eof());
    }

    std::remove(test_file_name);
}

