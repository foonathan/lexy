// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_PATTERN_VERIFY_HPP_INCLUDED
#define TEST_PATTERN_VERIFY_HPP_INCLUDED

#include <cassert>
#include <doctest.h>
#include <lexy/atom/literal.hpp>
#include <lexy/pattern/base.hpp>

#include "../test_encoding.hpp"

template <typename Pattern>
constexpr auto pattern_matches(Pattern pattern, const char* str)
{
    auto input = lexy::zstring_input<test_encoding>(str);

    auto result = lexy::pattern_match(input, pattern);
    if (!result)
    {
        assert(input.cur() == str);
        return result;
    }
    else
    {
        assert(result.id() >= 0);
        assert(result.match().begin() == str);
        assert(result.match().end() == input.cur());
        return result;
    }
}

#endif // TEST_PATTERN_VERIFY_HPP_INCLUDED

