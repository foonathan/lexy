// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_flag.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::context_flag")
{
    struct error;

    static constexpr auto flag = lexy::dsl::context_flag<struct id>;
    static constexpr auto get
        = [] { return flag.select(lexy::dsl::id<true>, lexy::dsl::id<false>); }();

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char* cur, bool value)
        {
            LEXY_VERIFY_CHECK(str == cur);
            return value;
        }

        LEXY_VERIFY_FN int error(test_error<error> error)
        {
            LEXY_VERIFY_CHECK(error.position() == str);
            return -1;
        }
    };

    SUBCASE("create - false")
    {
        static constexpr auto rule = flag.create() + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("create - true")
    {
        static constexpr auto rule = flag.create<true>() + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }

    SUBCASE("set")
    {
        static constexpr auto rule = flag.create() + flag.set() + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("reset")
    {
        static constexpr auto rule = flag.create<true>() + flag.reset() + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }

    SUBCASE("toggle - on")
    {
        static constexpr auto rule = flag.create() + flag.toggle() + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("toggle - off")
    {
        static constexpr auto rule = flag.create<true>() + flag.toggle() + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }

    SUBCASE("require - failed")
    {
        static constexpr auto rule = flag.create() + flag.require().error<error> + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == -1);
    }
    SUBCASE("require - pass")
    {
        static constexpr auto rule = flag.create() + flag.require<false>().error<error> + get;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
}

