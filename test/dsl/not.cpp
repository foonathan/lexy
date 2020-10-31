// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/not.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>

TEST_CASE("dsl::operator!")
{
    constexpr auto rule = !LEXY_LIT("abc");
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(!abc);
        CHECK(abc.match() == "abc");
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                assert(cur == str);
                return 0;
            }

            constexpr int error(test_error<lexy::unexpected> e)
            {
                assert(e.begin() == str);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == -1);
    }

    SUBCASE("try")
    {
        struct tag;
        constexpr auto try_rule = lexy::dsl::try_<tag>(rule);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                assert(cur == str);
                return 0;
            }

            constexpr int error(test_error<tag> e)
            {
                assert(e.begin() == str);
                assert(e.end() == lexy::_detail::string_view(str).end());
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(try_rule, "");
        CHECK(empty == 0);

        constexpr auto abc = rule_matches<callback>(try_rule, "abc");
        CHECK(abc == -1);
    }
}

