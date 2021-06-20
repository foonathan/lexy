// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/eof.hpp>

#include "verify.hpp"

TEST_CASE("dsl::eof")
{
    static constexpr auto rule = lexy::dsl::eof;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(cur == str);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("EOF"));
            return -1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto non_empty = LEXY_VERIFY("abc");
    CHECK(non_empty == -1);
}

