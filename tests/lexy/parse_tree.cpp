// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/parse_tree.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/brackets.hpp>
#include <lexy/dsl/delimited.hpp>
#include <lexy/dsl/digit.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/parse_tree_doctest.hpp>
#include <vector>

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
    static constexpr auto rule = lexy::dsl::parenthesized(LEXY_LIT("abc").kind<token_kind::c>)
                                 | lexy::dsl::quoted(lexy::dsl::ascii::character);
};

struct root_p
{
    static constexpr auto whitespace = lexy::dsl::ascii::space;

    static constexpr auto rule = [] {
        auto digits = lexy::dsl::digits<>.kind<token_kind::a>;
        return digits + lexy::dsl::p<child_p> + digits;
    }();
};
} // namespace

template <>
constexpr auto lexy::token_kind_map_for<
    token_kind> = lexy::token_kind_map.map<::token_kind::b>(lexy::dsl::parenthesized.open())
                      .map<::token_kind::b>(lexy::dsl::parenthesized.close())
                      .map<::token_kind::b>(lexy::dsl::quoted.open())
                      .map<::token_kind::c>(lexy::dsl::ascii::character);

TEST_CASE("parse_tree::builder")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    SUBCASE("basic")
    {
        auto input = lexy::zstring_input("123(abc)321");

        auto tree = [&] {
            parse_tree::builder builder(root_p{});
            builder.token(token_kind::a, input.begin(), input.begin() + 3);

            auto child = builder.start_production(child_p{});
            builder.token(token_kind::b, input.begin() + 3, input.begin() + 4);
            builder.token(token_kind::c, input.begin() + 4, input.begin() + 7);
            builder.token(token_kind::b, input.begin() + 7, input.begin() + 8);
            builder.finish_production(LEXY_MOV(child));

            builder.token(token_kind::a, input.begin() + 8, input.end());

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());

        // clang-format off
        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
            .token(token_kind::a, "123")
            .production(child_p{})
                .token(token_kind::b, "(")
                .token(token_kind::c, "abc")
                .token(token_kind::b, ")")
                .finish()
            .token(token_kind::a, "321");
        // clang-format on
        CHECK(tree == expected);
    }
    SUBCASE("only root")
    {
        auto tree = parse_tree::builder(root_p{}).finish();
        CHECK(!tree.empty());

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{});
        CHECK(tree == expected);
    }
    SUBCASE("shallow")
    {
        auto input = lexy::zstring_input("(abc)");
        auto tree  = [&] {
            parse_tree::builder builder(root_p{});

            builder.token(token_kind::b, input.begin(), input.begin() + 1);
            builder.token(token_kind::c, input.begin() + 1, input.begin() + 4);
            builder.token(token_kind::b, input.begin() + 4, input.end());

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
                            .token(token_kind::b, "(")
                            .token(token_kind::c, "abc")
                            .token(token_kind::b, ")");
        CHECK(tree == expected);
    }
    SUBCASE("linear")
    {
        auto input = lexy::zstring_input("abc");
        auto tree  = [&] {
            parse_tree::builder builder(root_p{});

            auto child = builder.start_production(child_p{});
            builder.token(token_kind::c, input.begin(), input.end());
            builder.finish_production(LEXY_MOV(child));

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
                            .production(child_p{})
                            .token(token_kind::c, "abc")
                            .finish();
        CHECK(tree == expected);
    }
    SUBCASE("empty child production")
    {
        auto input = lexy::zstring_input("123321");

        auto tree = [&] {
            parse_tree::builder builder(root_p{});
            builder.token(token_kind::a, input.begin(), input.begin() + 3);

            auto child = builder.start_production(child_p{});
            builder.finish_production(LEXY_MOV(child));

            builder.token(token_kind::a, input.begin() + 3, input.end());

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
                            .token(token_kind::a, "123")
                            .production(child_p{})
                            .finish()
                            .token(token_kind::a, "321");
        CHECK(tree == expected);
    }

    constexpr auto many_count = 1024u;
    SUBCASE("many linear productions")
    {
        auto input = lexy::zstring_input("abc");

        auto tree = [&] {
            parse_tree::builder builder(root_p{});

            std::vector<parse_tree::builder::production_state> states;
            for (auto i = 0u; i != many_count; ++i)
            {
                auto state = builder.start_production(child_p{});
                builder.token(token_kind::a, input.begin(), input.end());
                builder.finish_production(LEXY_MOV(state));
            }

            return LEXY_MOV(builder).finish();
        }();

        auto expected = [&] {
            lexy_ext::parse_tree_desc<token_kind> result(root_p{});
            for (auto i = 0u; i != many_count; ++i)
                result.production(child_p{}).token(token_kind::a, "abc").finish();
            return result;
        }();
        CHECK(tree == expected);
    }
    SUBCASE("many nested productions")
    {
        auto input = lexy::zstring_input("abc");

        auto tree = [&] {
            parse_tree::builder builder(root_p{});

            std::vector<parse_tree::builder::production_state> states;
            for (auto i = 0u; i != many_count; ++i)
            {
                auto state = builder.start_production(child_p{});
                states.push_back(LEXY_MOV(state));
            }
            builder.token(token_kind::a, input.begin(), input.end());
            for (auto i = 0u; i != many_count; ++i)
            {
                builder.finish_production(LEXY_MOV(states.back()));
                states.pop_back();
            }

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());

        auto expected = [&] {
            lexy_ext::parse_tree_desc<token_kind> result(root_p{});
            for (auto i = 0u; i != many_count; ++i)
                result.production(child_p{});
            result.token(token_kind::a, "abc");
            return result;
        }();
        CHECK(tree == expected);
    }
}

namespace
{
template <typename Production, typename NodeKind>
void check_kind(NodeKind kind, const char* name, bool root = false)
{
    CHECK(kind.is_root() == root);
    CHECK(kind.is_production());
    CHECK(!kind.is_token());

    CHECK(kind.name() == name);

    CHECK(kind == kind);
    CHECK_FALSE(kind != kind);

    CHECK(kind == Production{});
    CHECK(Production{} == kind);
    CHECK_FALSE(kind != Production{});
    CHECK_FALSE(Production{} != kind);
}

template <typename NodeKind>
void check_kind(NodeKind kind, token_kind tk)
{
    CHECK(!kind.is_root());
    CHECK(!kind.is_production());
    CHECK(kind.is_token());

    CHECK(kind.name() == token_kind_name(tk));

    CHECK(kind == kind);
    CHECK_FALSE(kind != kind);

    CHECK(kind == tk);
    CHECK(tk == kind);
    CHECK_FALSE(kind != tk);
    CHECK_FALSE(tk != kind);
}

template <typename Node, typename Iter>
void check_token(Node token, token_kind tk, Iter begin, Iter end)
{
    check_kind(token.kind(), tk);
    CHECK(token.lexeme().begin() == begin);
    CHECK(token.lexeme().end() == end);

    CHECK(token.token().kind() == tk);
    CHECK(token.token().lexeme().begin() == begin);
    CHECK(token.token().lexeme().end() == end);
}
} // namespace

TEST_CASE("parse_tree::node")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.begin(), input.begin() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.begin() + 3, input.begin() + 4);
        builder.token(token_kind::c, input.begin() + 4, input.begin() + 7);
        builder.token(token_kind::b, input.begin() + 7, input.begin() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.begin() + 8, input.end());

        child = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    auto root = tree.root();
    check_kind<root_p>(root.kind(), "root_p", true);
    CHECK(root.parent() == root);
    CHECK(root.lexeme().empty());

    auto children = root.children();
    CHECK(!children.empty());
    CHECK(children.size() == 4);

    auto iter = children.begin();
    CHECK(iter != children.end());
    check_token(*iter, token_kind::a, input.begin(), input.begin() + 3);
    CHECK(iter->parent() == root);

    ++iter;
    CHECK(iter != children.end());
    {
        auto child = *iter;
        check_kind<child_p>(child.kind(), "child_p");
        CHECK(child.parent() == root);
        CHECK(child.lexeme().empty());

        auto children = child.children();
        CHECK(!children.empty());
        CHECK(children.size() == 3);

        auto iter = children.begin();
        CHECK(iter != children.end());
        check_token(*iter, token_kind::b, input.begin() + 3, input.begin() + 4);
        CHECK(iter->parent() == child);

        ++iter;
        CHECK(iter != children.end());
        check_token(*iter, token_kind::c, input.begin() + 4, input.begin() + 7);
        CHECK(iter->parent() == child);

        ++iter;
        CHECK(iter != children.end());
        check_token(*iter, token_kind::b, input.begin() + 7, input.begin() + 8);
        CHECK(iter->parent() == child);

        ++iter;
        CHECK(iter == children.end());
    }

    ++iter;
    CHECK(iter != children.end());
    check_token(*iter, token_kind::a, input.begin() + 8, input.end());
    CHECK(iter->parent() == root);

    ++iter;
    CHECK(iter != children.end());
    {
        auto child = *iter;
        CHECK(child.parent() == root);
        check_kind<child_p>(child.kind(), "child_p");
        CHECK(child.lexeme().empty());

        auto children = child.children();
        CHECK(children.empty());
        CHECK(children.size() == 0);
    }

    ++iter;
    CHECK(iter == children.end());
}

TEST_CASE("parse_tree::node::sibling_range")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.begin(), input.begin() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.begin() + 3, input.begin() + 4);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.begin() + 8, input.end());

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    SUBCASE("siblings first child")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            check_token(*iter, token_kind::a, input.begin(), input.begin() + 3);
            return *iter;
        }();

        auto range = node.siblings();
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        check_kind<child_p>(iter->kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.begin() + 8, input.end());

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("siblings middle child")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            check_kind<child_p>(iter->kind(), "child_p");
            return *iter;
        }();

        auto range = node.siblings();
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.begin() + 8, input.end());

        ++iter;
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.begin(), input.begin() + 3);

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("siblings last child")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            ++iter;
            check_token(*iter, token_kind::a, input.begin() + 8, input.end());
            return *iter;
        }();

        auto range = node.siblings();
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.begin(), input.begin() + 3);

        ++iter;
        CHECK(iter != range.end());
        check_kind<child_p>(iter->kind(), "child_p");

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("siblings only child")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            iter = iter->children().begin();
            check_token(*iter, token_kind::b, input.begin() + 3, input.begin() + 4);
            return *iter;
        }();

        auto range = node.siblings();
        CHECK(range.empty());

        auto iter = range.begin();
        CHECK(iter == range.end());
    }
}

TEST_CASE("parse_tree::traverse_range")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.begin(), input.begin() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.begin() + 3, input.begin() + 4);
        builder.token(token_kind::c, input.begin() + 4, input.begin() + 7);
        builder.token(token_kind::b, input.begin() + 7, input.begin() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.begin() + 8, input.end());

        child = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    SUBCASE("entire empty tree")
    {
        tree.clear();
        REQUIRE(tree.empty());

        auto range = tree.traverse();
        CHECK(range.empty());
        CHECK(range.begin() == range.end());
    }
    SUBCASE("entire tree")
    {
        auto range = tree.traverse();
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::enter);
        CHECK(iter->node == tree.root());

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::a, input.begin(), input.begin() + 3);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::enter);
        check_kind<child_p>(iter->node.kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.begin() + 3, input.begin() + 4);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::c, input.begin() + 4, input.begin() + 7);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.begin() + 7, input.begin() + 8);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::exit);
        check_kind<child_p>(iter->node.kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::a, input.begin() + 8, input.end());

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::enter);
        check_kind<child_p>(iter->node.kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::exit);
        check_kind<child_p>(iter->node.kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::exit);
        CHECK(iter->node == tree.root());

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("child production")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            check_kind<child_p>(iter->kind(), "child_p");

            return *iter;
        }();

        auto range = tree.traverse(node);
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::enter);
        CHECK(iter->node == node);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.begin() + 3, input.begin() + 4);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::c, input.begin() + 4, input.begin() + 7);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.begin() + 7, input.begin() + 8);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::exit);
        CHECK(iter->node == node);

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("empty child production")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            ++iter;
            ++iter;
            check_kind<child_p>(iter->kind(), "child_p");

            return *iter;
        }();

        auto range = tree.traverse(node);
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::enter);
        CHECK(iter->node == node);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::exit);
        CHECK(iter->node == node);

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("token")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            check_token(*iter, token_kind::a, input.begin(), input.begin() + 3);
            return *iter;
        }();

        auto range = tree.traverse(node);
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        CHECK(iter->node == node);

        ++iter;
        CHECK(iter == range.end());
    }
}

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
            .token(token_kind::a, "321");
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
                .token(token_kind::b, "\"")
                .token(token_kind::c, "abc")
                .token(token_kind::b, "\"")
                .finish()
            .token(token_kind::a, "321");
        // clang-format on
        CHECK(tree == expected);
    }
    SUBCASE("whitespace")
    {
        auto input  = lexy::zstring_input("123 ( abc ) 321");
        auto result = lexy::parse_as_tree<root_p>(tree, input, lexy::noop);
        CHECK(result);

        // clang-format off
        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
            .token(token_kind::a, "123")
            .token(" ")
            .production(child_p{})
                .token(token_kind::b, "(")
                .token(" ")
                .token(token_kind::c, "abc")
                .token(" ")
                .token(token_kind::b, ")")
                .token(" ")
                .finish()
            .token(token_kind::a, "321");
        // clang-format on
        CHECK(tree == expected);
    }
    SUBCASE("failure")
    {
        auto input  = lexy::zstring_input("123(abc");
        auto result = lexy::parse_as_tree<root_p>(tree, input, lexy::noop);
        CHECK(!result);
        CHECK(tree.empty());
    }
}

