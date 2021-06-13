// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_counter.hpp>

#include "verify.hpp"
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/label.hpp>

TEST_CASE("dsl::context_counter")
{
    struct error;

    static constexpr auto counter = lexy::dsl::context_counter<struct counter_id>;

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*)
        {
            return 0;
        }
        LEXY_VERIFY_FN int success(const char*, int sign)
        {
            return sign;
        }

        LEXY_VERIFY_FN int error(test_error<error> error)
        {
            LEXY_VERIFY_CHECK(error.position() == str);
            return -2;
        }
        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
        {
            return -3;
        }
    };

    SUBCASE("create - 0")
    {
        static constexpr auto rule = counter.create() + counter.require<0>().error<error>;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("create - 42")
    {
        static constexpr auto rule = counter.create<42>() + counter.require<42>().error<error>;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }

    SUBCASE("inc")
    {
        static constexpr auto rule
            = counter.create() + counter.inc() + counter.inc() + counter.require<2>().error<error>;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("dec")
    {
        static constexpr auto rule
            = counter.create() + counter.dec() + counter.dec() + counter.require<-2>().error<error>;

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }

    SUBCASE("push")
    {
        static constexpr auto rule
            = counter.create() + counter.push(LEXY_LIT("abc")) + counter.require<3>().error<error>;

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 0);
    }
    SUBCASE("pop")
    {
        static constexpr auto rule
            = counter.create() + counter.pop(LEXY_LIT("abc")) + counter.require<-3>().error<error>;

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 0);
    }

    static constexpr auto compare = counter.compare<0>(label<-1>, label<0>, label<1>);
    SUBCASE("compare - less")
    {
        static constexpr auto rule = counter.create<-1>() + compare;

        auto result = LEXY_VERIFY("abc");
        CHECK(result.success(-1));
    }
    SUBCASE("compare - equal")
    {
        static constexpr auto rule = counter.create<0>() + compare;

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 0);
    }
    SUBCASE("compare - greater")
    {
        static constexpr auto rule = counter.create<1>() + compare;

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 1);
    }

    SUBCASE("require failed")
    {
        static constexpr auto rule = counter.create() + counter.require<1>().error<error>;

        auto result = LEXY_VERIFY("");
        CHECK(result == -2);
    }
}

