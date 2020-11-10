// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ASCII_HPP_INCLUDED
#define LEXY_DSL_ASCII_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd::ascii
{
template <typename Predicate>
struct _ascii : atom_base<_ascii<Predicate>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        if (!Predicate::template match<typename Reader::encoding>(reader.peek()))
            return false;
        reader.bump();
        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        return lexy::make_error<Reader, lexy::expected_char_class>(pos, Predicate::name());
    }

    constexpr bool operator()(char c) const
    {
        return Predicate::template match<lexy::ascii_encoding>(int(c));
    }
};

//=== control ===//
struct _control
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.control";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return (Encoding::to_int_type(0x0) <= c && c <= Encoding::to_int_type(0x1F))
               || c == Encoding::to_int_type(0x7F);
    }
};
inline constexpr auto control = _ascii<_control>{};

//=== whitespace ===//
struct _blank
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.blank";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c == Encoding::to_int_type(' ') || c == Encoding::to_int_type('\t');
    }
};
inline constexpr auto blank = _ascii<_blank>{};

struct _newline
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.newline";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c == Encoding::to_int_type('\n') || c == Encoding::to_int_type('\r');
    }
};
inline constexpr auto newline = _ascii<_newline>{};

struct _other_space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.other-space";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c == Encoding::to_int_type('\f') || c == Encoding::to_int_type('\v');
    }
};
inline constexpr auto other_space = _ascii<_other_space>{};

struct _space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.space";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return _blank::template match<Encoding>(c) || _newline::template match<Encoding>(c)
               || _other_space::template match<Encoding>(c);
    }
};
inline constexpr auto space = _ascii<_space>{};

//=== alpha ===//
struct _lower
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.lower";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c >= Encoding::to_int_type('a') && c <= Encoding::to_int_type('z');
    }
};
inline constexpr auto lower = _ascii<_lower>{};

struct _upper
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.upper";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c >= Encoding::to_int_type('A') && c <= Encoding::to_int_type('Z');
    }
};
inline constexpr auto upper = _ascii<_upper>{};

struct _alpha
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alpha";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return _lower::template match<Encoding>(c) || _upper::template match<Encoding>(c);
    }
};
inline constexpr auto alpha = _ascii<_alpha>{};

//=== digit ===//
struct _digit
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.digit";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c >= Encoding::to_int_type('0') && c <= Encoding::to_int_type('9');
    }
};
inline constexpr auto digit = _ascii<_digit>{};

struct _alnum
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alnum";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return _lower::template match<Encoding>(c) || _upper::template match<Encoding>(c)
               || _digit::template match<Encoding>(c);
    }
};
inline constexpr auto alnum = _ascii<_alnum>{};

//=== punct ===//
struct _punct
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.punct";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c == Encoding::to_int_type('!') || c == Encoding::to_int_type('"')
               || c == Encoding::to_int_type('#') || c == Encoding::to_int_type('$')
               || c == Encoding::to_int_type('%') || c == Encoding::to_int_type('&')
               || c == Encoding::to_int_type('\'') || c == Encoding::to_int_type('(')
               || c == Encoding::to_int_type(')') || c == Encoding::to_int_type('*')
               || c == Encoding::to_int_type('+') || c == Encoding::to_int_type(',')
               || c == Encoding::to_int_type('-') || c == Encoding::to_int_type('.')
               || c == Encoding::to_int_type('/') || c == Encoding::to_int_type(':')
               || c == Encoding::to_int_type(';') || c == Encoding::to_int_type('<')
               || c == Encoding::to_int_type('=') || c == Encoding::to_int_type('>')
               || c == Encoding::to_int_type('?') || c == Encoding::to_int_type('@')
               || c == Encoding::to_int_type('[') || c == Encoding::to_int_type('\\')
               || c == Encoding::to_int_type(']') || c == Encoding::to_int_type('^')
               || c == Encoding::to_int_type('_') || c == Encoding::to_int_type('`')
               || c == Encoding::to_int_type('{') || c == Encoding::to_int_type('|')
               || c == Encoding::to_int_type('}') || c == Encoding::to_int_type('~');
    }
};
inline constexpr auto punct = _ascii<_punct>{};

//=== categories ===//
struct _graph
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.graph";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return _lower::template match<Encoding>(c) || _upper::template match<Encoding>(c)
               || _digit::template match<Encoding>(c) || _punct::template match<Encoding>(c);
    }
};
inline constexpr auto graph = _ascii<_graph>{};

struct _print
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.print";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c == Encoding::to_int_type(' ') || _lower::template match<Encoding>(c)
               || _upper::template match<Encoding>(c) || _digit::template match<Encoding>(c)
               || _punct::template match<Encoding>(c);
    }
};
inline constexpr auto print = _ascii<_print>{};

struct _char
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return Encoding::to_int_type(0) <= c && c <= Encoding::to_int_type(0x7F);
    }
};
inline constexpr auto character = _ascii<_char>{};
} // namespace lexyd::ascii

#endif // LEXY_DSL_ASCII_HPP_INCLUDED
