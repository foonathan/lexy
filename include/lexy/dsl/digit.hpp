// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED

#include <lexy/_detail/ascii_table.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

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

    template <typename Encoding>
    static constexpr bool match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>('0')
               || i == lexy::_char_to_int_type<Encoding>('1');
    }

    template <typename CharT>
    static constexpr unsigned value(CharT c)
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

    template <typename Encoding>
    static constexpr bool match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('0') <= i
               && i <= lexy::_char_to_int_type<Encoding>('7');
    }

    template <typename CharT>
    static constexpr unsigned value(CharT c)
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

    template <typename Encoding>
    static constexpr bool match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::digit>(i);
    }

    template <typename CharT>
    static constexpr unsigned value(CharT c)
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

    template <typename Encoding>
    static constexpr bool match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::hex_lower>(i);
    }

    template <typename CharT>
    static constexpr unsigned value(CharT c)
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

    template <typename Encoding>
    static constexpr bool match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::hex_upper>(i);
    }

    template <typename CharT>
    static constexpr unsigned value(CharT c)
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

    template <typename Encoding>
    static constexpr bool match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::hex_lower, ascii_table_t::hex_upper>(
            i);
    }

    template <typename CharT>
    static constexpr unsigned value(CharT c)
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
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            if (reader.peek() != lexy::_char_to_int_type<typename Reader::encoding>('0'))
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), "digit.zero");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches the zero digit.
constexpr auto zero = _zero{};

template <typename Base>
struct _digit : token_base<_digit<Base>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            if (!Base::template match<typename Reader::encoding>(reader.peek()))
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), Base::name());
            context.on(_ev::error{}, err);
        }
    };
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
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;
        bool                      forbidden_leading_zero;

        constexpr explicit tp(const Reader& reader)
        : end(reader.position()), forbidden_leading_zero(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            // Check for a zero that is followed by a digit or separator.
            if (reader.peek() == lexy::_char_to_int_type<typename Reader::encoding>('0'))
            {
                reader.bump();
                end = reader.position();

                if (lexy::try_match_token(digit<Base>, reader)
                    || lexy::try_match_token(Sep{}, reader))
                {
                    forbidden_leading_zero = true;
                    return false;
                }

                // Just zero.
                return true;
            }
            // Need at least one digit.
            else if (!lexy::try_match_token(digit<Base>, reader))
            {
                forbidden_leading_zero = false;
                return false;
            }

            // Might have following digits.
            while (true)
            {
                if (lexy::try_match_token(Sep{}, reader))
                {
                    // Need a digit after a separator.
                    if (!lexy::try_match_token(digit<Base>, reader))
                    {
                        forbidden_leading_zero = false;
                        return false;
                    }
                }
                else if (!lexy::try_match_token(digit<Base>, reader))
                {
                    // If we're not having a digit, we're done.
                    break;
                }
            }

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (forbidden_leading_zero)
            {
                auto err
                    = lexy::error<Reader, lexy::forbidden_leading_zero>(reader.position(), end);
                context.on(_ev::error{}, err);
            }
            else
            {
                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                          Base::name());
                context.on(_ev::error{}, err);
            }
        }
    };
};

template <typename Base, typename Sep>
struct _digits_s : token_base<_digits_s<Base, Sep>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need at least one digit.
            if (!lexy::try_match_token(digit<Base>, reader))
                return false;

            // Might have following digits.
            while (true)
            {
                if (lexy::try_match_token(Sep{}, reader))
                {
                    // Need a digit after a separator.
                    if (!lexy::try_match_token(digit<Base>, reader))
                        return false;
                }
                else if (!lexy::try_match_token(digit<Base>, reader))
                {
                    // If we're not having a digit, we're done.
                    break;
                }
            }

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), Base::name());
            context.on(_ev::error{}, err);
        }
    };

    constexpr auto no_leading_zero() const
    {
        return _digits_st<Base, Sep>{};
    }
};

template <typename Base>
struct _digits_t : token_base<_digits_t<Base>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;
        bool                      forbidden_leading_zero;

        constexpr explicit tp(const Reader& reader)
        : end(reader.position()), forbidden_leading_zero(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            // Check for a zero that is followed by a digit.
            if (reader.peek() == lexy::_char_to_int_type<typename Reader::encoding>('0'))
            {
                reader.bump();
                end = reader.position();

                if (lexy::try_match_token(digit<Base>, reader))
                {
                    forbidden_leading_zero = true;
                    return false;
                }

                // Just zero.
                return true;
            }

            // Need at least one digit.
            if (!lexy::try_match_token(digit<Base>, reader))
            {
                forbidden_leading_zero = false;
                return false;
            }

            // Might have more than one digit afterwards.
            while (lexy::try_match_token(digit<Base>, reader))
            {}

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (forbidden_leading_zero)
            {
                auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(reader.position(),
                                                                             this->end);
                context.on(_ev::error{}, err);
            }
            else
            {
                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                          Base::name());
                context.on(_ev::error{}, err);
            }
        }
    };

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
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need at least one digit.
            if (!lexy::try_match_token(digit<Base>, reader))
                return false;

            // Might have more than one digit afterwards.
            while (lexy::try_match_token(digit<Base>, reader))
            {}

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), Base::name());
            context.on(_ev::error{}, err);
        }
    };

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
    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<N - 1>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the Base one time.
            if (!lexy::try_match_token(digit<Base>, reader))
                return false;

            // Match each other digit after a separator.
            auto success = (((void)Idx, lexy::try_match_token(Sep{}, reader),
                             lexy::try_match_token(digit<Base>, reader))
                            && ...);
            end          = reader.position();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), Base::name());
            context.on(_ev::error{}, err);
        }
    };
};

template <std::size_t N, typename Base>
struct _ndigits : token_base<_ndigits<N, Base>>
{
    static_assert(N > 1);

    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<N>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the Base N times.
            auto success = (((void)Idx, lexy::try_match_token(digit<Base>, reader)) && ...);
            end          = reader.position();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), Base::name());
            context.on(_ev::error{}, err);
        }
    };

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

