// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/literal.hpp>

#include "verify.hpp"

TEST_CASE("dsl::lit")
{
    constexpr auto rule = LEXY_LIT("abc");
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(std::is_same_v<decltype(rule), decltype(lexy::dsl::lit<"abc">)>);
#endif

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            CONSTEXPR_CHECK(str + 3 == cur);
            return 0;
        }

        constexpr int error(test_error<lexy::expected_literal> e)
        {
            CONSTEXPR_CHECK(e.position() == str);
            CONSTEXPR_CHECK(e.string() == "abc");
            CONSTEXPR_CHECK(e.string()[e.index()] == e.character());
            return -int(e.index()) - 1;
        }
    };

    constexpr auto empty = verify<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto a = verify<callback>(rule, "a");
    CHECK(a == -2);
    constexpr auto ab = verify<callback>(rule, "ab");
    CHECK(ab == -3);

    constexpr auto abc = verify<callback>(rule, "abc");
    CHECK(abc == 0);
    constexpr auto abcd = verify<callback>(rule, "abcd");
    CHECK(abcd == 0);
}

