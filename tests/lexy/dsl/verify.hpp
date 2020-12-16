// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_DSL_VERIFY_HPP_INCLUDED
#define TEST_DSL_VERIFY_HPP_INCLUDED

#include <doctest/doctest.h>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/lexeme.hpp>
#include <lexy/match.hpp>
#include <lexy/result.hpp>

#include "../test_encoding.hpp"

[[noreturn]] inline bool constexpr_check_failure()
{
    throw 0;
}

#define CONSTEXPR_CHECK(x) ((x) ? true : constexpr_check_failure())

template <typename Tag>
using test_error = lexy::error_for<test_input, Tag>;

template <typename Callback, typename CharT, typename Production = void>
struct test_handler
{
    const CharT* str;

    using result_type = lexy::result<int, int>;

    template <typename SubProduction>
    constexpr auto sub_handler(const lexy::input_reader<test_input>&)
    {
        return test_handler<Callback, CharT, SubProduction>{str};
    }

    constexpr auto list_sink()
    {
        return Callback{str}.list();
    }

    template <typename Error>
    constexpr auto error(Error&& error) &&
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

template <typename Callback, typename Encoding, typename CharT, typename Rule>
constexpr int verify(Rule, const CharT* str, std::size_t size = std::size_t(-1))
{
    auto input  = size == std::size_t(-1) ? lexy::zstring_input<Encoding>(str)
                                          : lexy::string_input<Encoding>(str, size);
    auto reader = input.reader();

    test_handler<Callback, CharT> handler{str};
    auto result = Rule::template parser<test_final_parser>::parse(handler, reader);
    if (result)
        return result.value();
    else
        return result.error();
}

template <typename Callback, typename Rule>
constexpr int verify(Rule rule, const char* str, std::size_t size = std::size_t(-1))
{
    return verify<Callback, test_encoding>(rule, str, size);
}

#endif // TEST_DSL_VERIFY_HPP_INCLUDED

