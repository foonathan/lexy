// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/if.hpp>

#include "verify.hpp"

TEST_CASE("dsl::if_()")
{
    SUBCASE("rule")
    {
        constexpr auto rule = if_(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                if (cur == str)
                    return 0;
                else
                {
                    CONSTEXPR_CHECK(cur - str == 3);
                    return 1;
                }
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto success = verify<callback>(rule, "abc");
        CHECK(success == 1);

        constexpr auto partial = verify<callback>(rule, "ab");
        CHECK(partial == 0);
    }
    SUBCASE("branch")
    {
        constexpr auto rule = if_(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                if (cur == str)
                    return 0;
                else
                {
                    CONSTEXPR_CHECK(cur - str == 3);
                    return 1;
                }
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "bc");
                return -1;
            }
        };

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto success = verify<callback>(rule, "abc");
        CHECK(success == 1);

        constexpr auto condition = verify<callback>(rule, "a");
        CHECK(condition == -1);
        constexpr auto partial = verify<callback>(rule, "ab");
        CHECK(partial == -1);
    }
}

