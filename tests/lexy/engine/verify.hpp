// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED
#define TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED

#include <doctest/doctest.h>
#include <lexy/engine/base.hpp>
#include <lexy/input/string_input.hpp>

template <typename Matcher>
struct engine_match_result
{
    typename Matcher::error_code ec;
    std::size_t                  count;

    explicit operator bool() const
    {
        return ec == typename Matcher::error_code();
    }
};

template <typename Matcher, typename CharT>
constexpr auto engine_matches(const CharT* str)
{
    auto input  = lexy::zstring_input(str);
    auto reader = input.reader();

    auto begin  = reader.cur();
    auto result = Matcher::match(reader);
    auto end    = reader.cur();

    return engine_match_result<Matcher>{result, std::size_t(end - begin)};
}

#endif // TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED

