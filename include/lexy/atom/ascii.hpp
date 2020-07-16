// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ATOM_ASCII_HPP_INCLUDED
#define LEXY_ATOM_ASCII_HPP_INCLUDED

#include <lexy/atom/base.hpp>

namespace lexy
{
struct expected_char_class
{
    template <typename Input>
    class error
    {
    public:
        constexpr explicit error(typename Input::iterator pos, const char* name) noexcept
        : _pos(pos), _name(name)
        {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

        constexpr auto character_class() const noexcept
        {
            return _name;
        }

    private:
        typename Input::iterator _pos;
        const char*              _name;
    };
};
} // namespace lexy

namespace lexyd::ascii
{
template <typename Predicate>
struct _ascii : atom_base
{
    template <typename Input>
    LEXY_ATOM_FUNC bool match(Input& input)
    {
        if (!Predicate::match(input.peek()))
            return false;
        input.bump();
        return true;
    }

    template <typename Input>
    LEXY_ATOM_FUNC auto error(const Input&, typename Input::iterator pos)
    {
        return lexy::expected_char_class::error<Input>(pos, Predicate::name());
    }
};

//=== control ===//
struct _control
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.control";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return (0x0 <= c && c <= 0x8) || (0xE <= c && c <= 0x1F) || c == 0x7F;
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

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c == ' ' || c == '\t';
    }
};
inline constexpr auto blank = _ascii<_blank>{};

struct _newline
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.newline";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c == '\n' || c == '\r' || c == '\f' || c == '\v';
    }
};
inline constexpr auto newline = _ascii<_newline>{};

struct _space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.space";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return _blank::match(c) || _newline::match(c);
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

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c >= 'a' && c <= 'z';
    }
};
inline constexpr auto lower = _ascii<_lower>{};

struct _upper
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.upper";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c >= 'A' && c <= 'Z';
    }
};
inline constexpr auto upper = _ascii<_upper>{};

struct _alpha
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alpha";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return _lower::match(c) || _upper::match(c);
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

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c >= '0' && c <= '9';
    }
};
inline constexpr auto digit = _ascii<_digit>{};

struct _alnum
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alnum";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return _lower::match(c) || _upper::match(c) || _digit::match(c);
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

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c == '!' || c == '"' || c == '#' || c == '$' || c == '%' || c == '&' || c == '\''
               || c == '(' || c == ')' || c == '*' || c == '+' || c == ',' || c == '-' || c == '.'
               || c == '/' || c == ':' || c == ';' || c == '<' || c == '=' || c == '>' || c == '?'
               || c == '@' || c == '[' || c == '\\' || c == ']' || c == '^' || c == '_' || c == '`'
               || c == '{' || c == '|' || c == '}' || c == '~';
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

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return _lower::match(c) || _upper::match(c) || _digit::match(c) || _punct::match(c);
    }
};
inline constexpr auto graph = _ascii<_graph>{};

struct _print
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.print";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return c == ' ' || _lower::match(c) || _upper::match(c) || _digit::match(c)
               || _punct::match(c);
    }
};
inline constexpr auto print = _ascii<_print>{};

struct _char
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII";
    }

    template <typename T>
    LEXY_ATOM_FUNC bool match(T c)
    {
        return 0 <= c && c <= 0x7F;
    }
};
inline constexpr auto character = _ascii<_char>{};
} // namespace lexyd::ascii

#endif // LEXY_ATOM_ASCII_HPP_INCLUDED

