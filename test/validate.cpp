// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/validate.hpp>

#include <doctest.h>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/input/string_input.hpp>

TEST_CASE("validate")
{
    struct prod_a
    {
        LEXY_CONSTEVAL auto rule()
        {
            return LEXY_LIT("abc");
        }
    };
    struct prod_b
    {
        LEXY_CONSTEVAL auto rule()
        {
            return LEXY_LIT("(") + capture(lexy::dsl::p<prod_a>) + LEXY_LIT(")");
        }
    };

    SUBCASE("success")
    {
        auto callback
            = [](auto, const lexy::string_input<>&, auto) { FAIL_CHECK("should not be called"); };

        auto result = lexy::validate<prod_b>(lexy::zstring_input("(abc)"), callback);
        CHECK(result);
    }
    SUBCASE("missing abc")
    {
        auto callback = [](auto prod, const lexy::string_input<>& input, auto error) {
            CHECK(std::is_same_v<decltype(prod), prod_a>);
            CHECK(input.peek() == ')');
            CHECK(error.string() == "abc");
        };

        auto result = lexy::validate<prod_b>(lexy::zstring_input("()"), callback);
        CHECK(!result);
    }
    SUBCASE("invalid abc")
    {
        auto callback = [](auto prod, const lexy::string_input<>& input, auto error) {
            CHECK(std::is_same_v<decltype(prod), prod_a>);
            CHECK(input.peek() == 'd');
            CHECK(error.string() == "abc");
        };

        auto result = lexy::validate<prod_b>(lexy::zstring_input("(adc)"), callback);
        CHECK(!result);
    }
    SUBCASE("missing )")
    {
        auto callback = [](auto prod, const lexy::string_input<>& input, auto error) {
            CHECK(std::is_same_v<decltype(prod), prod_b>);
            CHECK(input.peek() == ']');
            CHECK(error.string() == ")");
        };

        auto result = lexy::validate<prod_b>(lexy::zstring_input("(abc]"), callback);
        CHECK(!result);
    }
}

