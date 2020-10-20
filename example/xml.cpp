// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <lexy/dsl.hpp>        // lexy::dsl::*
#include <lexy/input/file.hpp> // lexy::read_file
#include <lexy/parse.hpp>      // lexy::parse

#include "report_error.hpp" // lexy_ex::report_error

// Datastructures for storing XML.
// Not really the point of the example, so as simple as possible.
namespace ast
{
class xml_node
{
public:
    xml_node(const xml_node&) = delete;
    xml_node& operator=(const xml_node&) = delete;
    virtual ~xml_node()                  = default;

    virtual void print() const = 0;

protected:
    xml_node() = default;
};

using xml_node_ptr = std::unique_ptr<xml_node>;

class xml_text final : public xml_node
{
public:
    explicit xml_text(std::string text) : _text(std::move(text)) {}

    void print() const override
    {
        std::fputs(_text.c_str(), stdout);
    }

private:
    std::string _text;
};

class xml_reference final : public xml_node
{
public:
    explicit xml_reference(char c) : _c(c) {}

    void print() const override
    {
        switch (_c)
        {
        case '"':
            std::fputs("&quot;", stdout);
            break;
        case '&':
            std::fputs("&amp;", stdout);
            break;
        case '\'':
            std::fputs("&apos;", stdout);
            break;
        case '<':
            std::fputs("&lt;", stdout);
            break;
        case '>':
            std::fputs("&gt;", stdout);
            break;
        }
    }

private:
    char _c;
};

class xml_cdata final : public xml_node
{
public:
    explicit xml_cdata(std::string text) : _text(std::move(text)) {}

    void print() const override
    {
        std::printf("<![CDATA[%s]]>", _text.c_str());
    }

private:
    std::string _text;
};

class xml_element final : public xml_node
{
public:
    explicit xml_element(std::string tag, std::vector<xml_node_ptr> children)
    : _tag(std::move(tag)), _children(std::move(children))
    {}

    void print() const override
    {
        std::printf("<%s>", _tag.c_str());
        for (auto& child : _children)
            child->print();
        std::printf("</%s>", _tag.c_str());
    }

private:
    std::string               _tag;
    std::vector<xml_node_ptr> _children;
};
} // namespace ast

// The grammar of (a subset of) XML.
// It does not support the XML prolog, doctype, or processing instructions.
// It also only supports the pre-defined entity references.
namespace grammar
{
namespace dsl = lexy::dsl;

// An invalid character error.
// Used with try_ to turn generic failures into more specific ones.
struct invalid_character
{
    static constexpr auto name()
    {
        return "invalid character";
    }
};

// Whitespace.
constexpr auto ws = dsl::ascii::space / dsl::ascii::newline;

// Comment.
struct comment
{
    static constexpr auto rule  = LEXY_LIT("<!--") >> until(LEXY_LIT("-->"));
    static constexpr auto value = lexy::noop;
};

// Non-markup content.
struct text
{
    static constexpr auto rule = [] {
        // We only support ASCII.
        auto char_ = dsl::ascii::character - dsl::lit_c<'<'> - dsl::lit_c<'&'>;

        return capture(while_one(dsl::try_<invalid_character>(char_)));
    }();
    static constexpr auto value = lexy::callback<ast::xml_node_ptr>(
        [](auto text) { return std::make_unique<ast::xml_text>(std::string(text)); });
};

// A pre-defined entity reference.
struct reference
{
    struct unknown_entity
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "unknown entity";
        }
    };

    static constexpr auto rule = [] {
        // The name of the reference is everything until ;, excluding the ;.
        auto name = until_peek(dsl::lit_c<';'>);
        // We then switch over the parsed name and create the appropriate character.
        auto reference = switch_(name)
                             .case_(LEXY_LIT("quot") >> dsl::value_c<'"'>)
                             .case_(LEXY_LIT("amp") >> dsl::value_c<'&'>)
                             .case_(LEXY_LIT("apos") >> dsl::value_c<'\''>)
                             .case_(LEXY_LIT("lt") >> dsl::value_c<'<'>)
                             .case_(LEXY_LIT("gt") >> dsl::value_c<'>'>)
                             .error<unknown_entity>(); // The error when nothing matches.

        return dsl::lit_c<'&'> >> reference + dsl::lit_c<';'>;
    }();
    static constexpr auto value = lexy::callback<ast::xml_node_ptr>(
        [](char c) { return std::make_unique<ast::xml_reference>(c); });
};

// A CDATA section.
struct cdata
{
    static constexpr auto rule = [] {
        // We define a string with custom delimiters.
        auto delim = delimited(LEXY_LIT("<![CDATA["), LEXY_LIT("]]>"));
        // And support only ASCII characters inside.
        return delim(dsl::ascii::character);
    }();
    static constexpr auto value = lexy::callback<ast::xml_node_ptr>(
        [](auto content) { return std::make_unique<ast::xml_cdata>(std::string(content)); });
};

// The name of a tag.
struct name
{
    static constexpr auto rule = [] {
        // We only support ASCII.
        auto head_char = dsl::lit_c<':'> / dsl::lit_c<'_'> / dsl::ascii::alpha;
        auto tail_char = head_char / dsl::lit_c<'-'> / dsl::lit_c<'.'> / dsl::ascii::digit;

        return capture(dsl::try_<invalid_character>(head_char) + while_(tail_char));
    }();

    static constexpr auto value = lexy::as_string<std::string>;
};

// A tagged XML element.
struct element
{
    struct tag_mismatch
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "closing tag doesn't match";
        }
    };

    static constexpr auto rule = [] {
        // The brackets for surrounding the opening and closing tag.
        auto open_tagged  = brackets(LEXY_LIT("<"), LEXY_LIT(">")[ws]);
        auto close_tagged = brackets(LEXY_LIT("</"), LEXY_LIT(">")[ws]);

        // The actual rule for the open and closing tag.
        // We use context_push to push a name, and then context_pop to pop it and check that it
        // matches. That is in both case surrounded by the brackets defined above.
        //
        // We also allow an empty tag by checking for the closing characters, dropping the pushed
        // name and immediately returning from parsing.
        auto empty     = LEXY_LIT("/") >> LEXY_LIT(">") + dsl::context_drop + dsl::return_;
        auto open_tag  = open_tagged(dsl::context_push(dsl::p<name>) + opt(empty));
        auto close_tag = close_tagged(dsl::context_pop(dsl::p<name>).error<tag_mismatch>());

        // The content of the element.
        auto content
            = dsl::p<comment> | dsl::p<cdata> // These also start with <, so put them first.
              | if_(LEXY_LIT("<")) >> dsl::recurse<element> // We need a condition for recursion.
              | dsl::p<reference> | dsl::else_ >> dsl::p<text>;

        // We match a (possibly empty) list of content surrounded itself by the open and close tag.
        return brackets(open_tag, close_tag).opt_list(content);
    }();

    // We collect the children as vector.
    static constexpr auto list = lexy::as_list<std::vector<ast::xml_node_ptr>>;
    // Then we construct the node.
    static constexpr auto value = lexy::callback<ast::xml_node_ptr>(
        [](std::string name) {
            return std::make_unique<ast::xml_element>(std::move(name),
                                                      std::vector<ast::xml_node_ptr>{});
        },
        [](std::string name, std::vector<ast::xml_node_ptr> children) {
            return std::make_unique<ast::xml_element>(std::move(name), std::move(children));
        });
};

// An XML document.
struct document
{
    static constexpr auto rule = [] {
        // We allow surrounding the document with comments and whitespace.
        auto ws_comment = ws | dsl::p<comment>;
        return dsl::p<element>[ws_comment] + dsl::eof[ws_comment];
    }();
    static constexpr auto value = lexy::construct<ast::xml_node_ptr>;
};
} // namespace grammar

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: %s <filename>", argv[0]);
        return 1;
    }

    auto file = lexy::read_file(argv[1]);
    if (!file)
    {
        std::fprintf(stderr, "file '%s' not found", argv[1]);
        return 1;
    }

    auto& input    = file.value();
    auto  document = lexy::parse<grammar::document>(input, lexy_ex::report_error);
    if (!document)
        return 2;

    document.value()->print();
}

