// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_flag.hpp>

#include "verify.hpp"
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>

TEST_CASE("dsl::context_flag")
{
    struct error;

    static constexpr auto flag = lexy::dsl::context_flag<struct flag_id>;

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
        static constexpr auto rule = flag.create() + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("create - true")
    {
        static constexpr auto rule = flag.create<true>() + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }

    SUBCASE("set")
    {
        static constexpr auto rule = flag.create() + flag.set() + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("reset")
    {
        static constexpr auto rule = flag.create<true>() + flag.reset() + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }

    SUBCASE("toggle - on")
    {
        static constexpr auto rule = flag.create() + flag.toggle() + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("toggle - off")
    {
        static constexpr auto rule = flag.create<true>() + flag.toggle() + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }

    SUBCASE("is_set - true")
    {
        static constexpr auto rule
            = flag.create<true>() + (flag.is_set() >> flag.reset() | lexy::dsl::else_ >> flag.set())
              + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("is_set - false")
    {
        static constexpr auto rule
            = flag.create<false>()
              + (flag.is_set() >> flag.reset() | lexy::dsl::else_ >> flag.set()) + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }

    SUBCASE("is_reset - true")
    {
        static constexpr auto rule
            = flag.create<false>()
              + (flag.is_reset() >> flag.set() | lexy::dsl::else_ >> flag.reset()) + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("is_reset - false")
    {
        static constexpr auto rule
            = flag.create<true>()
              + (flag.is_reset() >> flag.set() | lexy::dsl::else_ >> flag.reset()) + flag.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
}

