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

    std::fputs("     | ", stderr);
    for (auto i = 0u; i != location.column - 1; ++i)
        std::fputc(' ', stderr);

    // The next character printed will be under the error location of the column above.
}

template <typename Reader, typename Tag>
void _print_location_msg(const lexy::error_location<Reader>& location,
                         const lexy::error<Reader, Tag>&     e)
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
template <typename Reader>
void _print_location_msg(const lexy::error_location<Reader>&,
                         const lexy::error<Reader, lexy::expected_literal>& e)
{
    std::fprintf(stderr, "^ expected '%.*s'", int(e.string().size()), e.string().data());
}
template <typename Reader>
void _print_location_msg(const lexy::error_location<Reader>&,
                         const lexy::error<Reader, lexy::expected_char_class>& e)
{
    std::fprintf(stderr, "^ expected '%.*s' character", int(e.character_class().size()),
                 e.character_class().data());
}

constexpr auto report_error = lexy::callback([](const auto& context, const auto& error) {
    auto context_location
        = lexy::make_error_location(context.input(), context.position(),
                                    lexy::dsl::ascii::character, lexy::dsl::newline);
    auto location = lexy::make_error_location(context.input(), error.position(),
                                              lexy::dsl::ascii::character, lexy::dsl::newline);

    auto prod_name = context.production();
    std::fprintf(stderr, "error: while parsing %.*s\n", int(prod_name.size()), prod_name.data());

    if (location.line != context_location.line)
    {
        _print_location(context_location);
        std::fputs("^ beginning here\n", stderr);
        _print_location(location);
    }
    else
    {
        _print_location(context_location);
        for (auto i = context_location.column; i != location.column; ++i)
            std::fputc('~', stderr);
    }

    _print_location_msg(location, error);
});
} // namespace lexy_ex

#endif // EXAMPLE_REPORT_ERROR_HPP_INCLUDED

