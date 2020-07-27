// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/literal.hpp>

#include "verify.hpp"

TEST_CASE("atom: lit")
{
    constexpr auto atom = LEXY_LIT("abc");
    CHECK(lexy::is_atom<decltype(atom)>);
#if LEXY_HAS_NTTP
    CHECK(std::is_same_v<decltype(atom), lexy::dsl::lit<"abc">>);
#endif

    constexpr auto empty = atom_matches(atom, "");
    CHECK(!empty);
    CHECK(empty.count == 0);
    CHECK(empty.error.position() == empty.input);
    CHECK(empty.error.string() == "abc");
    CHECK(empty.error.character() == 'a');

    constexpr auto partial = atom_matches(atom, "ab");
    CHECK(!partial);
    CHECK(partial.count == 2);
    CHECK(partial.error.position() == partial.input);
    CHECK(partial.error.string() == "abc");
    CHECK(partial.error.character() == 'c');

    constexpr auto correct = atom_matches(atom, "abc");
    CHECK(correct);
    CHECK(correct.count == 3);

    constexpr auto extra = atom_matches(atom, "abcdef");
    CHECK(extra);
    CHECK(extra.count == 3);
}

