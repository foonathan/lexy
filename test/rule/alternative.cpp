// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/alternative.hpp>

#include "verify.hpp"

TEST_CASE("rule: alternative")
{
    constexpr auto rule = LEXY_LIT("abc") / LEXY_LIT("a") / LEXY_LIT("ab") / LEXY_LIT("def");
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur)
        {
            auto match = lexy::_detail::string_view(str, cur);
            if (match == "abc")
                return 0;
            else if (match == "a")
                return 1;
            else if (match == "def")
                return 2;
            else
                assert(false);
        }

        constexpr int error(test_error<lexy::exhausted_alternatives> e)
        {
            assert(e.position() == str);
            return -1;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == -1);

    constexpr auto abc = rule_matches<callback>(rule, "abc");
    CHECK(abc == 0);

    constexpr auto a = rule_matches<callback>(rule, "a");
    CHECK(a == 1);
    constexpr auto ab = rule_matches<callback>(rule, "ab");
    CHECK(ab == 1);

    constexpr auto def = rule_matches<callback>(rule, "def");
    CHECK(def == 2);
}

