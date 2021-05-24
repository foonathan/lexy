// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_STRING_HPP_INCLUDED
#define LEXY_CALLBACK_STRING_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>
#include <lexy/lexeme.hpp>

namespace lexy::_detail
{
template <typename Encoding>
struct encode_code_point
{
    static_assert(lexy::_detail::error<Encoding>, "cannot encode a code point in this encoding");
};
template <>
struct encode_code_point<lexy::ascii_encoding>
{
    static constexpr std::size_t encode(code_point cp, char* buffer, std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_ascii());
        LEXY_PRECONDITION(size >= 1);

        *buffer = char(cp.value());
        return 1;
    }
};
template <>
struct encode_code_point<lexy::utf8_encoding>
{
    static constexpr std::size_t encode(code_point cp, LEXY_CHAR8_T* buffer, std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());

        // Taken from http://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html.
        if (cp.is_ascii())
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = LEXY_CHAR8_T(cp.value());
            return 1;
        }
        else if (cp.value() <= 0x07'FF)
        {
            LEXY_PRECONDITION(size >= 2);

            auto first  = (cp.value() >> 6) & 0x1F;
            auto second = (cp.value() >> 0) & 0x3F;

            buffer[0] = LEXY_CHAR8_T(0xC0 | first);
            buffer[1] = LEXY_CHAR8_T(0x80 | second);
            return 2;
        }
        else if (cp.value() <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 3);

            auto first  = (cp.value() >> 12) & 0x0F;
            auto second = (cp.value() >> 6) & 0x3F;
            auto third  = (cp.value() >> 0) & 0x3F;

            buffer[0] = LEXY_CHAR8_T(0xE0 | first);
            buffer[1] = LEXY_CHAR8_T(0x80 | second);
            buffer[2] = LEXY_CHAR8_T(0x80 | third);
            return 3;
        }
        else
        {
            LEXY_PRECONDITION(size >= 4);

            auto first  = (cp.value() >> 18) & 0x07;
            auto second = (cp.value() >> 12) & 0x3F;
            auto third  = (cp.value() >> 6) & 0x3F;
            auto fourth = (cp.value() >> 0) & 0x3F;

            buffer[0] = LEXY_CHAR8_T(0xF0 | first);
            buffer[1] = LEXY_CHAR8_T(0x80 | second);
            buffer[2] = LEXY_CHAR8_T(0x80 | third);
            buffer[3] = LEXY_CHAR8_T(0x80 | fourth);
            return 4;
        }
    }
};
template <>
struct encode_code_point<lexy::utf16_encoding>
{
    static constexpr std::size_t encode(code_point cp, char16_t* buffer, std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());

        if (cp.is_bmp())
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char16_t(cp.value());
            return 1;
        }
        else
        {
            // Algorithm implemented from
            // https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF.
            LEXY_PRECONDITION(size >= 2);

            auto u_prime       = cp.value() - 0x1'0000;
            auto high_ten_bits = u_prime >> 10;
            auto low_ten_bits  = u_prime & 0b0000'0011'1111'1111;

            buffer[0] = char16_t(0xD800 + high_ten_bits);
            buffer[1] = char16_t(0xDC00 + low_ten_bits);
            return 2;
        }
    }
};
template <>
struct encode_code_point<lexy::utf32_encoding>
{
    static constexpr std::size_t encode(code_point cp, char32_t* buffer, std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());
        LEXY_PRECONDITION(size >= 1);

        *buffer = char32_t(cp.value());
        return 1;
    }
};
} // namespace lexy::_detail

namespace lexy
{
template <typename String>
using _string_char_type = std::decay_t<decltype(LEXY_DECLVAL(String)[0])>;

template <typename String, typename Encoding>
struct _as_string
{
    using return_type = String;
    using _char_type  = _string_char_type<String>;

    constexpr String operator()(String&& str) const
    {
        return LEXY_MOV(str);
    }
    constexpr String operator()(const String& str) const
    {
        return str;
    }

    template <typename CharT>
    constexpr auto operator()(const CharT* str, std::size_t length) const
        -> decltype(String(str, length))
    {
        return String(str, length);
    }

    template <typename Reader>
    constexpr String operator()(lexeme<Reader> lex) const
    {
        using iterator = typename lexeme<Reader>::iterator;
        if constexpr (std::is_pointer_v<iterator>)
        {
            static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                          "cannot convert lexeme to this string type");

            if constexpr (std::is_same_v<_char_type, typename Reader::encoding::char_type>)
                return String(lex.data(), lex.size());
            else
                return String(reinterpret_cast<const _char_type*>(lex.data()), lex.size());
        }
        else
        {
            // We're assuming the string constructor can do any necessary conversion/transcoding.
            return String(lex.begin(), lex.end());
        }
    }

    constexpr String operator()(code_point cp) const
    {
        typename Encoding::char_type buffer[4] = {};
        auto size = _detail::encode_code_point<Encoding>::encode(cp, buffer, 4);

        if constexpr (std::is_same_v<_char_type, typename Encoding::char_type>)
            return (*this)(buffer, size);
        else
            return (*this)(reinterpret_cast<const _char_type*>(buffer), size);
    }

    struct _sink
    {
        String _result;

        using return_type = String;

        template <typename CharT>
        auto operator()(CharT c) -> decltype(_result.push_back(c))
        {
            return _result.push_back(c);
        }

        void operator()(const String& str)
        {
            _result.append(str);
        }
        void operator()(String&& str)
        {
            _result.append(LEXY_MOV(str));
        }

        template <typename CharT>
        auto operator()(const CharT* str, std::size_t length)
            -> decltype(_result.append(str, length))
        {
            return _result.append(str, length);
        }

        template <typename Reader>
        void operator()(lexeme<Reader> lex)
        {
            using iterator = typename lexeme<Reader>::iterator;
            if constexpr (std::is_pointer_v<iterator>)
            {
                static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                              "cannot convert lexeme to this string type");
                _result.append(reinterpret_cast<const _char_type*>(lex.data()), lex.size());
            }
            else
            {
                // We're assuming the string append function can do any necessary
                // conversion/transcoding.
                _result.append(lex.begin(), lex.end());
            }
        }

        void operator()(code_point cp)
        {
            typename Encoding::char_type buffer[4] = {};
            auto size = _detail::encode_code_point<Encoding>::encode(cp, buffer, 4);
            (*this)(reinterpret_cast<const _char_type*>(buffer), size);
        }

        String&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{String()};
    }
    template <typename S = String>
    constexpr auto sink(const typename S::allocator_type& allocator) const
    {
        return _sink{String(allocator)};
    }
};

/// A callback with sink that creates a string (e.g. `std::string`).
/// As a callback, it converts a lexeme into the string.
/// As a sink, it repeatedly calls `.push_back()` for individual characters,
/// or `.append()` for lexemes or other strings.
template <typename String, typename Encoding = deduce_encoding<_string_char_type<String>>>
constexpr auto as_string = _as_string<String, Encoding>{};
} // namespace lexy

#endif // LEXY_CALLBACK_STRING_HPP_INCLUDED

