// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/member.hpp>

#include "verify.hpp"

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

        using callback = member_macro_callback;

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        constexpr auto string = verify<callback>(rule, "abc");
        CHECK(string == 0);
    }
    SUBCASE("macro")
    {
        constexpr auto rule = LEXY_MEM(member) = LEXY_LIT("abc");
        CHECK(lexy::is_rule<decltype(rule)>);

        using callback = member_macro_callback;

        constexpr auto empty = verify<callback>(rule, "");
        CHECK(empty == -1);

        // Not constexpr in C++17 due to the use of reinterpret_cast in stateless lambda.
        /* constexpr */ auto string = verify<callback>(rule, "abc");
        CHECK(string == 0);
    }
}

