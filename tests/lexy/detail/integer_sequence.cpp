// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/_detail/integer_sequence.hpp>

#include <doctest/doctest.h>

TEST_CASE("_detail::integer_sequence")
{
    using empty = lexy::_detail::index_sequence_for<>;
    CHECK(std::is_same_v<empty, lexy::_detail::index_sequence<>>);

    using one = lexy::_detail::index_sequence_for<int>;
    CHECK(std::is_same_v<one, lexy::_detail::index_sequence<0>>);

    using two = lexy::_detail::index_sequence_for<int, int>;
    CHECK(std::is_same_v<two, lexy::_detail::index_sequence<0, 1>>);

    using three = lexy::_detail::index_sequence_for<int, int, int>;
    CHECK(std::is_same_v<three, lexy::_detail::index_sequence<0, 1, 2>>);
}

