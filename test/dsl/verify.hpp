// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_DSL_VERIFY_HPP_INCLUDED
#define TEST_DSL_VERIFY_HPP_INCLUDED

#include <doctest.h>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/lexeme.hpp>
#include <lexy/result.hpp>

#include "../test_encoding.hpp"

[[noreturn]] inline bool constexpr_check_failure()
{
    throw 0;
}

#define CONSTEXPR_CHECK(x) ((x) ? true : constexpr_check_failure())

//=== atom ===//
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
template <>
struct atom_match_result<void>
{
    const char* input;
    std::size_t count;

    constexpr explicit atom_match_result(const char* input, std::size_t count)
    : input(input), count(count)
    {}

    constexpr explicit operator bool() const noexcept
    {
        return true;
    }
};

template <typename Atom>
constexpr auto atom_matches(Atom, const char* str, std::size_t size = std::size_t(-1))
{
    auto input = size == std::size_t(-1) ? lexy::zstring_input<test_encoding>(str)
                                         : lexy::string_input<test_encoding>(str, size);
    auto reader = input.reader();

    auto pos         = reader.cur();
    using error_type = decltype(Atom::error(reader, pos));

    if (Atom::match(reader))
        return atom_match_result<error_type>(str, std::size_t(reader.cur() - pos));
    else if constexpr (!std::is_same_v<error_type, void>)
        return atom_match_result<error_type>(str, std::size_t(reader.cur() - pos),
                                             Atom::error(reader, pos));
    else
        CONSTEXPR_CHECK(false);
}

//=== pattern ===//
struct pattern_match_result
{
    bool                       _matches;
    lexy::_detail::string_view _match;

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
    auto input  = lexy::zstring_input<test_encoding>(str);
    auto reader = input.reader();

    auto begin  = reader.cur();
    auto result = Pattern::matcher::match(reader);
    auto match  = lexy::lexeme(reader, begin);
    return pattern_match_result{result, {match.data(), match.size()}};
}

//=== rule ===//
template <typename Tag>
using test_error = lexy::error_for<test_input, Tag>;

template <typename Callback, typename Production = void>
struct test_handler
{
    const char* str;

    using result_type = lexy::result<int, int>;

    template <typename SubProduction>
    constexpr auto sub_handler(const lexy::input_reader<test_input>&)
    {
        return test_handler<Callback, SubProduction>{str};
    }

    constexpr auto list_sink()
    {
        return Callback{str}.list();
    }

    template <typename Error>
    constexpr auto error(const lexy::input_reader<test_input>&, Error&& error) &&
    {
        if constexpr (std::is_same_v<Production, void>)
        {
            auto code = Callback{str}.error(LEXY_FWD(error));
            return result_type(lexy::result_error, code);
        }
        else
        {
            auto code = Callback{str}.error(Production{}, LEXY_FWD(error));
            return result_type(lexy::result_error, code);
        }
    }

    template <typename... Args>
    constexpr auto value(Args&&... args) &&
    {
        if constexpr (std::is_same_v<Production, void>)
        {
            auto code = Callback{str}.success(LEXY_FWD(args)...);
            return result_type(lexy::result_value, code);
        }
        else
        {
            auto code = Callback{str}.success(Production{}, LEXY_FWD(args)...);
            return result_type(lexy::result_value, code);
        }
    }
};

struct test_final_parser
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
        typename Handler::result_type
    {
        // We sneak in the final input position.
        return LEXY_MOV(handler).value(reader.cur(), LEXY_FWD(args)...);
    }
};

template <typename Callback, typename Rule>
constexpr int rule_matches(Rule, const char* str)
{
    auto input  = lexy::zstring_input<test_encoding>(str);
    auto reader = input.reader();

    test_handler<Callback> handler{str};
    auto result = Rule::template parser<test_final_parser>::parse(handler, reader);
    if (result)
        return result.value();
    else
        return result.error();
}

#endif // TEST_DSL_VERIFY_HPP_INCLUDED

