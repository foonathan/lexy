// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
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

    using digit_set = lexy::engine_char_range<'0', '9'>;

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

    struct digit_set : lexy::engine_matcher_base
    {
        using dec   = lexy::engine_char_range<'0', '9'>;
        using lower = lexy::engine_char_range<'a', 'f'>;

        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (lexy::engine_try_match<dec>(reader) || lexy::engine_try_match<lower>(reader))
                return error_code();
            else
                return error_code::error;
        }
    };

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

    struct digit_set : lexy::engine_matcher_base
    {
        using dec   = lexy::engine_char_range<'0', '9'>;
        using upper = lexy::engine_char_range<'A', 'F'>;

        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (lexy::engine_try_match<dec>(reader) || lexy::engine_try_match<upper>(reader))
                return error_code();
            else
                return error_code::error;
        }
    };

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

    struct digit_set : lexy::engine_matcher_base
    {
        using dec   = lexy::engine_char_range<'0', '9'>;
        using lower = lexy::engine_char_range<'a', 'f'>;
        using upper = lexy::engine_char_range<'A', 'F'>;

        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (lexy::engine_try_match<dec>(reader) || lexy::engine_try_match<lower>(reader)
                || lexy::engine_try_match<upper>(reader))
                return error_code();
            else
                return error_code::error;
        }
    };

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

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "digit.zero");
        return LEXY_MOV(handler).error(err);
    }
};

template <typename Base>
struct _nzero : token_base<_nzero<Base>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            not_a_digit = 1,
            zero        = 2,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            using encoding = typename Reader::encoding;

            if (reader.peek() == encoding::to_int_type('0'))
                return error_code::zero;
            else if (!lexy::engine_try_match<typename Base::digit_set>(reader))
                return error_code::not_a_digit;
            else
                return error_code();
        }
    };

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler&                          handler, const Reader&,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        auto name = ec == token_engine::error_code::zero ? "digit.zero" : Base::Name();
        auto err  = lexy::make_error<Reader, lexy::expected_char_class>(pos, name);
        return LEXY_MOV(handler).error(err);
    }
};

template <typename Base>
struct _digit : token_base<_digit<Base>>
{
    using token_engine = typename Base::digit_set;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        return LEXY_MOV(handler).error(err);
    }

    //=== dsl ===//
    LEXY_CONSTEVAL auto zero() const
    {
        return _zero{};
    }

    LEXY_CONSTEVAL auto non_zero() const
    {
        return _nzero<Base>{};
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

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::leading_zero)
        {
            auto err = lexy::make_error<Reader, lexy::forbidden_leading_zero>(pos, reader.cur());
            return LEXY_MOV(handler).error(err);
        }
        else
        {
            auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
            return LEXY_MOV(handler).error(err);
        }
    }
};

template <typename Base, typename Sep>
struct _digits_s : token_base<_digits_s<Base, Sep>>
{
    using token_engine
        = lexy::engine_digits_sep<typename Base::digit_set, typename Sep::token_engine>;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        return LEXY_MOV(handler).error(err);
    }

    LEXY_CONSTEVAL auto no_leading_zero() const
    {
        return _digits_st<Base, Sep>{};
    }
};

template <typename Base>
struct _digits_t : token_base<_digits_t<Base>>
{
    using token_engine = lexy::engine_digits_trimmed<typename Base::digit_set, _zero::token_engine>;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::leading_zero)
        {
            auto err = lexy::make_error<Reader, lexy::forbidden_leading_zero>(pos, reader.cur());
            return LEXY_MOV(handler).error(err);
        }
        else
        {
            auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
            return LEXY_MOV(handler).error(err);
        }
    }

    template <typename Token>
    LEXY_CONSTEVAL auto sep(Token) const
    {
        static_assert(lexy::is_token<Token>);
        return _digits_st<Base, Token>{};
    }
};

template <typename Base>
struct _digits : token_base<_digits<Base>>
{
    using token_engine = lexy::engine_digits<typename Base::digit_set>;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        return LEXY_MOV(handler).error(err);
    }

    template <typename Token>
    LEXY_CONSTEVAL auto sep(Token) const
    {
        static_assert(lexy::is_token<Token>);
        return _digits_s<Base, Token>{};
    }

    LEXY_CONSTEVAL auto no_leading_zero() const
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

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        return LEXY_MOV(handler).error(err);
    }
};

template <std::size_t N, typename Base>
struct _ndigits : token_base<_ndigits<N, Base>>
{
    static_assert(N > 1);

    using token_engine = lexy::engine_ndigits<N, typename Base::digit_set>;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        return LEXY_MOV(handler).error(err);
    }

    template <typename Token>
    LEXY_CONSTEVAL auto sep(Token) const
    {
        static_assert(lexy::is_token<Token>);
        return _ndigits_s<N, Base, Token>{};
    }
};

/// Matches exactly N digits.
template <std::size_t N, typename Base = decimal>
constexpr auto n_digits = _ndigits<N, Base>{};
} // namespace lexyd

#endif // LEXY_DSL_DIGIT_HPP_INCLUDED

