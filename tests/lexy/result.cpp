// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/result.hpp>

#include <doctest/doctest.h>
#include <string>

TEST_CASE("result")
{
    auto check_value = [](const auto& res, auto value) {
        CHECK(res);
        CHECK(res.has_value());
        CHECK(!res.has_error());
        CHECK(res.value() == value);
    };
    auto check_error = [](const auto& res, auto error) {
        CHECK(!res);
        CHECK(!res.has_value());
        CHECK(res.has_error());
        CHECK(res.error() == error);
    };

    SUBCASE("trivial")
    {
        constexpr lexy::result<int, int> val(lexy::result_value, 42);
        check_value(val, 42);

        constexpr lexy::result<int, int> err(lexy::result_error, 42);
        check_error(err, 42);

        constexpr lexy::result<short, int> conv_err(err);
        check_error(conv_err, 42);

        constexpr auto conv_err_move = [&] {
            auto tmp = err;
            return lexy::result<short, int>(LEXY_MOV(tmp));
        }();
        check_error(conv_err_move, 42);
    }
    SUBCASE("non-trivial")
    {
        lexy::result<std::string, std::string> val(lexy::result_value,
                                                   "a somewhat long string against SSO");
        check_value(val, "a somewhat long string against SSO");

        lexy::result<std::string, std::string> err(lexy::result_error,
                                                   "a somewhat long string against SSO");
        check_error(err, "a somewhat long string against SSO");

        lexy::result<int, std::string> conv_err(err);
        check_error(conv_err, "a somewhat long string against SSO");

        auto conv_err_move = [&] {
            lexy::result<std::string, std::string> tmp(lexy::result_error,
                                                       "a somewhat long string against SSO");
            return lexy::result<int, std::string>(LEXY_MOV(tmp));
        }();
        check_error(conv_err_move, "a somewhat long string against SSO");

        SUBCASE("move ctor - value")
        {
            const auto moved = LEXY_MOV(val);
            check_value(moved, "a somewhat long string against SSO");
            check_value(val, "");
        }
        SUBCASE("move ctor - error")
        {
            const auto moved = LEXY_MOV(err);
            check_error(moved, "a somewhat long string against SSO");
            check_error(err, "");
        }
        SUBCASE("move assign - value = value")
        {
            lexy::result<std::string, std::string> val2(lexy::result_value,
                                                        "another somewhat long string, again SSO");
            check_value(val2, "another somewhat long string, again SSO");

            val2 = LEXY_MOV(val);
            check_value(val2, "a somewhat long string against SSO");
            check_value(val, "");
        }
        SUBCASE("move assign - value = error")
        {
            lexy::result<std::string, std::string> val2(lexy::result_value,
                                                        "another somewhat long string, again SSO");
            check_value(val2, "another somewhat long string, again SSO");

            val2 = LEXY_MOV(err);
            check_error(val2, "a somewhat long string against SSO");
            check_error(err, "");
        }
        SUBCASE("move assign - error = value")
        {
            lexy::result<std::string, std::string> err2(lexy::result_error,
                                                        "another somewhat long string, again SSO");
            check_error(err2, "another somewhat long string, again SSO");

            err2 = LEXY_MOV(val);
            check_value(err2, "a somewhat long string against SSO");
            check_value(val, "");
        }
        SUBCASE("move assign - error = error")
        {
            lexy::result<std::string, std::string> err2(lexy::result_error,
                                                        "another somewhat long string, again SSO");
            check_error(err2, "another somewhat long string, again SSO");

            err2 = LEXY_MOV(err);
            check_error(err2, "a somewhat long string against SSO");
            check_error(err, "");
        }
    }
}

TEST_CASE("result<int, void>")
{
    CHECK(lexy::result<int, void>::has_void_error());

    constexpr lexy::result<int, void> val(lexy::result_value, 42);
    CHECK(val);
    CHECK(val.has_value());
    CHECK(!val.has_error());
    CHECK(val.value() == 42);

    constexpr lexy::result<int, void> err(lexy::result_error);
    CHECK(!err);
    CHECK(!err.has_value());
    CHECK(err.has_error());
}

TEST_CASE("result<void, int>")
{
    CHECK(lexy::result<void, int>::has_void_value());

    constexpr lexy::result<void, int> val(lexy::result_value);
    CHECK(val);
    CHECK(val.has_value());
    CHECK(!val.has_error());

    constexpr lexy::result<void, int> err(lexy::result_error, 42);
    CHECK(!err);
    CHECK(!err.has_value());
    CHECK(err.has_error());
    CHECK(err.error() == 42);
}

