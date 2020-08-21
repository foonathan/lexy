// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TEST_RULE_VERIFY_HPP_INCLUDED
#define TEST_RULE_VERIFY_HPP_INCLUDED

#include <cassert>
#include <doctest.h>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>

#include "../test_encoding.hpp"

template <typename Error>
using test_error = typename Error::template error<test_input>;

template <typename Callback, typename Production = void>
struct test_context
{
    const char* str;

    using result_type = int;

    constexpr bool is_success(int i)
    {
        return i >= 0;
    }
    constexpr int forward_value(int i)
    {
        return i;
    }
    template <typename Parent>
    constexpr int forward_error_result(Parent&, int i)
    {
        return i;
    }

    template <typename SubProduction>
    constexpr auto sub_context()
    {
        return test_context<Callback, SubProduction>{str};
    }

    template <typename Error>
    constexpr int report_error(const test_input&, Error&& error)
    {
        if constexpr (std::is_same_v<Production, void>)
            return Callback{str}.error(LEXY_FWD(error));
        else
            return Callback{str}.error(Production{}, LEXY_FWD(error));
    }

    template <typename... Args>
    static constexpr int parse(test_context& self, test_input input, Args&&... args)
    {
        if constexpr (std::is_same_v<Production, void>)
            return Callback{self.str}.success(input.cur(), LEXY_FWD(args)...);
        else
            return Callback{self.str}.success(Production{}, input.cur(), LEXY_FWD(args)...);
    }
};

template <typename Callback, typename Rule>
constexpr auto rule_matches(Rule, const char* str)
{
    test_context<Callback> context{str};
    auto                   input = lexy::zstring_input<test_encoding>(str);
    return Rule::template parser<test_context<Callback>>::parse(context, input);
}

#endif // TEST_RULE_VERIFY_HPP_INCLUDED

