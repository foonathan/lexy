// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <lexy/dsl.hpp>        // lexy::dsl::*
#include <lexy/input/file.hpp> // lexy::read_file
#include <lexy/parse.hpp>      // lexy::parse

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

// Datastructures for storing JSON.
// Not really the point of the example, so as simple as possible.
namespace ast
{
struct json_value;

using json_null = std::nullptr_t;
using json_bool = bool;

struct json_number
{
    std::int64_t                integer;
    std::optional<std::string>  fraction;
    std::optional<std::int16_t> exponent;
};

using json_string = std::string;

using json_array  = std::vector<json_value>;
using json_object = std::map<std::string, json_value>;

struct json_value
{
    std::variant<json_null, json_bool, json_number, json_string, json_array, json_object> v;

    template <typename T>
    json_value(T t) : v(std::move(t))
    {}

    void _indent(int level) const
    {
        for (auto i = 0; i < level; ++i)
            std::fputc(' ', stdout);
    }

    void _print(json_null, int) const
    {
        std::fputs("null", stdout);
    }
    void _print(json_bool b, int) const
    {
        if (b)
            std::fputs("true", stdout);
        else
            std::fputs("false", stdout);
    }
    void _print(const json_number& i, int) const
    {
        std::fprintf(stdout, "%" PRId64, i.integer);
        if (i.fraction)
            std::fprintf(stdout, ".%s", i.fraction->c_str());
        if (i.exponent)
            std::fprintf(stdout, "e%" PRId16, *i.exponent);
    }
    void _print(const json_string& str, int) const
    {
        std::fputc('"', stdout);
        for (auto& c : str)
            if (c == '"')
                std::fputs(R"(\")", stdout);
            else if (c == '\\')
                std::fputs(R"(\\)", stdout);
            else if (std::iscntrl(c))
                std::fprintf(stdout, "\\x%02x", static_cast<unsigned char>(c));
            else
                std::fputc(c, stdout);
        std::fputc('"', stdout);
    }
    void _print(const json_array& array, int level) const
    {
        std::fputs("[\n", stdout);

        auto first = true;
        for (auto& elem : array)
        {
            if (first)
                first = false;
            else
                std::fputs(",\n", stdout);

            _indent(level + 1);
            elem.print(level + 1);
        }

        std::fputs("\n", stdout);
        _indent(level);
        std::fputs("]", stdout);
    }
    void _print(const json_object& object, int level) const
    {
        std::fputs("{\n", stdout);

        auto first = true;
        for (auto& [key, value] : object)
        {
            if (first)
                first = false;
            else
                std::fputs(",\n", stdout);

            _indent(level + 1);
            _print(key, level + 1);
            std::fputs(" : ", stdout);
            value.print(level + 1);
        }

        std::fputs("\n", stdout);
        _indent(level);
        std::fputs("}", stdout);
    }

    void print(int level = 0) const
    {
        std::visit([&](const auto& value) { _print(value, level); }, v);
    }
};
} // namespace ast

// The grammar of JSON.
// Modelled after the specificaton of https://www.json.org.
// It is compliant modulo bugs.
namespace grammar
{
namespace dsl = lexy::dsl;

struct json_value;

// A json value that is a number.
struct number : lexy::token_production
{
    // A signed integer parsed as int64_t.
    struct integer : lexy::transparent_production
    {
        static constexpr auto rule
            = dsl::minus_sign + dsl::integer<std::int64_t>(dsl::digits<>.no_leading_zero());
        static constexpr auto value = lexy::as_integer<std::int64_t>;
    };

    // The fractional part of a number parsed as the string.
    struct fraction : lexy::transparent_production
    {
        static constexpr auto rule  = dsl::lit_c<'.'> >> dsl::capture(dsl::digits<>);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    // The exponent of a number parsed as int64_t.
    struct exponent : lexy::transparent_production
    {
        static constexpr auto rule = [] {
            auto exp_char = dsl::lit_c<'e'> / dsl::lit_c<'E'>;
            return exp_char >> dsl::sign + dsl::integer<std::int16_t>(dsl::digits<>);
        }();
        static constexpr auto value = lexy::as_integer<std::int16_t>;
    };

    static constexpr auto rule
        = dsl::peek(dsl::lit_c<'-'> / dsl::digit<>)
          >> dsl::p<integer> + dsl::opt(dsl::p<fraction>) + dsl::opt(dsl::p<exponent>);
    static constexpr auto value = lexy::construct<ast::json_number>;
};

// A json value that is a string.
struct string
{
    struct invalid_char
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "invalid character in string literal";
        }
    };

    static constexpr auto rule = [] {
        auto code_point = (dsl::code_point - dsl::ascii::control).error<invalid_char>;
        auto escape     = dsl::backslash_escape //
                          .lit_c<'"'>()
                          .lit_c<'\\'>()
                          .lit_c<'/'>()
                          .lit_c<'b'>(dsl::value_c<'\b'>)
                          .lit_c<'f'>(dsl::value_c<'\f'>)
                          .lit_c<'n'>(dsl::value_c<'\n'>)
                          .lit_c<'r'>(dsl::value_c<'\r'>)
                          .lit_c<'t'>(dsl::value_c<'\t'>)
                          .rule(dsl::lit_c<'u'> >> dsl::code_point_id<4>);

        // String of code_point with specified escape sequences, surrounded by ".
        return dsl::quoted(code_point, escape);
    }();

    static constexpr auto value = lexy::as_string<ast::json_string, lexy::utf8_encoding>;
};

// A json value that is an array.
struct array
{
    // A (potentially empty) list of json values, seperated by comma and surrouned by square
    // brackets. Use trailing_sep() here to allow trailing commas.
    static constexpr auto rule
        = dsl::square_bracketed.opt_list(dsl::recurse<json_value>, sep(dsl::comma));

    static constexpr auto value = lexy::as_list<ast::json_array>;
};

// A json value that is an object.
struct object
{
    static constexpr auto rule = [] {
        auto item = dsl::p<string> + dsl::colon + dsl::recurse<json_value>;
        // A (potentially empty) list of items, seperated by comma and surrouned by curly brackets.
        // Use trailing_sep() here to allow trailing commas.
        return dsl::curly_bracketed.opt_list(item, dsl::sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_collection<ast::json_object>;
};

// A json value.
struct json_value : lexy::transparent_production
{
    struct expected_json_value
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "expected json value";
        }
    };

    static constexpr auto rule = [] {
        auto null   = LEXY_LIT("null") >> dsl::value_t<ast::json_null>;
        auto true_  = LEXY_LIT("true") >> dsl::value_c<true>;
        auto false_ = LEXY_LIT("false") >> dsl::value_c<false>;

        auto primitive = null | true_ | false_ | dsl::p<number> | dsl::p<string>;
        auto complex   = dsl::p<object> | dsl::p<array>;

        return primitive | complex | dsl::else_ >> dsl::error<expected_json_value>;
    }();

    static constexpr auto value = lexy::construct<ast::json_value>;
};

// Entry point of the production.
struct json
{
    // Whitespace is a sequence of space, tab, carriage return, or newline.
    // Add your comment syntax here.
    static constexpr auto whitespace = dsl::ascii::space / dsl::ascii::newline;

    static constexpr auto rule  = dsl::whitespace + dsl::p<json_value> + dsl::eof;
    static constexpr auto value = lexy::forward<ast::json_value>;
};
} // namespace grammar

#ifndef LEXY_TEST
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: %s <filename>", argv[0]);
        return 1;
    }

    // We're requiring UTF-8 input.
    auto file = lexy::read_file<lexy::utf8_encoding>(argv[1]);
    if (!file)
    {
        std::fprintf(stderr, "file '%s' not found", argv[1]);
        return 1;
    }

    auto& input = file.value();
    auto  json  = lexy::parse<grammar::json>(input, lexy_ext::report_error);
    if (!json)
        return 2;

    json.value().print();
}
#endif

