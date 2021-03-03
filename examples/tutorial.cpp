// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <lexy/dsl.hpp>        // lexy::dsl::*
#include <lexy/input/file.hpp> // lexy::read_file
#include <lexy/parse.hpp>      // lexy::parse

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

struct PackageVersion
{
    int major;
    int minor;
    int patch;
};

struct PackageConfig
{
    std::string              name;
    PackageVersion           version;
    std::vector<std::string> authors;
};

namespace grammar
{
namespace dsl = lexy::dsl;

struct name : lexy::token_production
{
    struct invalid_character
    {
        static constexpr auto name = "invalid name character";
    };

    // Match an alpha character, followed by zero or more alphanumeric characters or underscores.
    // Captures it all into a lexeme.
    static constexpr auto rule = [] {
        auto lead_char     = dsl::ascii::alpha;
        auto trailing_char = dsl::ascii::alnum / dsl::lit_c<'_'>;

        return dsl::capture(lead_char + dsl::while_(trailing_char))
               + dsl::require(dsl::ascii::space).error<invalid_character>;
    }();

    // The final value of this production is a std::string we've created from the lexeme.
    static constexpr auto value = lexy::as_string<std::string>;
};

struct version : lexy::token_production
{
    struct forbidden_build_string
    {
        static constexpr auto name = "build string not supported";
    };

    // Match three integers separated by dots, or the special tag "unreleased".
    static constexpr auto rule = [] {
        auto number      = dsl::integer<int>(dsl::digits<>);
        auto dot         = dsl::period;
        auto dot_version = number + dot + number + dot + number
                           + dsl::prevent(dsl::lit_c<'-'>).error<forbidden_build_string>;

        auto unreleased
            = LEXY_LIT("unreleased") >> dsl::value_c<0> + dsl::value_c<0> + dsl::value_c<0>;

        return unreleased | dsl::else_ >> dot_version;
    }();

    // Construct a PackageVersion as the result of the production.
    static constexpr auto value = lexy::construct<PackageVersion>;
};

struct author
{
    struct invalid_character
    {
        static constexpr auto name = "invalid string character";
    };

    // Match zero or more non-control code points ("characters") surrounded by quotation marks.
    // We allow `\"`, as well as `\u` and `\U` as escape sequences.
    static constexpr auto rule = [] {
        auto cp     = (dsl::code_point - dsl::ascii::control).error<invalid_character>;
        auto escape = dsl::backslash_escape                                //
                          .lit_c<'"'>()                                    //
                          .rule(dsl::lit_c<'u'> >> dsl::code_point_id<4>)  //
                          .rule(dsl::lit_c<'U'> >> dsl::code_point_id<8>); //

        return dsl::quoted(cp, escape);
    }();

    // Construct a UTF-8 string from the quoted content.
    static constexpr auto value = lexy::as_string<std::string, lexy::utf8_encoding>;
};

struct author_list
{
    // Match a comma separated (non-empty) list of authors surrounded by square brackets.
    static constexpr auto rule = dsl::square_bracketed.list(dsl::p<author>, dsl::sep(dsl::comma));

    // Collect all authors into a std::vector.
    static constexpr auto value = lexy::as_list<std::vector<std::string>>;
};

struct config
{
    struct unknown_field
    {
        static constexpr auto name = "unknown config field";
    };
    struct duplicate_field
    {
        static constexpr auto name = "duplicate config field";
    };

    // Whitespace is ' ' and '\t'.
    static constexpr auto whitespace = dsl::ascii::blank;

    static constexpr auto rule = [] {
        auto make_field
            = [](auto name, auto rule) { return name >> dsl::lit_c<'='> + rule + dsl::newline; };

        auto name_field    = make_field(LEXY_LIT("name"), LEXY_MEM(name) = dsl::p<name>);
        auto version_field = make_field(LEXY_LIT("version"), LEXY_MEM(version) = dsl::p<version>);
        auto authors_field
            = make_field(LEXY_LIT("authors"), LEXY_MEM(authors) = dsl::p<author_list>);

        auto combination = dsl::combination(name_field, version_field, authors_field)
                               .missing_error<unknown_field>.duplicate_error<duplicate_field>;
        return combination + dsl::whitespace(dsl::ascii::space) + dsl::eof;
    }();

    static constexpr auto value = lexy::as_aggregate<PackageConfig>;
};
} // namespace grammar

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

    auto result = lexy::parse<grammar::config>(file, lexy_ext::report_error);
    if (!result)
        return 2;
    auto& config = result.value();

    std::printf("Package %s (%d.%d.%d)\n", config.name.c_str(), config.version.major,
                config.version.minor, config.version.patch);

    std::puts("Created by:");
    for (auto& author : config.authors)
        std::puts(author.c_str());
}

