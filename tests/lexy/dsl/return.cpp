// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/return.hpp>

#include "verify.hpp"

TEST_CASE("dsl::return")
{
    static constexpr auto rule = lexy::dsl::return_ + LEXY_LIT("abc");
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success() // No argument, as we're changing the final parser.
        {
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
}

