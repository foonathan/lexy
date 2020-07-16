// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/_detail/string_view.hpp>

#include <doctest.h>

TEST_CASE("string_view")
{
    lexy::_detail::string_view str = "abc";
    REQUIRE(str.size() == 3);
    REQUIRE(str == "abc");

    REQUIRE(str.substr(1) == "bc");
    REQUIRE(str.substr(1, 1) == "b");
    REQUIRE(str.substr(1, 24) == "bc");

    REQUIRE(str.find("bc") == 1);
    REQUIRE(str.find("a", 1) == std::size_t(-1));
    REQUIRE(str.find('b') == 1);
    REQUIRE(str.find('a', 1) == std::size_t(-1));
}

