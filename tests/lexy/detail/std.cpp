// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/_detail/std.hpp>

#include <iterator>

#include <doctest/doctest.h>

TEST_CASE("std forward declarations")
{
    // We just create them and see whether it's ambiguous.
    (void)std::forward_iterator_tag{};
    (void)std::bidirectional_iterator_tag{};
}

