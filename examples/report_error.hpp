// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef EXAMPLE_REPORT_ERROR_HPP_INCLUDED
#define EXAMPLE_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>
#include <lexy/callback.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/newline.hpp>
#include <lexy/error.hpp>
#include <lexy/error_location.hpp>

namespace lexy_ex
{
template <typename Reader>
void _print_location(const lexy::error_location<Reader>& location)
{
    std::fputs("     | \n", stderr);

    std::fprintf(stderr, "%2zd:%2zd| ", location.line, location.column);
    for (auto c : location.context)
        std::fputc(c, stderr);
    std::fputc('\n', stderr);
}

template <typename Reader>
void _print_message_indent(const lexy::error_location<Reader>& location)
{
    std::fputs("     | ", stderr);
    for (auto i = 0u; i != location.column - 1; ++i)
        std::fputc(' ', stderr);

    // The next character printed will be under the error location of the column above.
}

// Print a generic error.
template <typename Reader, typename Tag>
void _print_message(const lexy::error_location<Reader>& location, const lexy::error<Reader, Tag>& e)
{
    if (e.begin() == e.end())
        std::fputc('^', stderr);
    else
    {
        for (auto cur = e.begin(); cur != e.end(); ++cur)
        {
            if (cur == location.context.end())
                break; // More than one line.
            std::fputc('^', stderr);
        }
    }

    std::fprintf(stderr, " %.*s", int(e.message().size()), e.message().data());
}

// Print an expected_literal error.
template <typename Reader>
void _print_message(const lexy::error_location<Reader>&,
                    const lexy::error<Reader, lexy::expected_literal>& e)
{
    std::fprintf(stderr, "^ expected '%.*s'", int(e.string().size()),
                 reinterpret_cast<const char*>(e.string().data()));
}

// Print an expected_char_class error.
template <typename Reader>
void _print_message(const lexy::error_location<Reader>&,
                    const lexy::error<Reader, lexy::expected_char_class>& e)
{
    std::fprintf(stderr, "^ expected '%.*s' character", int(e.character_class().size()),
                 e.character_class().data());
}

// The error callback that prints to stderr.
constexpr auto report_error = lexy::callback([](const auto& context, const auto& error) {
    // Convert the context location and error location into line/column information.
    auto context_location
        = lexy::make_error_location(context.input(), context.position(),
                                    lexy::dsl::ascii::character, lexy::dsl::newline);
    auto location = lexy::make_error_location(context.input(), error.position(),
                                              lexy::dsl::ascii::character, lexy::dsl::newline);

    // Print the main error headline.
    auto prod_name = context.production();
    std::fflush(stdout);
    std::fprintf(stderr, "error: while parsing %.*s\n", int(prod_name.size()), prod_name.data());

    if (location.line != context_location.line)
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
        for (auto i = context_location.column; i != location.column; ++i)
            std::fputc('~', stderr);
    }

    _print_message(location, error);
    std::putc('\n', stderr);
});
} // namespace lexy_ex

#endif // EXAMPLE_REPORT_ERROR_HPP_INCLUDED

