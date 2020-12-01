// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/validate.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/input/string_input.hpp>

#include "dsl/verify.hpp"

namespace
{
struct prod_a
{
    static constexpr auto rule = list(LEXY_LIT("abc"));
};
struct prod_b
{
    static constexpr auto rule = LEXY_LIT("(") + capture(lexy::dsl::p<prod_a>) + LEXY_LIT(")");
};
} // namespace

TEST_CASE("validate")
{
    SUBCASE("void callback")
    {
        SUBCASE("success")
        {
            constexpr auto callback = [](auto, auto) { FAIL_CHECK("should not be called"); };

            auto one
                = lexy::validate<prod_b>(lexy::zstring_input("(abc)"), lexy::callback(callback));
            CHECK(one);
            auto two
                = lexy::validate<prod_b>(lexy::zstring_input("(abcabc)"), lexy::callback(callback));
            CHECK(two);
        }
        SUBCASE("missing abc")
        {
            constexpr auto callback = [](auto ctx, auto error) {
                CHECK(ctx.production() == "prod_a");
                CHECK(*error.position() == ')');
                CHECK(error.string() == "abc");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("()"), lexy::callback(callback));
            CHECK(!result);
        }
        SUBCASE("invalid abc")
        {
            constexpr auto callback = [](auto ctx, auto error) {
                CHECK(ctx.production() == "prod_a");
                CHECK(*error.position() == 'a');
                CHECK(error.string() == "abc");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(adc)"), lexy::callback(callback));
            CHECK(!result);
        }
        SUBCASE("missing )")
        {
            constexpr auto callback = [](auto ctx, auto error) {
                CHECK(ctx.production() == "prod_b");
                CHECK(*error.position() == ']');
                CHECK(error.string() == ")");
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(abc]"), lexy::callback(callback));
            CHECK(!result);
        }
    }
    SUBCASE("non-void callback")
    {
        constexpr auto prod_a_error = [](lexy::string_error_context<prod_a>,
                                         lexy::string_error<lexy::expected_literal> error) {
            CONSTEXPR_CHECK(error.string() == "abc");
            return -1;
        };
        constexpr auto prod_b_error = [](lexy::string_error_context<prod_b>,
                                         lexy::string_error<lexy::expected_literal> error) {
            if (error.string() == "(")
                return -2;
            else if (error.string() == ")")
                return -3;
            else
                return -4;
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

