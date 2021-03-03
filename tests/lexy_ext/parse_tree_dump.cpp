// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy_ext/parse_tree_dump.hpp>

#include <doctest/doctest.h>
#include <lexy/input/file.hpp>
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

struct child_p
{
    static constexpr auto name = "child_p";
};
struct root_p
{
    static constexpr auto name = "root_p";
};

constexpr auto test_file_name = "lexy-dump_parse_tree.test.delete-me";
} // namespace

TEST_CASE("dump_parse_tree()")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc\"\n\x84)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.begin(), input.begin() + 3);

        auto child     = builder.start_production(child_p{});
        auto sub_child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.begin() + 3, input.begin() + 4);
        builder.token(token_kind::c, input.begin() + 4, input.begin() + 10);
        builder.token(token_kind::b, input.begin() + 10, input.begin() + 11);
        builder.finish_production(LEXY_MOV(sub_child));
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.begin() + 11, input.end());

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    std::remove(test_file_name);
    auto file = std::fopen(test_file_name, "wb");

    SUBCASE("fancy")
    {
        lexy_ext::dump_parse_tree(file, tree);
        std::fclose(file);

        auto result = lexy::read_file(test_file_name);
        REQUIRE(doctest::String(result.data(), unsigned(result.size())) == R"*(root_p:
├──a: "123"
├──child_p:
│  └──child_p:
│     ├──b: "("
│     ├──c: "abc\"\n\x84"
│     └──b: ")"
└──a: "321"
)*");
    }
    SUBCASE("simple")
    {
        lexy_ext::dump_parse_tree(file, tree, lexy_ext::simple_parse_tree_dump);
        std::fclose(file);

        auto result = lexy::read_file(test_file_name);
        REQUIRE(doctest::String(result.data(), unsigned(result.size())) == R"*(root_p:
- a: "123"
- child_p:
  - child_p:
    - b: "("
    - c: "abc\"\n\x84"
    - b: ")"
- a: "321"
)*");
    }

    std::remove(test_file_name);
}

