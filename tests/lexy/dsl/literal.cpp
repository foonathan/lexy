// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/literal.hpp>

#include "verify.hpp"

TEST_CASE("dsl::lit")
{
    static constexpr auto rule = LEXY_LIT("abc");
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(lexy::is_token_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(std::is_same_v<decltype(rule), decltype(lexy::dsl::lit<"abc">)>);
#endif

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur)
        {
            LEXY_VERIFY_CHECK(str + 3 == cur);
            return 0;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            LEXY_VERIFY_CHECK(e.string() == lexy::_detail::string_view("abc"));
            LEXY_VERIFY_CHECK(e.string()[e.index()] == e.character());
            return -int(e.index()) - 1;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == -1);

    auto a = LEXY_VERIFY("a");
    CHECK(a == -2);
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab == -3);

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc == 0);
    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd == 0);
}

