// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/parse.hpp>

#include "verify.hpp"
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>

namespace parse_state
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
} // namespace parse_state

TEST_CASE("rule: parse_state")
{
    using namespace parse_state;

    SUBCASE("different handler")
    {
        static constexpr auto rule = lexy::dsl::parse_state;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(str == cur);
                return 0;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto string = LEXY_VERIFY("abc");
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

namespace parse_state_member
{
struct state
{
    int i;
};

struct child_prod
{
    static constexpr auto rule  = lexy::dsl::parse_state_member<&state::i>;
    static constexpr auto value = lexy::callback([](int& i) { ++i; });
};

struct production
{
    static constexpr auto rule
        = lexy::dsl::p<
              child_prod> + lexy::dsl::p<child_prod> + lexy::dsl::parse_state_member<&state::i>;
    static constexpr auto value = lexy::construct<int>;
};
} // namespace parse_state_member

TEST_CASE("rule: parse_state_member")
{
    using namespace parse_state_member;

    SUBCASE("different handler")
    {
        static constexpr auto rule = lexy::dsl::parse_state_member<&state::i>;
        CHECK(lexy::is_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(str == cur);
                return 0;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == 0);

        auto string = LEXY_VERIFY("abc");
        CHECK(string == 0);
    }
    SUBCASE("parse rvalue")
    {
        auto result = lexy::parse<production>(lexy::zstring_input(""), state{0}, lexy::noop);
        CHECK(result);
        CHECK(result.value() == 2);
    }
    SUBCASE("parse lvalue")
    {
        auto state_obj = state{0};
        auto result    = lexy::parse<production>(lexy::zstring_input(""), state_obj, lexy::noop);
        CHECK(result);
        CHECK(result.value() == 2);
        CHECK(state_obj.i == 2);
    }
}

