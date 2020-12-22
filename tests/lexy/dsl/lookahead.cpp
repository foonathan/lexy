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
    constexpr auto rule = if_(lookahead(LEXY_LIT("!"), LEXY_LIT("\n")) >> lexy::dsl::error<error>);
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            CONSTEXPR_CHECK(cur == str);
            return -1;
        }

        constexpr int error(test_error<error> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto nothing = rule_matches<callback>(rule, "abc");
    CHECK(nothing == -1);

    constexpr auto nothing_newline = rule_matches<callback>(rule, "abc\n");
    CHECK(nothing_newline == -1);

    constexpr auto something = rule_matches<callback>(rule, "abc!def\n");
    CHECK(something == 0);

    constexpr auto something_after = rule_matches<callback>(rule, "abc\n!def\n");
    CHECK(something_after == -1);
}

