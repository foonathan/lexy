// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_ATOM_VERIFY_HPP_INCLUDED
#define TEST_ATOM_VERIFY_HPP_INCLUDED

#include <doctest.h>
#include <lexy/atom/base.hpp>
#include <lexy/input/string_input.hpp>

struct test_encoding
{
    using char_type = char;
    struct int_type
    {
        int value;

        friend constexpr bool operator==(int_type a, int_type b) noexcept
        {
            return a.value == b.value;
        }
        friend constexpr bool operator!=(int_type a, int_type b) noexcept
        {
            return a.value != b.value;
        }

        friend constexpr bool operator<(int_type a, int_type b) noexcept
        {
            return a.value < b.value;
        }
        friend constexpr bool operator<=(int_type a, int_type b) noexcept
        {
            return a.value <= b.value;
        }
        friend constexpr bool operator>(int_type a, int_type b) noexcept
        {
            return a.value > b.value;
        }
        friend constexpr bool operator>=(int_type a, int_type b) noexcept
        {
            return a.value >= b.value;
        }
    };

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        return {lexy::default_encoding::eof()};
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c) noexcept
    {
        return {lexy::default_encoding::to_int_type(c)};
    }
};

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
    auto input = size == std::size_t(-1) ? lexy::zstring_input<test_encoding>(str)
                                         : lexy::string_input<test_encoding>(str, size);
    auto pos         = input.cur();
    using error_type = decltype(Atom::error(input, pos));

    if (Atom::match(input))
        return atom_match_result<error_type>(str, std::size_t(input.cur() - pos));
    else
        return atom_match_result<error_type>(str, std::size_t(input.cur() - pos),
                                             Atom::error(input, pos));
}

#endif // TEST_ATOM_VERIFY_HPP_INCLUDED

