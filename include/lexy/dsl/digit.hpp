// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct binary
{
    static constexpr int radix = 2;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.binary";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c == Encoding::to_int_type('0') || c == Encoding::to_int_type('1');
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC int value(IntType c)
    {
        return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('0'));
    }
};

struct octal
{
    static constexpr int radix = 8;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.octal";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c >= Encoding::to_int_type('0') && c <= Encoding::to_int_type('7');
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC int value(IntType c)
    {
        return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('0'));
    }
};

struct decimal
{
    static constexpr int radix = 10;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.decimal";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return c >= Encoding::to_int_type('0') && c <= Encoding::to_int_type('9');
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC int value(IntType c)
    {
        return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('0'));
    }
};

struct hex_lower
{
    static constexpr int radix = 16;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.hex-lower";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return (c >= Encoding::to_int_type('0') && c <= Encoding::to_int_type('9'))
               || (c >= Encoding::to_int_type('a') && c <= Encoding::to_int_type('f'));
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC int value(IntType c)
    {
        if (c <= Encoding::to_int_type('9'))
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('0'));
        else
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('a')) + 10;
    }
};

struct hex_upper
{
    static constexpr int radix = 16;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.hex-upper";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return (c >= Encoding::to_int_type('0') && c <= Encoding::to_int_type('9'))
               || (c >= Encoding::to_int_type('A') && c <= Encoding::to_int_type('F'));
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC int value(IntType c)
    {
        if (c <= Encoding::to_int_type('9'))
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('0'));
        else
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('A')) + 10;
    }
};

struct hex
{
    static constexpr int radix = 16;

    static LEXY_CONSTEVAL auto name()
    {
        return "digit.hex";
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC bool match(IntType c)
    {
        return (c >= Encoding::to_int_type('0') && c <= Encoding::to_int_type('9'))
               || (c >= Encoding::to_int_type('A') && c <= Encoding::to_int_type('F'))
               || (c >= Encoding::to_int_type('a') && c <= Encoding::to_int_type('f'));
    }

    template <typename Encoding, typename IntType>
    LEXY_DSL_FUNC int value(IntType c)
    {
        if (c <= Encoding::to_int_type('9'))
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('0'));
        else if (c <= Encoding::to_int_type('F'))
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('A')) + 10;
        else
            return static_cast<int>(c) - static_cast<int>(Encoding::to_int_type('a')) + 10;
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename Base>
struct _zero : atom_base<_zero<Base>>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input)
    {
        if (!Base::template match<typename Input::encoding>(input.peek()))
            return false;
        else if (Base::template value<typename Input::encoding>(input.peek()) != 0)
            return false;

        input.bump();
        return true;
    }

    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input&, typename Input::iterator pos)
    {
        return lexy::expected_char_class::error<Input>(pos, "digit.zero");
    }
};

template <typename Base>
struct _nzero : atom_base<_nzero<Base>>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input)
    {
        if (!Base::template match<typename Input::encoding>(input.peek()))
            return false;
        else if (Base::template value<typename Input::encoding>(input.peek()) == 0)
            return false;

        input.bump();
        return true;
    }

    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input& input, typename Input::iterator pos)
    {
        if (!Base::template match<typename Input::encoding>(input.peek()))
            return lexy::expected_char_class::error<Input>(pos, Base::name());
        else
            return lexy::expected_char_class::error<Input>(pos, "digit.non-zero");
    }
};

template <typename Base>
struct _digit : atom_base<_digit<Base>>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input)
    {
        if (!Base::template match<typename Input::encoding>(input.peek()))
            return false;

        input.bump();
        return true;
    }

    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input&, typename Input::iterator pos)
    {
        return lexy::expected_char_class::error<Input>(pos, Base::name());
    }

    //=== dsl ===//
    LEXY_CONSTEVAL auto zero() const
    {
        return _zero<Base>{};
    }

    LEXY_CONSTEVAL auto non_zero() const
    {
        return _nzero<Base>{};
    }
};

/// Matches a single digit.
template <typename Base>
constexpr auto digit = _digit<Base>{};
} // namespace lexyd

#endif // LEXY_DSL_DIGIT_HPP_INCLUDED

