#pragma once

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>

// The complete AST definition
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
        std::fputs(b ? "true" : "false", stdout);
    }

    void _print(const json_number& n, int) const
    {
        std::printf("%" PRId64, n.integer);
        if (n.fraction)
        {
            std::printf(".%s", n.fraction->c_str());
        }
        if (n.exponent)
        {
            std::printf("e%" PRId16, *n.exponent);
        }
    }

    void _print(const json_string& str, int) const
    {
        std::fputc('"', stdout);
        for (auto c : str)
        {
            if (c == '"')
                std::fputs("\\\"", stdout);
            else if (c == '\\')
                std::fputs("\\\\", stdout);
            else if (std::iscntrl(c))
                std::printf("\\u%04x", static_cast<unsigned char>(c));
            else
                std::fputc(c, stdout);
        }
        std::fputc('"', stdout);
    }

    void _print(const json_array& arr, int level) const
    {
        std::fputs("[\n", stdout);
        bool first = true;
        for (const auto& elem : arr)
        {
            if (!first)
                std::fputs(",\n", stdout);
            first = false;
            _indent(level + 2);
            elem.print(level + 2);
        }
        std::fputs("\n", stdout);
        _indent(level);
        std::fputc(']', stdout);
    }

    void _print(const json_object& obj, int level) const
    {
        std::fputs("{\n", stdout);
        bool first = true;
        for (const auto& [key, value] : obj)
        {
            if (!first)
                std::fputs(",\n", stdout);
            first = false;
            _indent(level + 2);
            _print(key, level + 2);
            std::fputs(": ", stdout);
            value.print(level + 2);
        }
        std::fputs("\n", stdout);
        _indent(level);
        std::fputc('}', stdout);
    }

    void print(int level = 0) const
    {
        std::visit([this, level](const auto& value) { this->_print(value, level); }, v);
    }
};
} // namespace ast

// The complete grammar definition
namespace grammar
{
namespace dsl = lexy::dsl;

struct json_value; // Forward declaration needed

struct null : lexy::token_production
{
    static constexpr auto rule  = LEXY_LIT("null");
    static constexpr auto value = lexy::construct<ast::json_null>;
};

struct boolean : lexy::token_production
{
    struct true_ : lexy::transparent_production
    {
        static constexpr auto rule  = LEXY_LIT("true");
        static constexpr auto value = lexy::constant(true);
    };
    struct false_ : lexy::transparent_production
    {
        static constexpr auto rule  = LEXY_LIT("false");
        static constexpr auto value = lexy::constant(false);
    };

    static constexpr auto rule  = dsl::p<true_> | dsl::p<false_>;
    static constexpr auto value = lexy::forward<ast::json_bool>;
};

struct number : lexy::token_production
{
    // A signed integer parsed as int64_t
    struct integer : lexy::transparent_production
    {
        static constexpr auto rule  = dsl::minus_sign.opt() >> dsl::integer<std::int64_t>;
        static constexpr auto value = lexy::as_integer<std::int64_t>;
    };

    // The fractional part of a number
    struct fraction : lexy::transparent_production
    {
        static constexpr auto rule  = dsl::lit_c<'.'> >> dsl::capture(dsl::digits<>);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    // The exponent of a number
    struct exponent : lexy::transparent_production
    {
        static constexpr auto rule
            = (dsl::lit_c<'e'> | dsl::lit_c<'E'>) >> dsl::sign + dsl::integer<std::int16_t>;
        static constexpr auto value = lexy::as_integer<std::int16_t>;
    };

    static constexpr auto rule
        = dsl::p<integer> + dsl::opt(dsl::p<fraction>) + dsl::opt(dsl::p<exponent>);
    static constexpr auto value = lexy::construct<ast::json_number>;
};

struct string : lexy::token_production
{
    struct invalid_char
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "invalid character in string literal";
        }
    };

    // Escape sequence mapping
    static constexpr auto escaped_symbols = lexy::symbol_table<char>
            .map<'"'>('"')
            .map<'\\'>('\\')
            .map<'/'>('/')
            .map<'b'>('\b')
            .map<'f'>('\f')
            .map<'n'>('\n')
            .map<'r'>('\r')
            .map<'t'>('\t');

    static constexpr auto rule = [] {
        auto char_set = (-dsl::unicode::control).error<invalid_char>;
        auto escape   = dsl::backslash_escape.symbol<escaped_symbols>();
        return dsl::quoted(char_set, escape);
    }();
    static constexpr auto value = lexy::as_string<ast::json_string>;
};

struct array
{
    static constexpr auto rule
        = dsl::square_bracketed.opt_list(dsl::recurse<json_value>, dsl::sep(dsl::comma));
    static constexpr auto value = lexy::as_list<ast::json_array>;
};

struct object
{
    static constexpr auto rule
        = dsl::curly_bracketed.opt_list(dsl::p<string> + dsl::colon + dsl::recurse<json_value>,
                                        dsl::sep(dsl::comma));
    static constexpr auto value = lexy::as_collection<ast::json_object>;
};

struct json_value : lexy::transparent_production
{
    static constexpr auto rule = dsl::p<null> | dsl::p<boolean> | dsl::p<number> | dsl::p<string>
                                 | dsl::p<array> | dsl::p<object>;
    static constexpr auto value = lexy::construct<ast::json_value>;
};

struct json
{
    static constexpr auto whitespace = dsl::ascii::space | dsl::ascii::newline;
    static constexpr auto rule       = dsl::p<json_value> + dsl::eof;
    static constexpr auto value      = lexy::forward<ast::json_value>;
};
} // namespace grammar
