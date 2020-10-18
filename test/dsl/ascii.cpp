// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/ascii.hpp>

#include "verify.hpp"
#include <cctype>

TEST_CASE("dsl::ascii::*")
{
    auto is = [](auto atom, char c) {
        auto result = atom_matches(atom, &c, 1);
        if (result)
        {
            REQUIRE(result.count == 1);
            return true;
        }
        else
        {
            REQUIRE(result.count == 0);
            REQUIRE(result.error.position() == result.input);
            return false;
        }
    };

    auto verify = [&](char c) {
        using namespace lexyd::ascii;

        auto code = int(c);
        INFO(code << " '" << c << "'");
        REQUIRE(is(character, c));

        auto category_count = is(control, c) + is(blank, c) + is(newline, c) + is(other_space, c)
                              + is(digit, c) + is(lower, c) + is(upper, c) + is(punct, c);
        REQUIRE(category_count == 1);

        if (is(blank, c) || is(newline, c) || is(other_space, c))
            REQUIRE(is(space, c));
        else
            REQUIRE(!is(space, c));

        if (is(lower, c) || is(upper, c))
            REQUIRE(is(alpha, c));
        else
            REQUIRE(!is(alpha, c));

        if (is(alpha, c) || is(digit, c))
            REQUIRE(is(alnum, c));
        else
            REQUIRE(!is(alnum, c));

        if (is(alnum, c) || is(punct, c))
            REQUIRE(is(graph, c));
        else
            REQUIRE(!is(graph, c));

        if (is(graph, c) || c == ' ')
            REQUIRE(is(print, c));
        else
            REQUIRE(!is(print, c));

        REQUIRE(is(blank, c) == !!std::isblank(c));
        REQUIRE(is(digit, c) == !!std::isdigit(c));
        REQUIRE(is(lower, c) == !!std::islower(c));
        REQUIRE(is(upper, c) == !!std::isupper(c));
        REQUIRE(is(punct, c) == !!std::ispunct(c));
        REQUIRE(is(space, c) == !!std::isspace(c));
        REQUIRE(is(alpha, c) == !!std::isalpha(c));
        REQUIRE(is(alnum, c) == !!std::isalnum(c));
        REQUIRE(is(graph, c) == !!std::isgraph(c));
        REQUIRE(is(print, c) == !!std::isprint(c));
    };

    for (auto c = 0; c <= 127; ++c)
        verify(static_cast<char>(c));
}

