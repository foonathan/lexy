// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/parse.hpp>

#include "verify.hpp"
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>

namespace
{
struct child_prod
{
    static constexpr auto rule  = lexy::dsl::parse_state;
    static constexpr auto value = lexy::callback([](int& i) { ++i; });
};

struct production
{
    static constexpr auto rule
        = lexy::dsl::p<child_prod> + lexy::dsl::p<child_prod> + lexy::dsl::parse_state;
    static constexpr auto value = lexy::construct<int>;
};
} // namespace

TEST_CASE("rule: parse_state")
{
    SUBCASE("different context")
    {
        constexpr auto rule = lexy::dsl::parse_state;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(!lexy::is_pattern<decltype(rule)>);

        struct callback
        {
            const char* str;

            constexpr int success(const char* cur)
            {
                assert(str == cur);
                return 0;
            }
        };

        constexpr auto empty = rule_matches<callback>(rule, "");
        CHECK(empty == 0);

        constexpr auto string = rule_matches<callback>(rule, "abc");
        CHECK(string == 0);
    }
    SUBCASE("parse rvalue")
    {
        auto result = lexy::parse<production>(lexy::zstring_input(""), 0, lexy::noop);
        CHECK(result);
        CHECK(result.value() == 2);
    }
    SUBCASE("parse lvalue")
    {
        auto state  = 0;
        auto result = lexy::parse<production>(lexy::zstring_input(""), state, lexy::noop);
        CHECK(result);
        CHECK(result.value() == 2);
        CHECK(state == 2);
    }
}
