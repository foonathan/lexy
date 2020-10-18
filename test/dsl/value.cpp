// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/value.hpp>

#include "verify.hpp"

TEST_CASE("dsl::value_c")
{
    constexpr auto rule = lexy::dsl::value_c<42>;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, int i)
        {
            assert(str == cur);
            return i;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 42);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 42);
}

namespace
{
constexpr int f()
{
    return 42;
}
} // namespace

TEST_CASE("dsl::value_f")
{
    constexpr auto rule = lexy::dsl::value_f<f>;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, int i)
        {
            assert(str == cur);
            return i;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 42);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 42);
}

TEST_CASE("dsl::value_t")
{
    constexpr auto rule = lexy::dsl::value_t<int>;
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(!lexy::is_pattern<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, int i)
        {
            assert(str == cur);
            return i;
        }
    };

    constexpr auto empty = rule_matches<callback>(rule, "");
    CHECK(empty == 0);

    constexpr auto string = rule_matches<callback>(rule, "abc");
    CHECK(string == 0);
}

