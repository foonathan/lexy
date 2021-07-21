// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>
#include <lexy/callback/adapter.hpp>
#include <lexy/error.hpp>
#include <lexy_ext/input_location.hpp>

namespace lexy_ext
{
template <typename Location>
void _print_location(Location location)
{
    std::fputs("     | \n", stderr);

    std::fprintf(stderr, "%2zd:%2zd| ", location.line_nr(), location.column_nr());
    for (auto c : location.context())
        std::fputc(c, stderr);
    std::fputc('\n', stderr);
}

template <typename Location>
void _print_message_indent(Location location)
{
    std::fputs("     | ", stderr);
    for (auto i = 0u; i != location.column_nr() - 1; ++i)
        std::fputc(' ', stderr);

    // The next character printed will be under the error location of the column above.
}

// Print a generic error.
template <typename Location, typename Reader, typename Tag>
void _print_message(Location location, const lexy::error<Reader, Tag>& e)
{
    if (e.begin() == e.end())
        std::fputc('^', stderr);
    else
    {
        for (auto cur = e.begin(); cur != e.end(); ++cur)
        {
            if (cur == location.context().end())
                break; // More than one line.
            std::fputc('^', stderr);
        }
    }

    std::fprintf(stderr, " %s", e.message());
}

// Print an expected_literal error.
template <typename Location, typename Reader>
void _print_message(Location, const lexy::error<Reader, lexy::expected_literal>& e)
{
    for (auto i = 0u; i != e.index(); ++i)
        std::fputc('^', stderr);
    std::fprintf(stderr, "^ expected '%s'", reinterpret_cast<const char*>(e.string()));
}

// Print an expected_keyword error.
template <typename Location, typename Reader>
void _print_message(Location, const lexy::error<Reader, lexy::expected_keyword>& e)
{
    if (e.begin() == e.end())
        std::fputc('^', stderr);
    else
    {
        for (auto cur = e.begin(); cur != e.end(); ++cur)
            std::fputc('^', stderr);
    }

    std::fprintf(stderr, " expected keyword '%s'", reinterpret_cast<const char*>(e.string()));
}

// Print an expected_char_class error.
template <typename Location, typename Reader>
void _print_message(Location, const lexy::error<Reader, lexy::expected_char_class>& e)
{
    std::fprintf(stderr, "^ expected '%s' character", e.character_class());
}

// The error callback that prints to stderr.
constexpr auto report_error = lexy::callback([](const auto& context, const auto& error) {
    lexy_ext::input_location_finder finder(context.input());

    // Convert the context location and error location into line/column information.
    auto context_location = finder.find(context.position());
    // Error position is after context position.
    auto location = finder.find(error.position(), context_location);

    // Print the main error headline.
    auto prod_name = context.production();
    std::fflush(stdout);
    std::fprintf(stderr, "error: while parsing %s\n", prod_name);

    if (location.line_nr() != context_location.line_nr())
    {
        _print_location(context_location);
        _print_message_indent(context_location);
        std::fputs("^ beginning here\n", stderr);
        _print_location(location);
        _print_message_indent(location);
    }
    else
    {
        _print_location(location);
        _print_message_indent(context_location);
        for (auto i = context_location.column_nr(); i != location.column_nr(); ++i)
            std::fputc('~', stderr);
    }

    _print_message(location, error);
    std::fputs("\n------\n", stderr);
});
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

