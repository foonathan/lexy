// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_ATOM_VERIFY_HPP_INCLUDED
#define TEST_ATOM_VERIFY_HPP_INCLUDED

#include <doctest.h>
#include <lexy/atom/base.hpp>
#include <lexy/input/string_input.hpp>

template <typename Error>
struct atom_match_result
{
    bool        matches;
    const char* input;
    std::size_t count;
    union
    {
        bool  success;
        Error error;
    };

    constexpr explicit atom_match_result(const char* input, std::size_t count)
    : matches(true), input(input), count(count), success()
    {}
    constexpr explicit atom_match_result(const char* input, std::size_t count, Error error)
    : matches(false), input(input), count(count), error(error)
    {}

    constexpr explicit operator bool() const noexcept
    {
        return matches;
    }
};

template <typename Atom>
constexpr auto atom_matches(Atom, const char* str, std::size_t size = std::size_t(-1))
{
    auto input = size == std::size_t(-1) ? lexy::zstring_input(str) : lexy::string_input(str, size);
    auto pos   = input.cur();
    using error_type = decltype(Atom::error(input, pos));

    if (Atom::match(input))
        return atom_match_result<error_type>(str, std::size_t(input.cur() - pos));
    else
        return atom_match_result<error_type>(str, std::size_t(input.cur() - pos),
                                             Atom::error(input, pos));
}

#endif // TEST_ATOM_VERIFY_HPP_INCLUDED

