// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ASCII_HPP_INCLUDED
#define LEXY_DSL_ASCII_HPP_INCLUDED

#include <lexy/_detail/ascii_table.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/char_class.hpp>

namespace lexyd::ascii
{
template <typename Derived>
struct _ascii : token_base<Derived>
{
    template <typename Context, typename Reader, typename ErrorCode>
    static constexpr void token_error(Context&                  context, const Reader&, ErrorCode,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Derived::name());
        context.error(err);
    }
};

//=== control ===//
struct _control : _ascii<_control>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.control";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_control>;
};
inline constexpr auto control = _control{};

//=== whitespace ===//
struct _blank : _ascii<_blank>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.blank";
    }

    static constexpr auto _trie = lexy::shallow_trie<LEXY_NTTP_STRING(" \t")>;
    using token_engine          = lexy::engine_char_set<_trie>;
};
inline constexpr auto blank = _blank{};

struct _newline : _ascii<_newline>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.newline";
    }

    static constexpr auto _trie = lexy::shallow_trie<LEXY_NTTP_STRING("\n\r")>;
    using token_engine          = lexy::engine_char_set<_trie>;
};
inline constexpr auto newline = _newline{};

struct _other_space : _ascii<_other_space>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.other-space";
    }

    static constexpr auto _trie = lexy::shallow_trie<LEXY_NTTP_STRING("\f\v")>;
    using token_engine          = lexy::engine_char_set<_trie>;
};
inline constexpr auto other_space = _other_space{};

struct _space : _ascii<_space>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.space";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_space>;
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : _ascii<_lower>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.lower";
    }

    using token_engine = lexy::engine_char_range<'a', 'z'>;
};
inline constexpr auto lower = _lower{};

struct _upper : _ascii<_upper>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.upper";
    }

    using token_engine = lexy::engine_char_range<'A', 'Z'>;
};
inline constexpr auto upper = _upper{};

struct _alpha : _ascii<_alpha>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alpha";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_alpha>;
};
inline constexpr auto alpha = _alpha{};

struct _alphau : _ascii<_alphau>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alpha-underscore";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_alpha_underscore>;
};
inline constexpr auto alpha_underscore = _alphau{};

//=== digit ===//
struct _digit : _ascii<_digit>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.digit";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_digit>;
};
inline constexpr auto digit = _digit{};

struct _alnum : _ascii<_alnum>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alpha-digit";
    }

    using token_engine
        = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table, lexy::_detail::ascii_table_alpha,
                                   lexy::_detail::ascii_table_digit>;
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = _alnum{};

struct _alnumu : _ascii<_alnumu>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.alpha-digit-underscore";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_alpha_underscore,
                                                  lexy::_detail::ascii_table_digit>;
};
inline constexpr auto alpha_digit_underscore = _alnumu{};

//=== punct ===//
struct _punct : _ascii<_punct>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.punct";
    }

    using token_engine = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                                  lexy::_detail::ascii_table_punct>;
};
inline constexpr auto punct = _punct{};

//=== categories ===//
struct _graph : _ascii<_graph>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.graph";
    }

    using token_engine
        = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table, lexy::_detail::ascii_table_alpha,
                                   lexy::_detail::ascii_table_digit,
                                   lexy::_detail::ascii_table_punct>;
};
inline constexpr auto graph = _graph{};

struct _print : _ascii<_print>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.print";
    }

    struct token_engine : lexy::engine_matcher_base
    {
        using error_code = _graph::token_engine::error_code;

        template <typename Reader>
        static constexpr auto match(Reader& reader)
        {
            using encoding = typename Reader::encoding;
            if (reader.peek() == lexy::_char_to_int_type<encoding>(' '))
            {
                reader.bump();
                return error_code();
            }
            else
                return _graph::token_engine::match(reader);
        }
    };
};
inline constexpr auto print = _print{};

struct _char : _ascii<_char>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII";
    }

    using token_engine = lexy::engine_char_range<0x00, 0x7F>;
};
inline constexpr auto character = _char{};
} // namespace lexyd::ascii

#endif // LEXY_DSL_ASCII_HPP_INCLUDED

