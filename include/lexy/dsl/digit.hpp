// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED

#include <lexy/_detail/ascii_table.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/char_class.hpp>
#include <lexy/engine/digits.hpp>

//=== bases ===//
namespace lexyd
{
struct binary
{
    static constexpr unsigned radix = 2;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.binary";
    }

    using digit_set = lexy::engine_char_range<'0', '1'>;

    template <typename CharT>
    LEXY_DSL_FUNC unsigned value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }
};

struct octal
{
    static constexpr unsigned radix = 8;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.octal";
    }

    using digit_set = lexy::engine_char_range<'0', '7'>;

    template <typename CharT>
    LEXY_DSL_FUNC unsigned value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }
};

struct decimal
{
    static constexpr unsigned radix = 10;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.decimal";
    }

    using digit_set = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                               lexy::_detail::ascii_table_digit>;

    template <typename CharT>
    LEXY_DSL_FUNC unsigned value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }
};

struct hex_lower
{
    static constexpr unsigned radix = 16;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.hex-lower";
    }

    using digit_set = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                               lexy::_detail::ascii_table_hex_lower>;

    template <typename CharT>
    LEXY_DSL_FUNC unsigned value(CharT c)
    {
        if (c >= 'a')
            return static_cast<unsigned>(c) - 'a' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }
};

struct hex_upper
{
    static constexpr unsigned radix = 16;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.hex-upper";
    }

    using digit_set = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                               lexy::_detail::ascii_table_hex_upper>;

    template <typename CharT>
    LEXY_DSL_FUNC unsigned value(CharT c)
    {
        if (c >= 'A')
            return static_cast<unsigned>(c) - 'A' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }
};

struct hex
{
    static constexpr unsigned radix = 16;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.hex";
    }

    using digit_set = lexy::engine_ascii_table<lexy::_detail::dsl_ascii_table,
                                               lexy::_detail::ascii_table_hex_lower,
                                               lexy::_detail::ascii_table_hex_upper>;

    template <typename CharT>
    LEXY_DSL_FUNC unsigned value(CharT c)
    {
        if (c >= 'a')
            return static_cast<unsigned>(c) - 'a' + 10;
        else if (c >= 'A')
            return static_cast<unsigned>(c) - 'A' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }
};
} // namespace lexyd

//=== digit ===//
namespace lexyd
{
struct _zero : token_base<_zero>
{
    static constexpr auto _trie = lexy::linear_trie<LEXY_NTTP_STRING("0")>;
    using token_engine          = lexy::engine_literal<_trie>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, "digit.zero");
        context.on(_ev::error{}, err);
    }
};

/// Matches the zero digit.
constexpr auto zero = _zero{};

template <typename Base>
struct _digit : token_base<_digit<Base>>
{
    using token_engine = typename Base::digit_set;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.on(_ev::error{}, err);
    }
};

/// Matches a single digit.
template <typename Base = decimal>
constexpr auto digit = _digit<Base>{};
} // namespace lexyd

//=== digits ===//
namespace lexy
{
struct forbidden_leading_zero
{
    static LEXY_CONSTEVAL auto name()
    {
        return "forbidden leading zero";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Base, typename Sep>
struct _digits_st : token_base<_digits_st<Base, Sep>>
{
    using token_engine
        = lexy::engine_digits_trimmed_sep<typename Base::digit_set, _zero::token_engine,
                                          typename Sep::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::leading_zero)
        {
            auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(pos, reader.position());
            context.on(_ev::error{}, err);
        }
        else
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
            context.on(_ev::error{}, err);
        }
    }
};

template <typename Base, typename Sep>
struct _digits_s : token_base<_digits_s<Base, Sep>>
{
    using token_engine
        = lexy::engine_digits_sep<typename Base::digit_set, typename Sep::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.on(_ev::error{}, err);
    }

    constexpr auto no_leading_zero() const
    {
        return _digits_st<Base, Sep>{};
    }
};

template <typename Base>
struct _digits_t : token_base<_digits_t<Base>>
{
    using token_engine = lexy::engine_digits_trimmed<typename Base::digit_set, _zero::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::leading_zero)
        {
            auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(pos, reader.position());
            context.on(_ev::error{}, err);
        }
        else
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
            context.on(_ev::error{}, err);
        }
    }

    template <typename Token>
    constexpr auto sep(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _digits_st<Base, Token>{};
    }
};

template <typename Base>
struct _digits : token_base<_digits<Base>>
{
    using token_engine = lexy::engine_digits<typename Base::digit_set>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.on(_ev::error{}, err);
    }

    template <typename Token>
    constexpr auto sep(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _digits_s<Base, Token>{};
    }

    constexpr auto no_leading_zero() const
    {
        return _digits_t<Base>{};
    }
};

/// Matches a non-empty list of digits.
template <typename Base = decimal>
constexpr auto digits = _digits<Base>{};

constexpr auto digit_sep_underscore = LEXY_LIT("_");
constexpr auto digit_sep_tick       = LEXY_LIT("'");
} // namespace lexyd

//=== n_digits ===//
namespace lexyd
{
template <std::size_t N, typename Base, typename Sep>
struct _ndigits_s : token_base<_ndigits_s<N, Base, Sep>>
{
    using token_engine
        = lexy::engine_ndigits_sep<N, typename Base::digit_set, typename Sep::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.on(_ev::error{}, err);
    }
};

template <std::size_t N, typename Base>
struct _ndigits : token_base<_ndigits<N, Base>>
{
    static_assert(N > 1);

    using token_engine = lexy::engine_ndigits<N, typename Base::digit_set>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.on(_ev::error{}, err);
    }

    template <typename Token>
    constexpr auto sep(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _ndigits_s<N, Base, Token>{};
    }
};

/// Matches exactly N digits.
template <std::size_t N, typename Base = decimal>
constexpr auto n_digits = _ndigits<N, Base>{};
} // namespace lexyd

#endif // LEXY_DSL_DIGIT_HPP_INCLUDED

