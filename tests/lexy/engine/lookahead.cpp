// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/engine/lookahead.hpp>

#include "verify.hpp"
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/engine/literal.hpp>

namespace
{
constexpr auto needle_trie = lexy::linear_trie<LEXY_NTTP_STRING("<!>")>;
constexpr auto end_trie    = lexy::linear_trie<LEXY_NTTP_STRING("\n")>;
} // namespace

TEST_CASE("engine_lookahead")
{
    using engine
        = lexy::engine_lookahead<lexy::engine_literal<needle_trie>, lexy::engine_literal<end_trie>>;
    CHECK(lexy::engine_is_matcher<engine>);

    auto empty = engine_matches<engine>("");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.ec == engine::error_code::not_found_eof);

    auto abc = engine_matches<engine>("abc");
    CHECK(!abc);
    CHECK(abc.count == 0);
    CHECK(abc.ec == engine::error_code::not_found_eof);
    auto abc_end = engine_matches<engine>("abc\n");
    CHECK(!abc_end);
    CHECK(abc_end.count == 0);
    CHECK(abc_end.ec == engine::error_code::not_found_end);

    auto needle = engine_matches<engine>("<!>");
    CHECK(needle);
    CHECK(needle.count == 0);
    auto needle_end = engine_matches<engine>("<!>\n");
    CHECK(needle_end);
    CHECK(needle_end.count == 0);

    auto abc_needle_abc = engine_matches<engine>("abc<!>abc");
    CHECK(abc_needle_abc);
    CHECK(abc_needle_abc.count == 0);
    auto abc_needle_abc_end = engine_matches<engine>("abc<!>abc\n");
    CHECK(abc_needle_abc_end);
    CHECK(abc_needle_abc_end.count == 0);

    auto abc_end_needle = engine_matches<engine>("abc\n<!>");
    CHECK(!abc_end_needle);
    CHECK(abc_end_needle.count == 0);
    CHECK(abc_end_needle.ec == engine::error_code::not_found_end);
}

