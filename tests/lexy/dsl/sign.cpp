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

        LEXY_VERIFY_FN int success(const char*, int i)
        {
            return i;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == +1);
    auto plus = LEXY_VERIFY("+");
    CHECK(plus == +1);
    auto minus = LEXY_VERIFY("-");
    CHECK(minus == 0);
}

TEST_CASE("dsl::minus_sign")
{
    static constexpr auto rule = lexy::dsl::minus_sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*, int i)
        {
            return i;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == +1);
    auto plus = LEXY_VERIFY("+");
    CHECK(plus == 0);
    auto minus = LEXY_VERIFY("-");
    CHECK(minus == -1);
}

TEST_CASE("dsl::sign")
{
    static constexpr auto rule = lexy::dsl::sign + lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur, int i)
        {
            LEXY_VERIFY_CHECK(*cur == '\0');
            return i;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == +1);
    auto plus = LEXY_VERIFY("+");
    CHECK(plus == +1);
    auto minus = LEXY_VERIFY("-");
    CHECK(minus == -1);
}

