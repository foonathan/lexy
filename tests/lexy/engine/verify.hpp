// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED
#define TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED

#include <doctest/doctest.h>
#include <lexy/engine/base.hpp>
#include <lexy/input/string_input.hpp>

[[noreturn]] inline bool constexpr_check_failure()
{
    throw 0;
}

#define CONSTEXPR_CHECK(x) ((x) ? true : constexpr_check_failure())

//=== engine_matches ===//
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

template <typename Matcher, typename Encoding, typename CharT>
constexpr auto engine_matches(const CharT* str)
{
    auto input  = lexy::zstring_input<Encoding>(str);
    auto reader = input.reader();

    auto begin  = reader.cur();
    auto result = Matcher::match(reader);
    auto end    = reader.cur();

    return engine_match_result<Matcher>{result, std::size_t(end - begin)};
}
template <typename Matcher, typename CharT>
constexpr auto engine_matches(const CharT* str)
{
    return engine_matches<Matcher, lexy::deduce_encoding<CharT>>(str);
}

//=== engine_parses ===//
template <typename Parser, typename T>
struct engine_parse_result
{
    T                           value;
    typename Parser::error_code ec;
    std::size_t                 count;

    explicit operator bool() const
    {
        return ec == typename Parser::error_code();
    }
};

template <typename Parser, typename Encoding, typename CharT>
constexpr auto engine_parses(const CharT* str)
{
    auto input  = lexy::zstring_input<Encoding>(str);
    auto reader = input.reader();

    typename Parser::error_code ec{};
    auto                        begin  = reader.cur();
    auto                        result = Parser::parse(ec, reader);
    auto                        end    = reader.cur();

    return engine_parse_result<Parser, decltype(result)>{result, ec, std::size_t(end - begin)};
}
template <typename Parser, typename CharT>
constexpr auto engine_parses(const CharT* str)
{
    return engine_parses<Parser, lexy::deduce_encoding<CharT>>(str);
}

#endif // TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED

