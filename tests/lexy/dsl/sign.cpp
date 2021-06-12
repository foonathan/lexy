// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/sign.hpp>

#include "verify.hpp"
#include <lexy/dsl/eof.hpp>

TEST_CASE("dsl::plus_sign")
{
    static constexpr auto rule = lexy::dsl::plus_sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*)
        {
            return 0;
        }
        LEXY_VERIFY_FN int success(const char*, lexy::plus_sign sign)
        {
            return sign;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return -2;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.success(0));
    auto plus = LEXY_VERIFY("+");
    CHECK(plus.success(+1));
    auto minus = LEXY_VERIFY("-");
    CHECK(minus.fatal(-2));

    auto other = LEXY_VERIFY("a");
    CHECK(other.fatal(-2));
}

TEST_CASE("dsl::minus_sign")
{
    static constexpr auto rule = lexy::dsl::minus_sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*)
        {
            return 0;
        }
        LEXY_VERIFY_FN int success(const char*, lexy::minus_sign sign)
        {
            return sign;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return -2;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.success(0));
    auto plus = LEXY_VERIFY("+");
    CHECK(plus.fatal(-2));
    auto minus = LEXY_VERIFY("-");
    CHECK(minus.success(-1));

    auto other = LEXY_VERIFY("a");
    CHECK(other.fatal(-2));
}

TEST_CASE("dsl::sign")
{
    static constexpr auto rule = lexy::dsl::sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(*cur == '\0');
            return 0;
        }
        LEXY_VERIFY_FN int success(const char* cur, lexy::plus_sign sign)
        {
            LEXY_VERIFY_CHECK(*cur == '\0');
            return sign;
        }
        LEXY_VERIFY_FN int success(const char* cur, lexy::minus_sign sign)
        {
            LEXY_VERIFY_CHECK(*cur == '\0');
            return sign;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return -2;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.success(0));
    auto plus = LEXY_VERIFY("+");
    CHECK(plus.success(+1));
    auto minus = LEXY_VERIFY("-");
    CHECK(minus.success(-1));

    auto other = LEXY_VERIFY("a");
    CHECK(other.fatal(-2));
}

