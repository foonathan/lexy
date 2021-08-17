// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/action/trace.hpp>

#include <doctest/doctest.h>
#include <iterator>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <string>

namespace
{
namespace dsl = lexy::dsl;

struct id
{
    static constexpr auto name = "id";
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha);
};

struct alphabet
{
    static constexpr auto name = "alphabet";
    static constexpr auto rule = dsl::peek(LEXY_LIT("ab")) >> LEXY_LIT("abcd");
};

struct number
{
    static constexpr auto name = "number";
    static constexpr auto rule = dsl::identifier(dsl::ascii::digit);
};

struct object
{
    static constexpr auto name = "object";

    struct unexpected
    {
        static constexpr auto name = "unexpected";
    };

    static constexpr auto rule
        = dsl::p<alphabet> | dsl::p<id> | dsl::p<number> | dsl::try_(dsl::error<unexpected>);
};

struct production
{
    static constexpr auto name       = "production";
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto greeting = LEXY_LIT("Hello");
        return greeting + LEXY_DEBUG("greeting") + dsl::p<object>;
    }();
};
} // namespace

TEST_CASE("trace")
{
    auto trace = [](const char* input, lexy::visualization_options opts) {
        std::string str;
        lexy::trace_to<production>(std::back_insert_iterator(str), lexy::zstring_input(input),
                                   opts);
        return str;
    };

    SUBCASE("default flags")
    {
        auto opts = lexy::visualization_options();

        CHECK(trace("Hello abcd", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet:
 1:  7:     - backtracked: ab
 1:  7:     - token: abcd
 1: 11:     - finish
 1: 11:   - finish
 1: 11: - finish
)");
        CHECK(trace("Hello ax", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet:
 1:  7:     - backtracked: a
 1:  7:     -x
 1:  7:   - id:
 1:  7:     - identifier: ax
 1:  9:     - finish
 1:  9:   - finish
 1:  9: - finish
)");
        CHECK(trace("Hello name", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet:
 1:  7:     -x
 1:  7:   - id:
 1:  7:     - identifier: name
 1: 11:     - finish
 1: 11:   - finish
 1: 11: - finish
)");
        CHECK(trace("Hello 123", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet:
 1:  7:     -x
 1:  7:   - id:
 1:  7:     -x
 1:  7:   - number:
 1:  7:     - identifier: 123
 1: 10:     - finish
 1: 10:   - finish
 1: 10: - finish
)");

        CHECK(trace("Hello", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - debug: greeting
 1:  6: - object:
 1:  6:   - alphabet:
 1:  6:     -x
 1:  6:   - id:
 1:  6:     -x
 1:  6:   - number:
 1:  6:     -x
 1:  6:   - error: unexpected
 1:  6:   - finish
 1:  6: - finish
)");
        CHECK(trace("Hello abc", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet:
 1:  7:     - backtracked: ab
 1:  7:     - error: expected 'abcd'
 1: 10:     -x
 1: 10:   -x
 1: 10: -x
)");
    }
    SUBCASE("unicode")
    {
        auto opts = lexy::visualization_options{lexy::visualize_use_unicode};

        CHECK(trace("Hello abcd", opts) == R"( 1:  1: production:
 1:  1: ├──token: Hello
 1:  6: ├──whitespace: ⟨SP⟩
 1:  7: ├──debug: greeting
 1:  7: ├──object:
 1:  7: │  ├──alphabet:
 1:  7: │  │  ├──backtracked: ab
 1:  7: │  │  ├──token: abcd
 1: 11: │  │  ┴
 1: 11: │  ┴
 1: 11: ┴
)");
        CHECK(trace("Hello ax", opts) == R"( 1:  1: production:
 1:  1: ├──token: Hello
 1:  6: ├──whitespace: ⟨SP⟩
 1:  7: ├──debug: greeting
 1:  7: ├──object:
 1:  7: │  ├──alphabet:
 1:  7: │  │  ├──backtracked: a
 1:  7: │  │  └╳
 1:  7: │  ├──id:
 1:  7: │  │  ├──identifier: ax
 1:  9: │  │  ┴
 1:  9: │  ┴
 1:  9: ┴
)");
        CHECK(trace("Hello name", opts) == R"( 1:  1: production:
 1:  1: ├──token: Hello
 1:  6: ├──whitespace: ⟨SP⟩
 1:  7: ├──debug: greeting
 1:  7: ├──object:
 1:  7: │  ├──alphabet:
 1:  7: │  │  └╳
 1:  7: │  ├──id:
 1:  7: │  │  ├──identifier: name
 1: 11: │  │  ┴
 1: 11: │  ┴
 1: 11: ┴
)");
        CHECK(trace("Hello 123", opts) == R"( 1:  1: production:
 1:  1: ├──token: Hello
 1:  6: ├──whitespace: ⟨SP⟩
 1:  7: ├──debug: greeting
 1:  7: ├──object:
 1:  7: │  ├──alphabet:
 1:  7: │  │  └╳
 1:  7: │  ├──id:
 1:  7: │  │  └╳
 1:  7: │  ├──number:
 1:  7: │  │  ├──identifier: 123
 1: 10: │  │  ┴
 1: 10: │  ┴
 1: 10: ┴
)");

        CHECK(trace("Hello", opts) == R"( 1:  1: production:
 1:  1: ├──token: Hello
 1:  6: ├──debug: greeting
 1:  6: ├──object:
 1:  6: │  ├──alphabet:
 1:  6: │  │  └╳
 1:  6: │  ├──id:
 1:  6: │  │  └╳
 1:  6: │  ├──number:
 1:  6: │  │  └╳
 1:  6: │  ├──error: unexpected
 1:  6: │  ┴
 1:  6: ┴
)");
        CHECK(trace("Hello abc", opts) == R"( 1:  1: production:
 1:  1: ├──token: Hello
 1:  6: ├──whitespace: ⟨SP⟩
 1:  7: ├──debug: greeting
 1:  7: ├──object:
 1:  7: │  ├──alphabet:
 1:  7: │  │  ├──backtracked: ab
 1:  7: │  │  ├──error: expected 'abcd'
 1: 10: │  │  └╳
 1: 10: │  └╳
 1: 10: └╳
)");
    }

    SUBCASE("depth-limited")
    {
        auto opts = lexy::visualization_options{{}, 2};

        CHECK(trace("Hello abcd", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet: ...
 1: 11:   - finish
 1: 11: - finish
)");
        CHECK(trace("Hello name", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet: ...
 1:  7:   - id: ...
 1: 11:   - finish
 1: 11: - finish
)");
        CHECK(trace("Hello 123", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet: ...
 1:  7:   - id: ...
 1:  7:   - number: ...
 1: 10:   - finish
 1: 10: - finish
)");

        CHECK(trace("Hello", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - debug: greeting
 1:  6: - object:
 1:  6:   - alphabet: ...
 1:  6:   - id: ...
 1:  6:   - number: ...
 1:  6:   - error: unexpected
 1:  6:   - finish
 1:  6: - finish
)");
        CHECK(trace("Hello abc", opts) == R"( 1:  1: production:
 1:  1: - token: Hello
 1:  6: - whitespace: \u0020
 1:  7: - debug: greeting
 1:  7: - object:
 1:  7:   - alphabet: ...
 1: 10:   -x
 1: 10: -x
)");
    }
}

