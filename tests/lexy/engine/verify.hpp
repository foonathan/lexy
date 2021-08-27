// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED
#define TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED

#include <doctest/doctest.h>
#include <lexy/engine/base.hpp>
#include <lexy/input/string_input.hpp>

//=== engine_matches ===//
template <typename Matcher>
struct engine_match_result
{
    typename Matcher::error_code ec;
    std::size_t                  count;
    int recovered; // < 0 if unable to recover, >= 0 count that was recovered

    explicit operator bool() const
    {
        return ec == typename Matcher::error_code();
    }
};

template <typename Matcher, typename Encoding, typename CharT>
auto engine_matches(const CharT* str, std::size_t size = std::size_t(-1))
{
    auto input  = size == std::size_t(-1) ? lexy::zstring_input<Encoding>(str)
                                          : lexy::string_input<Encoding>(str, size);
    auto reader = input.reader();

    auto begin  = reader.position();
    auto result = Matcher::match(reader);
    auto end    = reader.position();
    if (result == typename Matcher::error_code())
        return engine_match_result<Matcher>{result, std::size_t(end - begin), -1};

    auto recovered     = Matcher::recover(reader, result);
    auto recovered_end = reader.position();
    return engine_match_result<Matcher>{result, std::size_t(end - begin),
                                        recovered ? int(recovered_end - end) : -1};
}
template <typename Matcher, typename CharT>
auto engine_matches(const CharT* str, std::size_t size = std::size_t(-1))
{
    return engine_matches<Matcher, lexy::deduce_encoding<CharT>>(str, size);
}

//=== engine_parses ===//
template <typename Parser, typename T>
struct engine_parse_result
{
    T                           value;
    typename Parser::error_code ec;
    std::size_t                 count;
    int recovered; // < 0 if unable to recover, >= 0 count that was recovered

    explicit operator bool() const
    {
        return ec == typename Parser::error_code();
    }
};

template <typename Parser, typename Encoding, typename CharT>
auto engine_parses(const CharT* str)
{
    auto input  = lexy::zstring_input<Encoding>(str);
    auto reader = input.reader();

    typename Parser::error_code ec{};
    auto                        begin  = reader.position();
    auto                        result = Parser::parse(ec, reader);
    auto                        end    = reader.position();
    if (ec == typename Parser::error_code())
        return engine_parse_result<Parser, decltype(result)>{result, ec, std::size_t(end - begin),
                                                             -1};

    auto recovered     = Parser::recover(reader, ec);
    auto recovered_end = reader.position();
    return engine_parse_result<Parser, decltype(result)>{result, ec, std::size_t(end - begin),
                                                         recovered ? int(recovered_end - end) : -1};
}
template <typename Parser, typename CharT>
auto engine_parses(const CharT* str)
{
    return engine_parses<Parser, lexy::deduce_encoding<CharT>>(str);
}

#endif // TESTS_LEXY_ENGINE_VERIFY_HPP_INCLUDED

