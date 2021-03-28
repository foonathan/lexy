// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_STRING_HPP_INCLUDED
#define LEXY_CALLBACK_STRING_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>
#include <lexy/lexeme.hpp>

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
        auto                         size      = Encoding::encode_code_point(cp, buffer, 4);

        if constexpr (std::is_same_v<_char_type, typename Encoding::char_type>)
            return (*this)(buffer, size);
        else
            return (*this)(reinterpret_cast<const _char_type*>(buffer), size);
    }

    struct _sink
    {
        String _result{};

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
            auto                         size      = Encoding::encode_code_point(cp, buffer, 4);
            (*this)(reinterpret_cast<const _char_type*>(buffer), size);
        }

        String&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
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

