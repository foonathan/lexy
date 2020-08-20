// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/validate.hpp>

#include <doctest.h>
#include <lexy/dsl/capture.hpp>
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

    SUBCASE("void callback")
    {
        SUBCASE("success")
        {
            constexpr auto callback = [](auto, const lexy::string_input<>&, auto) {
                FAIL_CHECK("should not be called");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(abc)"), lexy::callback(callback));
            CHECK(result);
        }
        SUBCASE("missing abc")
        {
            constexpr auto callback = [](auto prod, const lexy::string_input<>& input, auto error) {
                CHECK(std::is_same_v<decltype(prod), prod_a>);
                CHECK(input.peek() == ')');
                CHECK(error.string() == "abc");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("()"), lexy::callback(callback));
            CHECK(!result);
        }
        SUBCASE("invalid abc")
        {
            constexpr auto callback = [](auto prod, const lexy::string_input<>& input, auto error) {
                CHECK(std::is_same_v<decltype(prod), prod_a>);
                CHECK(input.peek() == 'd');
                CHECK(error.string() == "abc");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(adc)"), lexy::callback(callback));
            CHECK(!result);
        }
        SUBCASE("missing )")
        {
            constexpr auto callback = [](auto prod, const lexy::string_input<>& input, auto error) {
                CHECK(std::is_same_v<decltype(prod), prod_b>);
                CHECK(input.peek() == ']');
                CHECK(error.string() == ")");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(abc]"), lexy::callback(callback));
            CHECK(!result);
        }
    }
    SUBCASE("non-void callback")
    {
        constexpr auto prod_a_error = [](prod_a, const lexy::string_input<>&, auto error) {
            assert(error.string() == "abc");
            return -1;
        };
        constexpr auto prod_b_error = [](prod_b, const lexy::string_input<>&, auto error) {
            if (error.string() == "(")
                return -2;
            else if (error.string() == ")")
                return -3;

            assert(false);
        };
        constexpr auto callback = lexy::callback<int>(prod_a_error, prod_b_error);

        constexpr auto success = lexy::validate<prod_b>(lexy::zstring_input("(abc)"), callback);
        CHECK(success);

        constexpr auto missing_abc = lexy::validate<prod_b>(lexy::zstring_input("()"), callback);
        CHECK(!missing_abc);
        CHECK(missing_abc.error() == -1);

        constexpr auto empty = lexy::validate<prod_b>(lexy::zstring_input(""), callback);
        CHECK(!empty);
        CHECK(empty.error() == -2);
        constexpr auto bad_paren = lexy::validate<prod_b>(lexy::zstring_input("[abc]"), callback);
        CHECK(!bad_paren);
        CHECK(bad_paren.error() == -2);

        constexpr auto missing_paren
            = lexy::validate<prod_b>(lexy::zstring_input("(abc"), callback);
        CHECK(!missing_paren);
        CHECK(missing_paren.error() == -3);
    }
}

