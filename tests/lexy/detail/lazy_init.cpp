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
        lexy::_detail::lazy_init<int> lazy;
        CHECK(!lazy);

        lazy.emplace(42);
        CHECK(lazy);
        CHECK(*lazy == 42);
    }
    SUBCASE("non-trivial")
    {
        lexy::_detail::lazy_init<std::string> lazy;
        CHECK(!lazy);

        lazy.emplace(5u, 'a');
        CHECK(lazy);
        CHECK(*lazy == "aaaaa");
        CHECK(lazy->size() == 5);
    }
    SUBCASE("void")
    {
        lexy::_detail::lazy_init<void> lazy;
        CHECK(!lazy);

        lazy.emplace();
        CHECK(lazy);
    }
}

