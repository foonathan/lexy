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
using test_error = typename Error::template error<lexy::string_input<test_encoding>>;

template <typename Callback>
struct test_context
{
    const char* str;

    using result_type = int;

    template <typename Error>
    constexpr int report_error(Error&& error)
    {
        return Callback{str}.error(LEXY_FWD(error));
    }

    template <typename... Args>
    static constexpr int parse(test_context& self, lexy::string_input<test_encoding> input,
                               Args&&... args)
    {
        return Callback{self.str}.success(input.cur(), LEXY_FWD(args)...);
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

