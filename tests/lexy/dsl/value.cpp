// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/value.hpp>

#include "verify.hpp"

TEST_CASE("dsl::value_c")
{
    static constexpr auto rule = lexy::dsl::value_c<42>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, int i)
        {
            CONSTEXPR_CHECK(str == cur);
            return i;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 42);

    auto string = LEXY_VERIFY("abc");
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
    static constexpr auto rule = lexy::dsl::value_f<f>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, int i)
        {
            CONSTEXPR_CHECK(str == cur);
            return i;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 42);

    auto string = LEXY_VERIFY("abc");
    CHECK(string == 42);
}

TEST_CASE("dsl::value_t")
{
    static constexpr auto rule = lexy::dsl::value_t<int>;
    CHECK(lexy::is_rule<decltype(rule)>);

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, int i)
        {
            CONSTEXPR_CHECK(str == cur);
            return i;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto string = LEXY_VERIFY("abc");
    CHECK(string == 0);
}

TEST_CASE("dsl::value_str")
{
    static constexpr auto rule = LEXY_VALUE_STR("abc");
    CHECK(lexy::is_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(std::is_same_v<decltype(rule), decltype(lexy::dsl::value_str<"abc">)>);
#endif

    struct callback
    {
        const char* str;

        constexpr int success(const char* cur, const char* value, std::size_t length)
        {
            CONSTEXPR_CHECK(str == cur);
            CONSTEXPR_CHECK(lexy::_detail::string_view(value, length) == "abc");
            return 0;
        }
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty == 0);

    auto string = LEXY_VERIFY("abc");
    CHECK(string == 0);
}

