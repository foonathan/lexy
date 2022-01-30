// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED
#define LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED

#include <climits>
#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

namespace lexy::_detail
{
class ascii_table_t
{
public:
    enum category
    {
        control,
        space,
        alpha,
        alpha_underscore,
        digit, // 0-9 only
        hex_lower,
        hex_upper,
        punct,

        _count,
    };
    static_assert(_count <= CHAR_BIT);

    LEXY_CONSTEVAL ascii_table_t() : _table{}
    {
        for (char c = 0x00; c <= 0x1F; ++c)
            insert(c, control);
        insert(0x7F, control);

        insert(' ', space);
        insert('\t', space);
        insert('\n', space);
        insert('\r', space);
        insert('\f', space);
        insert('\v', space);

        for (auto c = 'A'; c <= 'Z'; ++c)
        {
            insert(c, alpha);
            insert(c, alpha_underscore);
        }
        for (auto c = 'a'; c <= 'z'; ++c)
        {
            insert(c, alpha);
            insert(c, alpha_underscore);
        }
        insert('_', alpha_underscore);

        for (auto c = '0'; c <= '9'; ++c)
        {
            insert(c, digit);
            insert(c, hex_lower);
            insert(c, hex_upper);
        }
        for (auto c = 'A'; c <= 'F'; ++c)
            insert(c, hex_upper);
        for (auto c = 'a'; c <= 'f'; ++c)
            insert(c, hex_lower);

        insert('!', punct);
        insert('"', punct);
        insert('#', punct);
        insert('$', punct);
        insert('%', punct);
        insert('&', punct);
        insert('\'', punct);
        insert('(', punct);
        insert(')', punct);
        insert('*', punct);
        insert('+', punct);
        insert(',', punct);
        insert('-', punct);
        insert('.', punct);
        insert('/', punct);
        insert(':', punct);
        insert(';', punct);
        insert('<', punct);
        insert('=', punct);
        insert('>', punct);
        insert('?', punct);
        insert('@', punct);
        insert('[', punct);
        insert('\\', punct);
        insert(']', punct);
        insert('^', punct);
        insert('_', punct);
        insert('`', punct);
        insert('{', punct);
        insert('|', punct);
        insert('}', punct);
        insert('~', punct);
    }

    template <typename Encoding, category... Cats>
    constexpr bool contains(typename Encoding::int_type i) const
    {
        constexpr auto mask = ((1 << Cats) | ...);

        if (i < lexy::_detail::transcode_int<Encoding>(0x00)
            || lexy::_detail::transcode_int<Encoding>(0x7F) < i)
            return false;

        // NOLINTNEXTLINE: We've checked that we're positive in the condition above.
        auto index = static_cast<std::size_t>(i);
        return (_table[index] & mask) != 0;
    }

private:
    constexpr void insert(char c, category cat)
    {
        auto as_unsigned = static_cast<unsigned char>(c);
        LEXY_PRECONDITION(as_unsigned <= 0x7F);

        _table[as_unsigned] = static_cast<unsigned char>(_table[as_unsigned] | 1 << cat);
    }

    unsigned char _table[0x80];
};

inline constexpr ascii_table_t ascii_table = {};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED

