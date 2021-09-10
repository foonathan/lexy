// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/action/parse_as_tree.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/parse_tree_doctest.hpp>

namespace
{
enum class token_kind
{
    a,
    b,
    c,
};

const char* token_kind_name(token_kind k)
{
    switch (k)
    {
    case token_kind::a:
        return "a";
    case token_kind::b:
        return "b";
    case token_kind::c:
        return "c";
    }

    return "";
}

struct string_p : lexy::token_production
{
    static constexpr auto name = "string_p";
    static constexpr auto rule = lexy::dsl::quoted(lexy::dsl::ascii::character);
};

struct child_p
{
    static constexpr auto name = "child_p";
    static constexpr auto rule
        = lexy::dsl::p<string_p> | lexy::dsl::parenthesized(LEXY_LIT("abc").kind<token_kind::c>);
};

struct root_p
{
    static constexpr auto name = "root_p";
    static constexpr auto whitespace
        = lexy::dsl::ascii::space | LEXY_LIT("//") >> lexy::dsl::until(lexy::dsl::eol);

    static constexpr auto rule = [] {
        auto digits = lexy::dsl::digits<>.kind<token_kind::a>;
        return digits + lexy::dsl::p<child_p> + digits + lexy::dsl::eof;
    }();
};
} // namespace

template <>
constexpr auto lexy::token_kind_map_for<
    token_kind> = lexy::token_kind_map.map<::token_kind::b>(lexy::dsl::parenthesized.open())
                      .map<::token_kind::b>(lexy::dsl::parenthesized.close())
                      .map<::token_kind::b>(lexy::dsl::quoted.open())
                      .map<::token_kind::c>(lexy::dsl::ascii::character);

TEST_CASE("parse_as_tree")
{
    // Only basic tests necessary, the test for the JSON example does more complex integration
    // testing.

    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    parse_tree tree;

    SUBCASE("parenthesized")
    {
        auto input  = lexy::zstring_input("123(abc)321");
        auto result = lexy::parse_as_tree<root_p>(tree, input, lexy::noop);
        CHECK(result);

        // clang-format off
        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
            .token(token_kind::a, "123")
            .production(child_p{})
                .token(token_kind::b, "(")
                .token(token_kind::c, "abc")
                .token(token_kind::b, ")")
                .finish()
            .token(token_kind::a, "321")
            .token(lexy::eof_token_kind, "");
        // clang-format on
        CHECK(tree == expected);
    }
    SUBCASE("quoted")
    {
        auto input  = lexy::zstring_input("123\"abc\"321");
        auto result = lexy::parse_as_tree<root_p>(tree, input, lexy::noop);
        CHECK(result);

        // clang-format off
        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
            .token(token_kind::a, "123")
            .production(child_p{})
                .production(string_p{})
                    .token(token_kind::b, "\"")
                    .token(token_kind::c, "abc")
                    .token(token_kind::b, "\"")
                    .finish()
                .finish()
            .token(token_kind::a, "321")
            .token(lexy::eof_token_kind, "");
        // clang-format on
        CHECK(tree == expected);
    }
    SUBCASE("whitespace")
    {
        auto input  = lexy::zstring_input("123 ( abc //  \n) 321");
        auto result = lexy::parse_as_tree<root_p>(tree, input, lexy::noop);
        CHECK(result);

        // clang-format off
        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
            .token(token_kind::a, "123")
            .whitespace(" ")
            .production(child_p{})
                .token(token_kind::b, "(")
                .whitespace(" ")
                .token(token_kind::c, "abc")
                .whitespace(" //  \\{a}")
                .token(token_kind::b, ")")
                .whitespace(" ")
                .finish()
            .token(token_kind::a, "321")
            .token(lexy::eof_token_kind, "");
        // clang-format on
        CHECK(tree == expected);
    }
    SUBCASE("failure")
    {
        tree = parse_tree::builder(root_p{}).finish();
        CHECK(!tree.empty());

        auto input  = lexy::zstring_input("123(abc");
        auto result = lexy::parse_as_tree<root_p>(tree, input, lexy::noop);
        CHECK(!result);
        CHECK(tree.empty());
    }
}

