// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy_ext/shell.hpp>

#include <doctest/doctest.h>

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/char_class.hpp>
#include <lexy/engine/literal.hpp>
#include <lexy/engine/trie.hpp>

namespace
{
class test_prompt
{
public:
    using encoding = lexy::default_encoding;

    void primary_prompt()
    {
        continuation_count = 0;
    }

    void continuation_prompt()
    {
        REQUIRE(max_lines >= 0);
        ++continuation_count;
    }

    void eof_prompt()
    {
        REQUIRE(!open);
    }

    bool is_open() const
    {
        return open;
    }

    auto read_line()
    {
        struct callback
        {
            test_prompt& self;

            std::size_t operator()(char* buffer, std::size_t size)
            {
                REQUIRE(size > 4);

                if (self.max_lines-- == 0)
                {
                    self.open = false;
                    return 0;
                }

                buffer[0] = 'a';
                buffer[1] = 'b';
                buffer[2] = 'c';
                buffer[3] = '\n';
                return 4;
            }

            void done() && {}
        };

        return callback{*this};
    }

    // write_message() not needed as it's not tested

public:
    explicit test_prompt(int max_lines) : continuation_count(-1), max_lines(max_lines), open(true)
    {}

    int  continuation_count;
    int  max_lines;
    bool open;
};
} // namespace

TEST_CASE("shell")
{
    lexy_ext::shell<test_prompt> shell(test_prompt(3));
    CHECK(shell.is_open());

    {
        auto input  = shell.prompt_for_input();
        auto reader = input.reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == '\n');

        CHECK(shell.get_prompt().max_lines == 2);
        CHECK(shell.get_prompt().continuation_count == 0);
    }

    {
        auto input  = shell.prompt_for_input();
        auto reader = input.reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == '\n');

        reader.bump();
        CHECK(reader.peek() == 'a');

        CHECK(shell.get_prompt().max_lines == 0);
        CHECK(shell.get_prompt().continuation_count == 1);
    }

    {
        auto input  = shell.prompt_for_input();
        auto reader = input.reader();
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
}

namespace
{
constexpr auto linear_trie = lexy::linear_trie<LEXY_NTTP_STRING("abc\n")>;
}

TEST_CASE("engine_literal and shell")
{
    lexy_ext::shell<test_prompt> shell(test_prompt(3));

    auto input  = shell.prompt_for_input();
    auto reader = input.reader();

    using engine = lexy::engine_literal<linear_trie>;
    auto result  = engine::match(reader);
    CHECK(result == engine::error_code());
    CHECK(shell.get_prompt().max_lines == 2);
    CHECK(shell.get_prompt().continuation_count == 0);
}

namespace
{
constexpr auto trie = lexy::trie<char, LEXY_NTTP_STRING("abc\n")>;
}

TEST_CASE("engine_trie and shell")
{
    lexy_ext::shell<test_prompt> shell(test_prompt(3));

    auto input  = shell.prompt_for_input();
    auto reader = input.reader();

    using engine = lexy::engine_trie<trie>;
    auto result  = engine::match(reader);
    CHECK(result == engine::error_code());
    CHECK(shell.get_prompt().max_lines == 2);
    CHECK(shell.get_prompt().continuation_count == 0);
}

