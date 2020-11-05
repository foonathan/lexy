// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/label.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::label")
{
    SUBCASE("simple")
    {
        constexpr auto rule = lexy::dsl::label<struct lab>;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::label<lab>)
            {
                CONSTEXPR_CHECK(str == cur);
                return 0;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 0);
    }
    SUBCASE("operator()")
    {
        constexpr auto rule = lexy::dsl::label<struct lab>(LEXY_LIT("abc"));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur, lexy::label<lab>)
            {
                CONSTEXPR_CHECK(cur == str + 3);
                return 0;
            }

            constexpr int error(test_error<lexy::expected_literal> e)
            {
                CONSTEXPR_CHECK(e.string() == "abc");
                return -1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 0);
    }
    SUBCASE("branch")
    {
        constexpr auto rule = if_(lexy::dsl::label<struct lab>(LEXY_LIT("abc")));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                CONSTEXPR_CHECK(cur == str);
                return 0;
            }
            constexpr int success(const char* cur, lexy::label<lab>)
            {
                CONSTEXPR_CHECK(cur == str + 3);
                return 1;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 1);
    }
}

TEST_CASE("dsl::id")
{
    constexpr auto rule = lexy::dsl::id<0>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, lexy::id<0>)
        {
            static_assert(lexy::id<0>{} == 0);
            CONSTEXPR_CHECK(str == cur);
            return 0;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 0);
}

namespace
{
struct test_type
{
    int member;
};

struct member_macro_callback
{
    const char* str;

    template <typename Fn>
    constexpr int success(const char* cur, lexy::member<Fn>)
    {
        CONSTEXPR_CHECK(cur == str + 3);

        test_type tt{};
        Fn()(tt, 42);
        CONSTEXPR_CHECK(tt.member == 42);

        return 0;
    }

    constexpr int error(test_error<lexy::expected_literal> e)
    {
        CONSTEXPR_CHECK(e.string() == "abc");
        return -1;
    }
};
} // namespace

TEST_CASE("dsl::member")
{
    SUBCASE("non-macro")
    {
        constexpr auto rule = lexy::dsl::member<& test_type::member> = LEXY_LIT("abc");
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        using callback = member_macro_callback;

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 0);
    }
    SUBCASE("macro")
    {
        constexpr auto rule = LEXY_MEM(member) = LEXY_LIT("abc");
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        using callback = member_macro_callback;

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == -1);

        // Not constexpr in C++17 due to the use of reinterpret_cast in stateless lambda.
        /* constexpr */ auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 0);
    }
}

