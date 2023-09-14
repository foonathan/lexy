// Copyright (C) 2020-2023 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/parse_tree_input.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/any.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/parse_tree.hpp>

namespace
{
enum class token_kind
{
    a,
    b,
    c,
};

struct child_p
{
    static constexpr auto rule = lexy::dsl::any;
};

struct root_p
{
    static constexpr auto rule = lexy::dsl::any;
};
} // namespace

TEST_CASE("parse_tree_input")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        return LEXY_MOV(builder).finish(input.data() + 11);
    }();
    CHECK(!tree.empty());

    auto tree_input = lexy::parse_tree_input(tree);
    CHECK(tree_input.root().address() == tree.root().address());

    auto root_reader = tree_input.reader();
    CHECK(root_reader.position() == input.data());
    CHECK(root_reader.peek().kind() == token_kind::a);

    {
        auto child_reader = root_reader.child_reader();
        CHECK(child_reader.peek().address() == nullptr);
    }

    root_reader.bump();
    CHECK(root_reader.position() == input.data() + 3);
    CHECK(root_reader.peek().kind() == child_p{});

    {
        auto child_reader = root_reader.child_reader();
        CHECK(child_reader.position() == input.data() + 3);
        CHECK(child_reader.peek().kind() == token_kind::b);

        child_reader.bump();
        CHECK(child_reader.position() == input.data() + 4);
        CHECK(child_reader.peek().kind() == token_kind::c);

        child_reader.bump();
        CHECK(child_reader.position() == input.data() + 7);
        CHECK(child_reader.peek().kind() == token_kind::b);

        child_reader.bump();
        CHECK(child_reader.position() == input.data() + 8);
        CHECK(child_reader.peek().address() == nullptr);
    }

    root_reader.bump();
    CHECK(root_reader.position() == input.data() + 8);
    CHECK(root_reader.peek().kind() == token_kind::a);

    {
        auto child_reader = root_reader.child_reader();
        CHECK(child_reader.peek().address() == nullptr);
    }

    root_reader.bump();
    CHECK(root_reader.position() == input.data() + 11);
    CHECK(root_reader.peek().address() == nullptr);
}

