// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/parse_tree.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/any.hpp>
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
    static constexpr auto name = "child_p";
    static constexpr auto rule = lexy::dsl::any;
};

struct root_p
{
    static constexpr auto name = "root_p";
    static constexpr auto rule = lexy::dsl::any;
};
} // namespace

TEST_CASE("parse_tree::builder")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    SUBCASE("empty")
    {
        parse_tree tree;
        CHECK(tree.empty());
        CHECK(tree.size() == 0);
    }

    SUBCASE("basic")
    {
        auto input = lexy::zstring_input("123(abc)321");

        auto tree = [&] {
            parse_tree::builder builder(root_p{});
            builder.token(token_kind::a, input.data(), input.data() + 3);

            auto child = builder.start_production(child_p{});
            builder.token(token_kind::b, input.data() + 3, input.data() + 4);
            builder.token(token_kind::c, input.data() + 4, input.data() + 7);
            builder.token(token_kind::b, input.data() + 7, input.data() + 8);
            builder.finish_production(LEXY_MOV(child));

            builder.token(token_kind::a, input.data() + 8, input.data() + 11);

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());
        CHECK(tree.size() == 7);
        CHECK(tree.depth() == 2);

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
        CHECK(tree.size() == 1);
        CHECK(tree.depth() == 0);

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{});
        CHECK(tree == expected);
    }
    SUBCASE("shallow")
    {
        auto input = lexy::zstring_input("(abc)");
        auto tree  = [&] {
            parse_tree::builder builder(root_p{});

            builder.token(token_kind::b, input.data(), input.data() + 1);
            builder.token(token_kind::c, input.data() + 1, input.data() + 4);
            builder.token(token_kind::b, input.data() + 4, input.data() + 5);

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());
        CHECK(tree.size() == 4);
        CHECK(tree.depth() == 1);

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
                            .token(token_kind::b, "(")
                            .token(token_kind::c, "abc")
                            .token(token_kind::b, ")");
        CHECK(tree == expected);
    }
    SUBCASE("nested")
    {
        auto input = lexy::zstring_input("abc");
        auto tree  = [&] {
            parse_tree::builder builder(root_p{});

            auto child = builder.start_production(child_p{});
            builder.token(token_kind::c, input.data(), input.data() + 3);
            builder.finish_production(LEXY_MOV(child));

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());
        CHECK(tree.size() == 3);
        CHECK(tree.depth() == 2);

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
            builder.token(token_kind::a, input.data(), input.data() + 3);

            auto child = builder.start_production(child_p{});
            builder.finish_production(LEXY_MOV(child));

            builder.token(token_kind::a, input.data() + 3, input.data() + 6);

            return LEXY_MOV(builder).finish();
        }();
        CHECK(!tree.empty());
        CHECK(tree.size() == 4);
        CHECK(tree.depth() == 1);

        auto expected = lexy_ext::parse_tree_desc<token_kind>(root_p{})
                            .token(token_kind::a, "123")
                            .production(child_p{})
                            .finish()
                            .token(token_kind::a, "321");
        CHECK(tree == expected);
    }

    constexpr auto many_count = 1024u;
    SUBCASE("many shallow productions")
    {
        auto input = lexy::zstring_input("abc");

        auto tree = [&] {
            parse_tree::builder builder(root_p{});

            std::vector<parse_tree::builder::marker> markers;
            for (auto i = 0u; i != many_count; ++i)
            {
                auto m = builder.start_production(child_p{});
                builder.token(token_kind::a, input.data(), input.data() + input.size());
                builder.finish_production(LEXY_MOV(m));
            }

            return LEXY_MOV(builder).finish();
        }(); // root -> (p_1 -> token), ..., (p_many_count -> token)
        CHECK(!tree.empty());
        CHECK(tree.size() == 2 * many_count + 1);
        CHECK(tree.depth() == 2);

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

            std::vector<parse_tree::builder::marker> markers;
            for (auto i = 0u; i != many_count; ++i)
            {
                auto m = builder.start_production(child_p{});
                markers.push_back(LEXY_MOV(m));
            }
            builder.token(token_kind::a, input.data(), input.data() + input.size());
            for (auto i = 0u; i != many_count; ++i)
            {
                builder.finish_production(LEXY_MOV(markers.back()));
                markers.pop_back();
            }

            return LEXY_MOV(builder).finish();
        }(); // root -> p_1 -> ... p_many_count -> token
        CHECK(!tree.empty());
        CHECK(tree.size() == many_count + 2);
        CHECK(tree.depth() == many_count + 1);

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

    CHECK(kind.name() == lexy::_detail::string_view(name));

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
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

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
    check_token(*iter, token_kind::a, input.data(), input.data() + 3);
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
        check_token(*iter, token_kind::b, input.data() + 3, input.data() + 4);
        CHECK(iter->parent() == child);

        ++iter;
        CHECK(iter != children.end());
        check_token(*iter, token_kind::c, input.data() + 4, input.data() + 7);
        CHECK(iter->parent() == child);

        ++iter;
        CHECK(iter != children.end());
        check_token(*iter, token_kind::b, input.data() + 7, input.data() + 8);
        CHECK(iter->parent() == child);

        ++iter;
        CHECK(iter == children.end());
    }

    ++iter;
    CHECK(iter != children.end());
    check_token(*iter, token_kind::a, input.data() + 8, input.data() + 11);
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
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    SUBCASE("siblings first child")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            check_token(*iter, token_kind::a, input.data(), input.data() + 3);
            return *iter;
        }();

        auto range = node.siblings();
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        check_kind<child_p>(iter->kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.data() + 8, input.data() + 11);

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
        check_token(*iter, token_kind::a, input.data() + 8, input.data() + 11);

        ++iter;
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.data(), input.data() + 3);

        ++iter;
        CHECK(iter == range.end());
    }
    SUBCASE("siblings last child")
    {
        auto node = [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            ++iter;
            check_token(*iter, token_kind::a, input.data() + 8, input.data() + 11);
            return *iter;
        }();

        auto range = node.siblings();
        CHECK(!range.empty());

        auto iter = range.begin();
        CHECK(iter != range.end());
        check_token(*iter, token_kind::a, input.data(), input.data() + 3);

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
            check_token(*iter, token_kind::b, input.data() + 3, input.data() + 4);
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
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

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
        check_token(iter->node, token_kind::a, input.data(), input.data() + 3);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::enter);
        check_kind<child_p>(iter->node.kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.data() + 3, input.data() + 4);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::c, input.data() + 4, input.data() + 7);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.data() + 7, input.data() + 8);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::exit);
        check_kind<child_p>(iter->node.kind(), "child_p");

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::a, input.data() + 8, input.data() + 11);

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
        check_token(iter->node, token_kind::b, input.data() + 3, input.data() + 4);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::c, input.data() + 4, input.data() + 7);

        ++iter;
        CHECK(iter != range.end());
        CHECK(iter->event == lexy::traverse_event::leaf);
        check_token(iter->node, token_kind::b, input.data() + 7, input.data() + 8);

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
            check_token(*iter, token_kind::a, input.data(), input.data() + 3);
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

