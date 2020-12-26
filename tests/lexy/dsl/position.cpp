// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/position.hpp>

#include "verify.hpp"

TEST_CASE("dsl::position")
{
    static constexpr auto rule = lexy::dsl::position;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, const char* pos)
        {
            CONSTEXPR_CHECK(str == cur);
            CONSTEXPR_CHECK(cur == pos);
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto string = LEXY_VERIFY("abc");
    CHECK(string == 0);
}

