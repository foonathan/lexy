// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_RULE_VERIFY_HPP_INCLUDED
#define TEST_RULE_VERIFY_HPP_INCLUDED

#include <cassert>
#include <doctest.h>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/result.hpp>

#include "../test_encoding.hpp"

template <typename Error>
using test_error = typename Error::template error<test_input>;

template <typename Callback, typename Production = void>
struct test_context
{
    const char* str;

    using result_type = lexy::result<int, int>;

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return test_context<Callback, SubProduction>{str};
    }

    template <typename Error>
    constexpr auto report_error(const test_input&, Error&& error)
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
    static constexpr auto parse(test_context& self, test_input input, Args&&... args)
    {
        if constexpr (std::is_same_v<Production, void>)
        {
            auto code = Callback{self.str}.success(input.cur(), LEXY_FWD(args)...);
            return result_type(lexy::result_value, code);
        }
        else
        {
            auto code = Callback{self.str}.success(Production{}, input.cur(), LEXY_FWD(args)...);
            return result_type(lexy::result_value, code);
        }
    }
};

template <typename Callback, typename Rule>
constexpr int rule_matches(Rule, const char* str)
{
    test_context<Callback> context{str};
    auto                   input = lexy::zstring_input<test_encoding>(str);
    auto result = Rule::template parser<test_context<Callback>>::parse(context, input);
    if (result)
        return result.value();
    else
        return result.error();
}

#endif // TEST_RULE_VERIFY_HPP_INCLUDED

