// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENCODING_HPP_INCLUDED
#define LEXY_ENCODING_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>

//=== code_point ===//
namespace lexy
{
/// A unicode code point.
class code_point
{
public:
    constexpr explicit code_point(std::uint_least32_t value) noexcept : _value(value) {}

    constexpr auto value() const noexcept
    {
        return _value;
    }

    //=== classification ===//
    constexpr bool is_valid() const noexcept
    {
        if (_value > 0x10'FFFF)
            return false; // Out of range.
        else if (0xD800 <= _value && _value <= 0xDFFF)
            return false; // UTF-16 surrogate.
        else
            return true;
    }

    constexpr bool is_ascii() const noexcept
    {
        return _value <= 0x7F;
    }
    constexpr bool is_bmp() const noexcept
    {
        return _value <= 0xFFFF;
    }

private:
    std::uint_least32_t _value;
};
} // namespace lexy

//=== encoding ===//
namespace lexy
{
/// The endianness used by an encoding.
enum class encoding_endianness
{
    /// Little endian.
    little,
    /// Big endian.
    big,
    /// Checks for a BOM and uses its endianness.
    /// If there is no BOM, assumes big endian.
    bom,
};

/// An encoding where the input is some 8bit encoding (ASCII, UTF-8, extended ASCII etc.).
struct default_encoding
{
    using char_type = char;
    using int_type  = int;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        return -1;
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        if constexpr (std::is_unsigned_v<char_type>)
            // We can just convert it to int directly.
            return static_cast<int_type>(c);
        else
        {
            // We first need to prevent negative values, by making it unsigned.
            auto value = static_cast<unsigned char>(c);
            return static_cast<int_type>(value);
        }
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_ascii());
        LEXY_PRECONDITION(size >= 1);

        *buffer = static_cast<char_type>(cp.value());
        return 1;
    }
};

// An encoding where the input is assumed to be valid ASCII.
struct ascii_encoding
{
    using char_type = char;
    using int_type  = char;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        if constexpr (std::is_signed_v<char_type>)
            return static_cast<int_type>(-1);
        else
            return static_cast<int_type>(0xFF);
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return static_cast<int_type>(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_ascii());
        LEXY_PRECONDITION(size >= 1);

        *buffer = static_cast<char_type>(cp.value());
        return 1;
    }
};

/// An encoding where the input is assumed to be valid UTF-8.
struct utf8_encoding
{
    using char_type = LEXY_CHAR8_T;
    using int_type  = LEXY_CHAR8_T;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        // 0xFF is not part of valid UTF-8.
        return static_cast<int_type>(0xFF);
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return static_cast<int_type>(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());

        // Taken from http://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html.
        if (cp.is_ascii())
        {
            LEXY_PRECONDITION(size >= 1);

            *buffer = static_cast<char_type>(cp.value());
            return 1;
        }
        else if (cp.value() <= 0x07'FF)
        {
            LEXY_PRECONDITION(size >= 2);

            auto first  = (cp.value() >> 6) & 0x1F;
            auto second = (cp.value() >> 0) & 0x3F;

            buffer[0] = 0xC0 | static_cast<char_type>(first);
            buffer[1] = 0x80 | static_cast<char_type>(second);
            return 2;
        }
        else if (cp.value() <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 3);

            auto first  = (cp.value() >> 12) & 0x0F;
            auto second = (cp.value() >> 6) & 0x3F;
            auto third  = (cp.value() >> 0) & 0x3F;

            buffer[0] = 0xE0 | static_cast<char_type>(first);
            buffer[1] = 0x80 | static_cast<char_type>(second);
            buffer[2] = 0x80 | static_cast<char_type>(third);
            return 3;
        }
        else
        {
            LEXY_PRECONDITION(size >= 4);

            auto first  = (cp.value() >> 18) & 0x07;
            auto second = (cp.value() >> 12) & 0x3F;
            auto third  = (cp.value() >> 6) & 0x3F;
            auto fourth = (cp.value() >> 0) & 0x3F;

            buffer[0] = 0xF0 | static_cast<char_type>(first);
            buffer[1] = 0x80 | static_cast<char_type>(second);
            buffer[2] = 0x80 | static_cast<char_type>(third);
            buffer[3] = 0x80 | static_cast<char_type>(fourth);
            return 4;
        }
    }
};
template <>
constexpr bool utf8_encoding::is_secondary_char_type<char> = true;

/// An encoding where the input is assumed to be valid UTF-16.
struct utf16_encoding
{
    using char_type = char16_t;
    using int_type  = std::int_least32_t;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        // Every value of char16_t is valid UTF16.
        return int_type(-1);
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return static_cast<int_type>(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());

        if (cp.is_bmp())
        {
            LEXY_PRECONDITION(size >= 1);

            *buffer = static_cast<char_type>(cp.value());
            return 1;
        }
        else
        {
            // Algorithm implemented from
            // https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF.
            LEXY_PRECONDITION(size >= 2);

            auto u_prime       = cp.value() - 0x1'0000;
            auto high_ten_bits = u_prime >> 10;
            auto low_ten_bits  = u_prime & 0b11'1111'1111;

            buffer[0] = static_cast<char_type>(0xD800 + high_ten_bits);
            buffer[1] = static_cast<char_type>(0xDC00 + low_ten_bits);
            return 2;
        }
    }
};
template <>
constexpr bool utf16_encoding::is_secondary_char_type<wchar_t> = sizeof(wchar_t)
                                                                 == sizeof(char16_t);

/// An encoding where the input is assumed to be valid UTF-32.
struct utf32_encoding
{
    using char_type = char32_t;
    using int_type  = char32_t;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        // The highest unicode code point is U+10'FFFF, so this is never a valid code point.
        return int_type(0xFFFF'FFFF);
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return c;
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());
        LEXY_PRECONDITION(size >= 1);

        *buffer = static_cast<char_type>(cp.value());
        return 1;
    }
};
template <>
constexpr bool utf32_encoding::is_secondary_char_type<wchar_t> = sizeof(wchar_t)
                                                                 == sizeof(char32_t);

/// An encoding where the input is just raw bytes, not characters.
struct raw_encoding
{
    using char_type = unsigned char;
    using int_type  = int;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        return -1;
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return static_cast<int_type>(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
        = delete;
};
template <>
constexpr bool raw_encoding::is_secondary_char_type<char> = true;
} // namespace lexy

//=== deduce_encoding ===//
namespace lexy
{
template <typename CharT>
struct _deduce_encoding;
template <typename CharT>
using deduce_encoding = typename _deduce_encoding<CharT>::type;

template <>
struct _deduce_encoding<char>
{
    using type = default_encoding; // Don't know the exact encoding.
};

#if LEXY_HAS_CHAR8_T
template <>
struct _deduce_encoding<LEXY_CHAR8_T>
{
    using type = utf8_encoding;
};
#endif
template <>
struct _deduce_encoding<char16_t>
{
    using type = utf16_encoding;
};
template <>
struct _deduce_encoding<char32_t>
{
    using type = utf32_encoding;
};

template <>
struct _deduce_encoding<unsigned char>
{
    using type = raw_encoding;
};
} // namespace lexy

//=== impls ===//
namespace lexy
{
template <typename Encoding, typename CharT>
using _require_secondary_char_type
    = std::enable_if_t<Encoding::template is_secondary_char_type<CharT>>;
} // namespace lexy

#endif // LEXY_ENCODING_HPP_INCLUDED
