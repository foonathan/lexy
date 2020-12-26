// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/lookahead.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::lookahead()")
{
    struct error
    {};
    static constexpr auto rule
        = if_(lookahead(LEXY_LIT("!"), LEXY_LIT("\n")) >> lexy::dsl::error<error>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(cur == str);
            return -1;
        }

        LEXY_VERIFY_FN int error(test_error<error> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto nothing = LEXY_VERIFY("abc");
    CHECK(nothing == -1);

    auto nothing_newline = LEXY_VERIFY("abc\n");
    CHECK(nothing_newline == -1);

    auto something = LEXY_VERIFY("abc!def\n");
    CHECK(something == 0);

    auto something_after = LEXY_VERIFY("abc\n!def\n");
    CHECK(something_after == -1);
}

