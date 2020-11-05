// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/error.hpp>

#include "verify.hpp"

TEST_CASE("dsl::error")
{
    struct tag;

    SUBCASE("pattern")
    {
        constexpr auto pattern = lexy::dsl::error<tag>;
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(!abc);
    }
    SUBCASE("rule")
    {
        constexpr auto rule = lexy::dsl::error<tag>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int error(test_error<tag> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == -1);
    }
    SUBCASE("range")
    {
        constexpr auto rule = lexy::dsl::error<tag>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int error(test_error<tag> e)
            {
                if (e.begin() != e.end())
                {
                    CONSTEXPR_CHECK(e.begin() == str);
                    CONSTEXPR_CHECK(e.end() == lexy::_detail::string_view(str).end());
                    return -1;
                }
                else
                {
                    CONSTEXPR_CHECK(e.position() == str);
                    return -2;
                }
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -2);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == -1);
    }
}

TEST_CASE("dsl::require")
{
    struct tag;
    constexpr auto rule = lexy::dsl::require<tag>(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(abc);
        CHECK(abc.match().empty());
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }

            constexpr int error(test_error<tag> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
    }
}

TEST_CASE("dsl::prevent")
{
    struct tag;
    constexpr auto rule = lexy::dsl::prevent<tag>(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_pattern<decltype(rule)>);

    SUBCASE("pattern")
    {
        constexpr auto empty = pattern_matches(rule, "");
        CHECK(empty);
        CHECK(empty.match().empty());

        constexpr auto abc = pattern_matches(rule, "abc");
        CHECK(!abc);
    }
    SUBCASE("rule")
    {
        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }

            constexpr int error(test_error<tag> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == -1);
    }
}

TEST_CASE("dsl::try_")
{
    struct error;

    SUBCASE("pattern")
    {
        constexpr auto pattern = lexy::dsl::try_<error>(LEXY_LIT("abc"));
        CHECK(lexy::is_pattern<decltype(pattern)>);

        constexpr auto empty = pattern_matches(pattern, "");
        CHECK(!empty);

        constexpr auto abc = pattern_matches(pattern, "abc");
        CHECK(abc);
        CHECK(abc.match() == "abc");
    }
    SUBCASE("rule")
    {
        constexpr auto rule = lexy::dsl::try_<error>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur - str == 3);
                return 0;
            }

            constexpr int error(test_error<error> e)
            {
                CONSTEXPR_CHECK(e.position() == str);
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto abc = rule_matches<callback>(rule, "abc");
        CHECK(abc == 0);
    }
}

