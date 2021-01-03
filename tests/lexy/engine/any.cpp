// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/any.hpp>

#include "verify.hpp"

TEST_CASE("engine_any")
{
    using engine = lexy::engine_any;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(empty);
    CHECK(empty.count == 0);

    auto abc = engine_matches<engine>("abc");
    CHECK(abc);
    CHECK(abc.count == 3);
}

