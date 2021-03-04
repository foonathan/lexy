// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/_detail/lazy_init.hpp>

#include <doctest/doctest.h>
#include <string>

TEST_CASE("_detail::lazy_init")
{
    SUBCASE("trivial")
    {
        using lazy_init = lexy::_detail::lazy_init<int>;

        constexpr auto empty = lazy_init();
        CHECK(!empty);

        constexpr auto emplaced = [] {
            lazy_init result;
            result.emplace(42);
            return result;
        }();
        CHECK(emplaced);
        CHECK(*emplaced == 42);
    }
    SUBCASE("non-trivial")
    {
        using lazy_init = lexy::_detail::lazy_init<std::string>;

        auto empty = lazy_init();
        CHECK(!empty);

        auto emplaced = [] {
            lazy_init result;
            result.emplace(5u, 'a');
            return result;
        }();
        CHECK(emplaced);
        CHECK(*emplaced == "aaaaa");
        CHECK(emplaced->size() == 5);

        SUBCASE("copy constructor from empty")
        {
            auto copy = empty;
            CHECK(!copy);
        }
        SUBCASE("copy constructor from emplaced")
        {
            auto copy = emplaced;
            CHECK(copy);
            CHECK(*copy == "aaaaa");
            CHECK(copy->size() == 5);
        }

        SUBCASE("move constructor from empty")
        {
            auto copy = LEXY_MOV(empty);
            CHECK(!copy);
        }
        SUBCASE("move constructor from emplaced")
        {
            auto copy = LEXY_MOV(emplaced);
            CHECK(copy);
            CHECK(*copy == "aaaaa");
            CHECK(copy->size() == 5);
        }

        SUBCASE("copy assignment from empty to empty")
        {
            lazy_init assigned;

            assigned = empty;
            CHECK(!assigned);
        }
        SUBCASE("copy assignment from emplaced to empty")
        {
            lazy_init assigned;

            assigned = emplaced;
            CHECK(assigned);
            CHECK(*assigned == "aaaaa");
            CHECK(assigned->size() == 5);
        }
        SUBCASE("copy assignment from empty to emplaced")
        {
            lazy_init assigned;
            assigned.emplace(3u, 'b');

            assigned = empty;
            CHECK(!assigned);
        }
        SUBCASE("copy assignment from emplaced to emplaced")
        {
            lazy_init assigned;
            assigned.emplace(3u, 'b');

            assigned = emplaced;
            CHECK(assigned);
            CHECK(*assigned == "aaaaa");
            CHECK(assigned->size() == 5);
        }

        SUBCASE("move assignment from empty to empty")
        {
            lazy_init assigned;

            assigned = LEXY_MOV(empty);
            CHECK(!assigned);
        }
        SUBCASE("move assignment from emplaced to empty")
        {
            lazy_init assigned;

            assigned = LEXY_MOV(emplaced);
            CHECK(assigned);
            CHECK(*assigned == "aaaaa");
            CHECK(assigned->size() == 5);
        }
        SUBCASE("move assignment from empty to emplaced")
        {
            lazy_init assigned;
            assigned.emplace(3u, 'b');

            assigned = LEXY_MOV(empty);
            CHECK(!assigned);
        }
        SUBCASE("move assignment from emplaced to emplaced")
        {
            lazy_init assigned;
            assigned.emplace(3u, 'b');

            assigned = LEXY_MOV(emplaced);
            CHECK(assigned);
            CHECK(*assigned == "aaaaa");
            CHECK(assigned->size() == 5);
        }
    }
    SUBCASE("void")
    {
        using lazy_init = lexy::_detail::lazy_init<void>;

        constexpr auto empty = lazy_init();
        CHECK(!empty);

        constexpr auto emplaced = [] {
            lazy_init result;
            result.emplace();
            return result;
        }();
        CHECK(emplaced);
    }
}

