// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/any.hpp>

#include "verify.hpp"

TEST_CASE("dsl::any")
{
    static constexpr auto rule = lexy::dsl::any;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto non_empty = LEXY_VERIFY("abc");
    CHECK(non_empty == 3);
}

