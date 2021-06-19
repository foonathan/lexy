// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/context_counter.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::context_counter")
{
    static constexpr auto counter = lexy::dsl::context_counter<struct counter_id>;

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*)
        {
            return -1;
        }
        LEXY_VERIFY_FN int success(const char*, int value)
        {
            return value;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal>)
        {
            return -42;
        }
    };

    SUBCASE("create - 0")
    {
        static constexpr auto rule = counter.create() + counter.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("create - 42")
    {
        static constexpr auto rule = counter.create<42>() + counter.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 42);
    }

    SUBCASE("inc")
    {
        static constexpr auto rule
            = counter.create() + counter.inc() + counter.inc() + counter.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 2);
    }
    SUBCASE("dec")
    {
        static constexpr auto rule
            = counter.create() + counter.dec() + counter.dec() + counter.value();

        auto result = LEXY_VERIFY("");
        CHECK(result.success(-2));
    }

    SUBCASE("push")
    {
        static constexpr auto rule
            = counter.create() + counter.push(LEXY_LIT("abc")) + counter.value();

        auto result = LEXY_VERIFY("abc");
        CHECK(result == 3);
    }
    SUBCASE("pop")
    {
        static constexpr auto rule
            = counter.create() + counter.pop(LEXY_LIT("abc")) + counter.value();

        auto result = LEXY_VERIFY("abc");
        CHECK(result.success(-3));
    }

    SUBCASE("is_zero - true")
    {
        static constexpr auto rule = counter.create() + if_(counter.is_zero() >> counter.value());

        auto result = LEXY_VERIFY("");
        CHECK(result == 0);
    }
    SUBCASE("is_zero - false")
    {
        static constexpr auto rule
            = counter.create<42>() + if_(counter.is_zero() >> counter.value());

        auto result = LEXY_VERIFY("");
        CHECK(result.success(-1));
    }
}

TEST_CASE("dsl::equal_counts")
{
    static constexpr auto counter_a = lexy::dsl::context_counter<struct id_a>;
    static constexpr auto counter_b = lexy::dsl::context_counter<struct id_b>;
    static constexpr auto counter_c = lexy::dsl::context_counter<struct id_c>;

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN int success(const char*)
        {
            return -1;
        }
        LEXY_VERIFY_FN int success(const char*, int value)
        {
            return value;
        }

        LEXY_VERIFY_FN int error(test_error<lexy::unequal_counts> e)
        {
            LEXY_VERIFY_CHECK(e.position() == str);
            return -1;
        }
    };

    SUBCASE("equal")
    {
        static constexpr auto rule
            = counter_a.create<11>() + counter_b.create<11>() + counter_c.create<11>()
              + lexy::dsl::equal_counts(counter_a, counter_b, counter_c) + counter_a.value();

        auto result = LEXY_VERIFY("");
        CHECK(result == 11);
    }
    SUBCASE("not equal")
    {
        static constexpr auto rule
            = counter_a.create<11>() + counter_b.create<42>() + counter_c.create<11>()
              + lexy::dsl::equal_counts(counter_a, counter_b, counter_c) + counter_a.value();

        auto result = LEXY_VERIFY("");
        CHECK(result.value == 11);
        CHECK(result.errors(-1));
    }

    SUBCASE("equal branch")
    {
        static constexpr auto rule
            = counter_a.create<11>() + counter_b.create<11>() + counter_c.create<11>()
              + if_(lexy::dsl::equal_counts(counter_a, counter_b, counter_c) >> counter_a.value());

        auto result = LEXY_VERIFY("");
        CHECK(result == 11);
    }
    SUBCASE("not equal branch")
    {
        static constexpr auto rule
            = counter_a.create<11>() + counter_b.create<42>() + counter_c.create<11>()
              + if_(lexy::dsl::equal_counts(counter_a, counter_b, counter_c) >> counter_a.value());

        auto result = LEXY_VERIFY("");
        CHECK(result.success(-1));
    }
}

