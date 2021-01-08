// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_counter.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/value.hpp>

TEST_CASE("dsl::context_counter")
{
    struct error;

    static constexpr auto counter = lexy::dsl::context_counter<struct id>;
    auto                  get     = [](auto check) {
        return check >> lexy::dsl::value_c<true> | lexy::dsl::else_ >> lexy::dsl::value_c<false>;
    };

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*, bool value)
        {
            return value;
        }
        LEXY_VERIFY_FN int success(const char*)
        {
            return 2;
        }

        LEXY_VERIFY_FN int error(test_error<error> error)
        {
            LEXY_VERIFY_CHECK(error.position() == str);
            return -1;
        }
        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
        {
            return -2;
        }
    };

    SUBCASE("declare - 0")
    {
        static constexpr auto rule = counter.create() + get(counter.check_eq<0>());

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("declare - 42")
    {
        static constexpr auto rule = counter.create<42>() + get(counter.check_eq<42>());

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }

    SUBCASE("inc")
    {
        static constexpr auto rule
            = counter.create() + counter.inc() + counter.inc() + get(counter.check_eq<2>());

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }
    SUBCASE("dec")
    {
        static constexpr auto rule
            = counter.create() + counter.dec() + counter.dec() + get(counter.check_eq<-2>());

        auto result = LEXY_VERIFY("");
        CHECK(result == 1);
    }

    SUBCASE("push")
    {
        static constexpr auto rule
            = counter.create() + counter.push(LEXY_LIT("abc")) + get(counter.check_eq<3>());

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 1);
    }
    SUBCASE("pop")
    {
        static constexpr auto rule
            = counter.create() + counter.pop(LEXY_LIT("abc")) + get(counter.check_eq<-3>());

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 1);
    }

    SUBCASE("require - failed")
    {
        static constexpr auto rule = counter.create<1>() + counter.require<error>();

        auto result = LEXY_VERIFY("");
        CHECK(result == -1);
    }
    SUBCASE("require - pass")
    {
        static constexpr auto rule = counter.create<1>() + counter.require<1, error>();

        auto result = LEXY_VERIFY("");
        CHECK(result == 2);
    }
}

