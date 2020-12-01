// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/range_input.hpp>

#include <doctest/doctest.h>

namespace
{
struct test_iterator
{
    int count;

    // No initialization in default ctor.

    char operator*() const
    {
        return 'a';
    }

    test_iterator& operator++()
    {
        --count;
        return *this;
    }
};

struct test_sentinel
{
    friend bool operator==(test_iterator iter, test_sentinel)
    {
        return iter.count == 0;
    }
};
} // namespace

TEST_CASE("range_input")
{
    lexy::range_input<lexy::default_encoding, test_iterator, test_sentinel> input;
    CHECK(sizeof(input) == (LEXY_HAS_EMPTY_MEMBER ? sizeof(int) : 2 * sizeof(int)));

    CHECK(input.reader().cur().count == 0);
    CHECK(input.reader().peek() == lexy::default_encoding::eof());

    input       = lexy::range_input(test_iterator{3}, test_sentinel{});
    auto reader = input.reader();
    CHECK(reader.cur().count == 3);
    CHECK(reader.peek() == 'a');
    CHECK(!reader.eof());

    reader.bump();
    CHECK(reader.cur().count == 2);
    CHECK(reader.peek() == 'a');
    CHECK(!reader.eof());

    reader.bump();
    CHECK(reader.cur().count == 1);
    CHECK(reader.peek() == 'a');
    CHECK(!reader.eof());

    reader.bump();
    CHECK(reader.cur().count == 0);
    CHECK(reader.peek() == lexy::default_encoding::eof());
    CHECK(reader.eof());
}

