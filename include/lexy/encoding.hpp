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
    constexpr code_point() noexcept : _value(0xFFFF'FFFF) {}
    constexpr explicit code_point(char32_t value) noexcept : _value(value) {}

    constexpr auto value() const noexcept
    {
        return _value;
    }

    //=== classification ===//
    constexpr bool is_valid() const noexcept
    {
        return _value <= 0x10'FFFF;
    }
    constexpr bool is_surrogate() const noexcept
    {
        return 0xD800 <= _value && _value <= 0xDFFF;
    }
    constexpr bool is_scalar() const noexcept
    {
        return is_valid() && !is_surrogate();
    }

    constexpr bool is_ascii() const noexcept
    {
        return _value <= 0x7F;
    }
    constexpr bool is_bmp() const noexcept
    {
        return _value <= 0xFFFF;
    }

    friend constexpr bool operator==(code_point lhs, code_point rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(code_point lhs, code_point rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

private:
    char32_t _value;
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
        = delete;
    class code_point_decoder;
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
            return int_type(-1);
        else
            return int_type(0xFFu);
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return int_type(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_ascii());
        LEXY_PRECONDITION(size >= 1);

        *buffer = char_type(cp.value());
        return 1;
    }

    class code_point_decoder
    {
    public:
        int init(int_type c)
        {
            if (c == eof())
                return -1;

            _result = char32_t(c);
            return 0;
        }

        bool next(int_type)
        {
            return false;
        }

        auto finish() &&
        {
            auto cp = code_point(_result);
            if (!cp.is_ascii())
                return code_point();
            return cp;
        }

    private:
        char32_t _result = {};
    };
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
        return int_type(0xFF);
    }

    static LEXY_CONSTEVAL int_type to_int_type(char_type c)
    {
        return int_type(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());

        // Taken from http://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html.
        if (cp.is_ascii())
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char_type(cp.value());
            return 1;
        }
        else if (cp.value() <= 0x07'FF)
        {
            LEXY_PRECONDITION(size >= 2);

            auto first  = (cp.value() >> 6) & 0x1F;
            auto second = (cp.value() >> 0) & 0x3F;

            buffer[0] = char_type(0xC0 | first);
            buffer[1] = char_type(0x80 | second);
            return 2;
        }
        else if (cp.value() <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 3);

            auto first  = (cp.value() >> 12) & 0x0F;
            auto second = (cp.value() >> 6) & 0x3F;
            auto third  = (cp.value() >> 0) & 0x3F;

            buffer[0] = char_type(0xE0 | first);
            buffer[1] = char_type(0x80 | second);
            buffer[2] = char_type(0x80 | third);
            return 3;
        }
        else
        {
            LEXY_PRECONDITION(size >= 4);

            auto first  = (cp.value() >> 18) & 0x07;
            auto second = (cp.value() >> 12) & 0x3F;
            auto third  = (cp.value() >> 6) & 0x3F;
            auto fourth = (cp.value() >> 0) & 0x3F;

            buffer[0] = char_type(0xF0 | first);
            buffer[1] = char_type(0x80 | second);
            buffer[2] = char_type(0x80 | third);
            buffer[3] = char_type(0x80 | fourth);
            return 4;
        }
    }

    class code_point_decoder
    {
        static constexpr auto payload_lead1 = 0b0111'1111;
        static constexpr auto payload_lead2 = 0b0001'1111;
        static constexpr auto payload_lead3 = 0b0000'1111;
        static constexpr auto payload_lead4 = 0b0000'0111;
        static constexpr auto payload_cont  = 0b0011'1111;

        static constexpr auto pattern_lead1 = 0b0 << 7;
        static constexpr auto pattern_lead2 = 0b110 << 5;
        static constexpr auto pattern_lead3 = 0b1110 << 4;
        static constexpr auto pattern_lead4 = 0b11110 << 3;
        static constexpr auto pattern_cont  = 0b10 << 6;

    public:
        int init(int_type c)
        {
            if ((c & ~payload_lead1) == pattern_lead1)
            {
                _result = char32_t(c & payload_lead1);
                return 0;
            }
            else if (c == 0xC0 || c == 0xC1)
            {
                // These leading characters can only used for overlong ASCII.
                return -1;
            }
            else if ((c & ~payload_lead2) == pattern_lead2)
            {
                _result = char32_t(c & payload_lead2);
                return 1;
            }
            else if ((c & ~payload_lead3) == pattern_lead3)
            {
                _result = char32_t(c & payload_lead3);
                if (c == 0xE0)
                    _min_cont_value = 0xA0;
                return 2;
            }
            else if ((c & ~payload_lead4) == pattern_lead4)
            {
                _result = char32_t(c & payload_lead4);
                if (c == 0xF0)
                    _min_cont_value = 0x90;
                return 3;
            }
            else
            {
                return -1;
            }
        }

        bool next(int_type c)
        {
            if ((c & ~payload_cont) != pattern_cont)
                return false; // Not a continuation byte.
            else if (c < _min_cont_value)
                return false; // Overlong sequence.

            _result <<= 6;
            _result |= char32_t(c & payload_cont);

            // We're only having overlong sequences in the second byte, so overwrite.
            _min_cont_value = 0x80;
            return true;
        }

        auto finish() &&
        {
            auto cp = code_point(_result);
            if (cp.is_surrogate())
                // Surrogates are not allowed.
                return code_point();
            return cp;
        }

    private:
        char32_t _result = {};
        // The minimal continuation value to prevent overlong sequences.
        int_type _min_cont_value = 0x80;
    };
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
        return int_type(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
    {
        LEXY_PRECONDITION(cp.is_valid());

        if (cp.is_bmp())
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char_type(cp.value());
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

            buffer[0] = char_type(0xD800 + high_ten_bits);
            buffer[1] = char_type(0xDC00 + low_ten_bits);
            return 2;
        }
    }

    class code_point_decoder
    {
        static constexpr auto payload_lead1 = 0b0000'0011'1111'1111;
        static constexpr auto payload_lead2 = payload_lead1;

        static constexpr auto pattern_lead1 = 0b110110 << 10;
        static constexpr auto pattern_lead2 = 0b110111 << 10;

    public:
        int init(int_type c)
        {
            // We need to handle EOF separately and then convert it to a uint16_t.
            if (c == eof())
                return -1;
            auto value = char_type(c);

            if ((value & ~payload_lead1) == pattern_lead1)
            {
                _result = char32_t(value & payload_lead1);
                return 1;
            }
            else if ((value & ~payload_lead2) == pattern_lead2)
            {
                return -1;
            }
            else
            {
                _result = char32_t(value);
                return 0;
            }
        }

        bool next(int_type c)
        {
            if (c == eof())
                return false;
            auto value = char_type(c);

            if ((value & ~payload_lead2) != pattern_lead2)
                return false;

            _result <<= 10;
            _result |= char32_t(value & payload_lead2);
            _result |= 0x10000;
            return true;
        }

        auto finish() &&
        {
            auto cp = code_point(_result);
            LEXY_PRECONDITION(!cp.is_surrogate());
            return cp;
        }

    private:
        char32_t _result = {};
    };
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

        *buffer = char_type(cp.value());
        return 1;
    }

    class code_point_decoder
    {
    public:
        int init(int_type c)
        {
            // No need to handle EOF, the code point validation will take care of that one.
            _result = c;
            return 0;
        }

        bool next(int_type)
        {
            return false;
        }

        auto finish() &&
        {
            auto cp = code_point(_result);
            if (cp.is_surrogate())
                // Surrogates are not allowed.
                return code_point();
            return cp;
        }

    private:
        char32_t _result = {};
    };
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
        return int_type(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
        = delete;

    class code_point_decoder;
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
#if defined(LEXY_ENCODING_OF_CHAR)
    using type = LEXY_ENCODING_OF_CHAR;
    static_assert(std::is_same_v<type, default_encoding>      //
                      || std::is_same_v<type, ascii_encoding> //
                      || std::is_same_v<type, utf8_encoding>,
                  "invalid value for LEXY_ENCODING_OF_CHAR");
#else
    using type = default_encoding; // Don't know the exact encoding.
#endif
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
