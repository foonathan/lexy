// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_PATTERN_VERIFY_HPP_INCLUDED
#define TEST_PATTERN_VERIFY_HPP_INCLUDED

#include <cassert>
#include <doctest.h>
#include <lexy/dsl/literal.hpp>
#include <lexy/lexeme.hpp>

#include "../test_encoding.hpp"

struct pattern_match_result
{
    bool                     _matches;
    lexy::lexeme<test_input> _match;

    constexpr explicit operator bool() const
    {
        return _matches;
    }

    constexpr auto match() const
    {
        return _match;
    }
};

template <typename Pattern>
constexpr auto pattern_matches(Pattern, const char* str)
{
    auto input = lexy::zstring_input<test_encoding>(str);

    auto begin  = input.cur();
    auto result = Pattern::matcher::match(input);
    auto match  = lexy::lexeme(input, begin);
    return pattern_match_result{result, match};
}

#endif // TEST_PATTERN_VERIFY_HPP_INCLUDED

