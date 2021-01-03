// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED
#define LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED

#include <lexy/engine/char_class.hpp>

namespace lexy::_detail
{
// The categories in the lookup table.
enum ascii_table_categories
{
    ascii_table_control,
    ascii_table_space,
    ascii_table_alpha,
    ascii_table_digit, // 0-9 only
    ascii_table_hex_lower,
    ascii_table_hex_upper,
    ascii_table_punct,

    _ascii_table_count,
};
static_assert(_ascii_table_count <= CHAR_BIT);

// The actual table the lookup code uses.
constexpr auto dsl_ascii_table = [] {
    lexy::ascii_table<_ascii_table_count> result;

    for (auto c = 0x00; c <= 0x1F; ++c)
        result.insert(c, ascii_table_control);
    result.insert(0x7F, ascii_table_control);

    result.insert(' ', ascii_table_space);
    result.insert('\t', ascii_table_space);
    result.insert('\n', ascii_table_space);
    result.insert('\r', ascii_table_space);
    result.insert('\f', ascii_table_space);
    result.insert('\v', ascii_table_space);

    for (auto c = 'A'; c <= 'Z'; ++c)
        result.insert(c, ascii_table_alpha);
    for (auto c = 'a'; c <= 'z'; ++c)
        result.insert(c, ascii_table_alpha);

    for (auto c = '0'; c <= '9'; ++c)
    {
        result.insert(c, ascii_table_digit);
        result.insert(c, ascii_table_hex_lower);
        result.insert(c, ascii_table_hex_upper);
    }
    for (auto c = 'A'; c <= 'F'; ++c)
        result.insert(c, ascii_table_hex_upper);
    for (auto c = 'a'; c <= 'f'; ++c)
        result.insert(c, ascii_table_hex_lower);

    result.insert('!', ascii_table_punct);
    result.insert('"', ascii_table_punct);
    result.insert('#', ascii_table_punct);
    result.insert('$', ascii_table_punct);
    result.insert('%', ascii_table_punct);
    result.insert('&', ascii_table_punct);
    result.insert('\'', ascii_table_punct);
    result.insert('(', ascii_table_punct);
    result.insert(')', ascii_table_punct);
    result.insert('*', ascii_table_punct);
    result.insert('+', ascii_table_punct);
    result.insert(',', ascii_table_punct);
    result.insert('-', ascii_table_punct);
    result.insert('.', ascii_table_punct);
    result.insert('/', ascii_table_punct);
    result.insert(':', ascii_table_punct);
    result.insert(';', ascii_table_punct);
    result.insert('<', ascii_table_punct);
    result.insert('=', ascii_table_punct);
    result.insert('>', ascii_table_punct);
    result.insert('?', ascii_table_punct);
    result.insert('@', ascii_table_punct);
    result.insert('[', ascii_table_punct);
    result.insert('\\', ascii_table_punct);
    result.insert(']', ascii_table_punct);
    result.insert('^', ascii_table_punct);
    result.insert('_', ascii_table_punct);
    result.insert('`', ascii_table_punct);
    result.insert('{', ascii_table_punct);
    result.insert('|', ascii_table_punct);
    result.insert('}', ascii_table_punct);
    result.insert('~', ascii_table_punct);

    return result;
}();
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED

