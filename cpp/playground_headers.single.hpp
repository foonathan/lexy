// Copyright (C) 2020-2021-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_HPP_INCLUDED
#define LEXY_DSL_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

// Copyright (C) 2020-2021 Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_DETECT_HPP_INCLUDED
#define LEXY_DETAIL_DETECT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_CONFIG_HPP_INCLUDED
#define LEXY_DETAIL_CONFIG_HPP_INCLUDED

#include <cstddef>
#include <type_traits>

#if defined(LEXY_USER_CONFIG_HEADER)
#    include LEXY_USER_CONFIG_HEADER
#elif defined(__has_include)
#    if __has_include(<lexy_user_config.hpp>)
#        include <lexy_user_config.hpp>
#    elif __has_include("lexy_user_config.hpp")
#        include "lexy_user_config.hpp"
#    endif
#endif

//=== move/fwd/declval/swap ===//
namespace lexy::_detail
{
template <typename T>
using add_rvalue_ref = T&&;

template <typename... T>
constexpr bool error = false;
} // namespace lexy::_detail

#define LEXY_MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define LEXY_FWD(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define LEXY_DECLVAL(...)                                                                          \
    reinterpret_cast<::lexy::_detail::add_rvalue_ref<__VA_ARGS__>>(*reinterpret_cast<char*>(1024))

namespace lexy::_detail
{
template <typename T>
constexpr void swap(T& lhs, T& rhs)
{
    T tmp = LEXY_MOV(lhs);
    lhs   = LEXY_MOV(rhs);
    rhs   = LEXY_MOV(tmp);
}
} // namespace lexy::_detail

//=== NTTP ===//
#ifndef LEXY_HAS_NTTP
// See https://github.com/foonathan/lexy/issues/15.
#    if __cpp_nontype_template_parameter_class >= 201806 || __cpp_nontype_template_args >= 201911
#        define LEXY_HAS_NTTP 1
#    else
#        define LEXY_HAS_NTTP 0
#    endif
#endif

//=== consteval ===//
#ifndef LEXY_HAS_CONSTEVAL
#    if __cpp_consteval
#        define LEXY_HAS_CONSTEVAL 1
#    else
#        define LEXY_HAS_CONSTEVAL 0
#    endif
#endif

#if LEXY_HAS_CONSTEVAL
#    define LEXY_CONSTEVAL consteval
#else
#    define LEXY_CONSTEVAL constexpr
#endif

//=== char8_t ===//
#ifndef LEXY_HAS_CHAR8_T
#    if __cpp_char8_t
#        define LEXY_HAS_CHAR8_T 1
#    else
#        define LEXY_HAS_CHAR8_T 0
#    endif
#endif

#if LEXY_HAS_CHAR8_T

#    define LEXY_CHAR8_T char8_t
#    define LEXY_CHAR8_STR(Str) u8##Str

#else

namespace lexy
{
using _char8_t = unsigned char;

template <typename String>
struct _char8_str
{
    struct str
    {
        _char8_t data[String::get().size() + 1];

        constexpr str() : data{}
        {
            auto i = 0;
            for (auto c : String::get())
                data[i++] = _char8_t(c);
        }
    };

    static constexpr auto get = str{};
};
} // namespace lexy

#    define LEXY_CHAR8_T ::lexy::_char8_t
#    define LEXY_CHAR8_STR(Str) (::lexy::_char8_str<LEXY_NTTP_STRING(u8##Str)>::get.data)

#endif

//=== endianness ===//
#ifndef LEXY_IS_LITTLE_ENDIAN
#    if defined(__BYTE_ORDER__)
#        if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#            define LEXY_IS_LITTLE_ENDIAN 1
#        elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#            define LEXY_IS_LITTLE_ENDIAN 0
#        else
#            error "unsupported byte order"
#        endif
#    elif defined(_MSC_VER)
#        define LEXY_IS_LITTLE_ENDIAN 1
#    else
#        error "unknown endianness"
#    endif
#endif

//=== force inline ===//
#ifndef LEXY_FORCE_INLINE
#    if defined(__has_cpp_attribute)
#        if __has_cpp_attribute(gnu::always_inline)
#            define LEXY_FORCE_INLINE [[gnu::always_inline]]
#        endif
#    endif
#
#    ifndef LEXY_FORCE_INLINE
#        define LEXY_FORCE_INLINE inline
#    endif
#endif

//=== empty_member ===//
#ifndef LEXY_EMPTY_MEMBER

#    if defined(__has_cpp_attribute)
#        if __has_cpp_attribute(no_unique_address)
#            define LEXY_HAS_EMPTY_MEMBER 1
#        endif
#    endif
#    ifndef LEXY_HAS_EMPTY_MEMBER
#        define LEXY_HAS_EMPTY_MEMBER 0
#    endif

#    if LEXY_HAS_EMPTY_MEMBER
#        define LEXY_EMPTY_MEMBER [[no_unique_address]]
#    else
#        define LEXY_EMPTY_MEMBER
#    endif

#endif

#endif // LEXY_DETAIL_CONFIG_HPP_INCLUDED


namespace lexy::_detail
{
template <typename... Args>
using void_t = void;

template <template <typename...> typename Op, typename Void, typename... Args>
struct _detector : std::false_type
{};
template <template <typename...> typename Op, typename... Args>
struct _detector<Op, void_t<Op<Args...>>, Args...> : std::true_type
{};

template <template <typename...> typename Op, typename... Args>
constexpr bool is_detected = _detector<Op, void, Args...>::value;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_DETECT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ASSERT_HPP_INCLUDED
#define LEXY_DETAIL_ASSERT_HPP_INCLUDED



#ifndef LEXY_ENABLE_ASSERT

// By default, enable assertions if NDEBUG is not defined.

#    if NDEBUG
#        define LEXY_ENABLE_ASSERT 0
#    else
#        define LEXY_ENABLE_ASSERT 1
#    endif

#endif

#if LEXY_ENABLE_ASSERT

// We want assertions: use assert() if that's available, otherwise abort.
// We don't use assert() directly as that's not constexpr.

#    if NDEBUG

#        include <cstdlib>
#        define LEXY_PRECONDITION(Expr) ((Expr) ? void(0) : std::abort())
#        define LEXY_ASSERT(Expr, Msg) ((Expr) ? void(0) : std::abort())

#    else

#        include <cassert>

#        define LEXY_PRECONDITION(Expr) ((Expr) ? void(0) : assert(Expr))
#        define LEXY_ASSERT(Expr, Msg) ((Expr) ? void(0) : assert((Expr) && Msg))

#    endif

#else

// We don't want assertions.

#    define LEXY_PRECONDITION(Expr) static_cast<void>(sizeof(Expr))
#    define LEXY_ASSERT(Expr, Msg) static_cast<void>(sizeof(Expr))

#endif

#endif // LEXY_DETAIL_ASSERT_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED
#define LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED




namespace lexy::_detail
{
template <typename T>
struct _lazy_init_storage_trivial
{
    bool _init;
    union
    {
        char _empty;
        T    _value;
    };

    constexpr _lazy_init_storage_trivial() noexcept : _init(false), _empty() {}

    template <typename... Args>
    constexpr _lazy_init_storage_trivial(int, Args&&... args)
    : _init(true), _value(LEXY_FWD(args)...)
    {}
};

template <typename T>
struct _lazy_init_storage_non_trivial
{
    bool _init;
    union
    {
        char _empty;
        T    _value;
    };

    constexpr _lazy_init_storage_non_trivial() noexcept : _init(false), _empty() {}

    template <typename... Args>
    constexpr _lazy_init_storage_non_trivial(int, Args&&... args)
    : _init(true), _value(LEXY_FWD(args)...)
    {}

    ~_lazy_init_storage_non_trivial() noexcept
    {
        if (_init)
            _value.~T();
    }

    _lazy_init_storage_non_trivial(const _lazy_init_storage_non_trivial& other) noexcept
    : _init(other._init), _empty()
    {
        if (_init)
            ::new (static_cast<void*>(&_value)) T(other._value);
    }
    _lazy_init_storage_non_trivial(_lazy_init_storage_non_trivial&& other) noexcept
    : _init(other._init), _empty()
    {
        if (_init)
            ::new (static_cast<void*>(&_value)) T(LEXY_MOV(other._value));
    }

    _lazy_init_storage_non_trivial& operator=(const _lazy_init_storage_non_trivial& other) noexcept
    {
        if (_init && other._init)
            _value = other._value;
        else if (_init && !other._init)
        {
            _value.~T();
            _init = false;
        }
        else if (!_init && other._init)
        {
            ::new (static_cast<void*>(&_value)) T(other._value);
            _init = true;
        }
        else
        {
            // Both not initialized, nothing to do.
        }

        return *this;
    }
    _lazy_init_storage_non_trivial& operator=(_lazy_init_storage_non_trivial&& other) noexcept
    {
        if (_init && other._init)
            _value = LEXY_MOV(other._value);
        else if (_init && !other._init)
        {
            _value.~T();
            _init = false;
        }
        else if (!_init && other._init)
        {
            ::new (static_cast<void*>(&_value)) T(LEXY_MOV(other._value));
            _init = true;
        }
        else
        {
            // Both not initialized, nothing to do.
        }

        return *this;
    }
};

// https://github.com/foonathan/lexy/pull/17
template <typename T>
constexpr auto _lazy_init_trivial = [] {
    return std::is_trivially_destructible_v<T>                                                    //
           && std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> //
           && std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>;
}();
template <typename T>
using _lazy_init_storage = std::conditional_t<_lazy_init_trivial<T>, _lazy_init_storage_trivial<T>,
                                              _lazy_init_storage_non_trivial<T>>;

template <typename T>
class lazy_init : _lazy_init_storage<T>
{
public:
    using value_type = T;

    constexpr lazy_init() noexcept = default;

    template <typename... Args>
    constexpr void emplace(Args&&... args)
    {
        LEXY_PRECONDITION(!*this);

        *this = lazy_init(0, LEXY_FWD(args)...);
    }

    constexpr explicit operator bool() const noexcept
    {
        return this->_init;
    }

    constexpr T& operator*() & noexcept
    {
        LEXY_PRECONDITION(*this);
        return this->_value;
    }
    constexpr const T& operator*() const& noexcept
    {
        LEXY_PRECONDITION(*this);
        return this->_value;
    }
    constexpr T&& operator*() && noexcept
    {
        LEXY_PRECONDITION(*this);
        return LEXY_MOV(this->_value);
    }
    constexpr const T&& operator*() const&& noexcept
    {
        LEXY_PRECONDITION(*this);
        return LEXY_MOV(this->_value);
    }

    constexpr T* operator->() noexcept
    {
        LEXY_PRECONDITION(*this);
        return &this->_value;
    }
    constexpr const T* operator->() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return &this->_value;
    }

private:
    template <typename... Args>
    constexpr explicit lazy_init(int, Args&&... args) noexcept
    : _lazy_init_storage<T>(0, LEXY_FWD(args)...)
    {}
};
template <>
class lazy_init<void>
{
public:
    using value_type = void;

    constexpr lazy_init() noexcept : _init(false) {}

    constexpr void emplace()
    {
        LEXY_PRECONDITION(!*this);
        _init = true;
    }

    constexpr explicit operator bool() const noexcept
    {
        return _init;
    }

private:
    bool _init;
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_BASE_HPP_INCLUDED
#define LEXY_ENGINE_BASE_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENCODING_HPP_INCLUDED
#define LEXY_ENCODING_HPP_INCLUDED

#include <cstdint>



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

    static constexpr int_type to_int_type(char_type c)
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

    static constexpr int_type to_int_type(char_type c)
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

    static constexpr int_type to_int_type(char_type c)
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

    static constexpr int_type to_int_type(char_type c)
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

    static constexpr int_type to_int_type(char_type c)
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
};
template <>
constexpr bool utf32_encoding::is_secondary_char_type<wchar_t> = sizeof(wchar_t)
                                                                 == sizeof(char32_t);

/// An encoding where the input is just raw bytes, not characters.
struct byte_encoding
{
    using char_type = unsigned char;
    using int_type  = int;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type = false;

    static LEXY_CONSTEVAL int_type eof()
    {
        return -1;
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }

    static constexpr std::size_t encode_code_point(code_point cp, char_type* buffer,
                                                   std::size_t size)
        = delete;
};
template <>
constexpr bool byte_encoding::is_secondary_char_type<char> = true;
template <>
constexpr bool byte_encoding::is_secondary_char_type<std::byte> = true;
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
    using type = byte_encoding;
};
template <>
struct _deduce_encoding<std::byte>
{
    using type = byte_encoding;
};
} // namespace lexy

//=== impls ===//
namespace lexy
{
template <typename Encoding, typename CharT>
using _require_secondary_char_type
    = std::enable_if_t<Encoding::template is_secondary_char_type<CharT>>;

template <typename CharT>
constexpr bool _is_ascii(CharT c)
{
    if constexpr (std::is_signed_v<CharT>)
        return 0 <= c && c <= 0x7F;
    else
        return c <= 0x7F;
}

template <typename Encoding, typename CharT>
LEXY_CONSTEVAL auto _char_to_int_type(CharT c)
{
    using encoding_char_type = typename Encoding::char_type;

    if constexpr (std::is_same_v<CharT, encoding_char_type>)
        return Encoding::to_int_type(c);
    else if constexpr (std::is_same_v<CharT, unsigned char> && sizeof(encoding_char_type) == 1)
    {
        // We allow using unsigned char to express raw bytes, if we have a byte-only input.
        // This enables the BOM rule.
        return Encoding::to_int_type(static_cast<encoding_char_type>(c));
    }
#if !LEXY_HAS_CHAR8_T
    else if constexpr (std::is_same_v<CharT, char> && std::is_same_v<Encoding, lexy::utf8_encoding>)
    {
        // If we don't have char8_t, `LEXY_LIT(u8"ä")` would have the type char, not LEXY_CHAR8_T
        // (which is unsigned char). So we disable checking in that case, to allow such usage. Note
        // that this prevents catching `LEXY_LIT("ä")`, but there is nothing we can do.
        return Encoding::to_int_type(static_cast<LEXY_CHAR8_T>(c));
    }
#endif
    else
    {
        LEXY_ASSERT(_is_ascii(c), "character type of string literal didn't match, "
                                  "so only ASCII characters are supported");
        return Encoding::to_int_type(static_cast<encoding_char_type>(c));
    }
}
} // namespace lexy

#endif // LEXY_ENCODING_HPP_INCLUDED


#if 0
/// Readers are non-owning, cheaply copyable types.
class Reader
{
public:
    /// The encoding the input uses.
    using encoding = XXX_encoding;
    using char_type = typename encoding::char_type;

    /// An iterator of char_type, not int_type.
    using iterator = ForwardIterator;

    /// A reader with the same encoding and iterator that is used for reporting errors/lexemes.
    /// If you're writing an Input, the Input's Reader is always canonical.
    using canonical_reader = Reader;

    /// Checks whether the reader is at EOF.
    bool eof() const;

    /// If the reader is at eof, returns Encoding::eof().
    /// Otherwise, returns Encoding::to_int_type(/* current character */).
    typename Encoding::int_type peek() const;

    /// Advances to the next character in the input.
    void bump();

    /// Returns an iterator to the current character.
    /// The following code must produce a valid range:
    /// ```
    /// auto begin = reader.cur();
    /// reader.bump();
    /// ... // more bumps
    /// auto end = reader.cur();
    /// ```
    iterator cur() const;
};

/// An Input produces a reader.
class Input
{
public:
    /// Returns a reader to the beginning of the input.
    Reader reader() const &;
};
#endif

namespace lexy::_detail
{
template <typename I>
constexpr auto range_size(I begin, I end) -> decltype(std::size_t(end - begin))
{
    return std::size_t(end - begin);
}
template <typename I, typename I2> // always worse match because two different params
constexpr auto range_size(I begin, I2 end)
{
    std::size_t result = 0;
    for (auto cur = begin; cur != end; ++cur)
        ++result;
    return result;
}

template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class range_reader
{
public:
    using encoding         = Encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = Iterator;
    using canonical_reader = range_reader<Encoding, Iterator, Sentinel>;

    constexpr explicit range_reader(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end)
    {}

    constexpr bool eof() const noexcept
    {
        return _cur == _end;
    }

    constexpr auto peek() const noexcept
    {
        if (_cur == _end)
            return encoding::eof();
        else
            return encoding::to_int_type(*_cur);
    }

    constexpr void bump() noexcept
    {
        ++_cur;
    }

    constexpr iterator cur() const noexcept
    {
        return _cur;
    }

    constexpr void _make_eof() noexcept
    {
        static_assert(std::is_same_v<Iterator, Sentinel>);
        _cur = _end;
    }

private:
    Iterator                   _cur;
    LEXY_EMPTY_MEMBER Sentinel _end;
};
} // namespace lexy::_detail

namespace lexy
{
template <typename Input>
using input_reader = decltype(LEXY_DECLVAL(Input).reader());

template <typename Reader, typename CharT>
constexpr bool char_type_compatible_with_reader
    = (std::is_same_v<CharT, typename Reader::char_type>)
      || Reader::encoding::template is_secondary_char_type<CharT>;

template <typename Reader>
constexpr bool is_canonical_reader = std::is_same_v<typename Reader::canonical_reader, Reader>;

/// Creates a reader that only reads until the given end.
template <typename Reader>
constexpr auto partial_reader(Reader reader, typename Reader::iterator end)
{
    struct partial_reader_t
    : _detail::range_reader<typename Reader::encoding, typename Reader::iterator>
    {
        using canonical_reader = Reader;
        using _detail::range_reader<typename Reader::encoding,
                                    typename Reader::iterator>::range_reader;
    };
    return partial_reader_t(reader.cur(), end);
}
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED


#if 0
/// Matches something, i.e. consumes input and returns success or failure.
struct Matcher : engine_matcher_base
{
    /// The error code of the match operation.
    /// A value constructed `error_code` corresponds to success.
    enum class error_code;

    /// Tries to match the input of the reader.
    /// If it works, consumes all matched input and returns `error_code()`.
    /// Otherwise, leaves the reader at the position of the error and returns some other error_code.
    template <typename Reader>
    static error_code match(Reader& reader);

    /// Tries to recover after an error (optional).
    /// If possible, advances the input to the recovered position and returns true.
    /// If not possible, keeps input at the error position and returns false.
    template <typename Reader>
    static bool recover(Reader& reader, error_code ec);
};

/// Parses something, i.e. consumes and input and returns a result or error.
struct Parser : engine_parser_base
{
    /// The error code of the match operation.
    /// A value constructed `error_code` corresponds to success.
    enum class error_code;

    /// Tries to match the input of the reader.
    /// If it works, consumes all matched input and returns the result leaving error code unchanged.
    /// Otherwise, leaves the reader at the position of the error, sets the error and returns some partial result.
    template <typename Reader>
    static auto parse(error_code& ec, Reader& reader);

    /// Same as for Matcher.
    template <typename Reader>
    static bool recover(Reader& reader, error_code ec);
};
#endif

namespace lexy
{
struct engine_matcher_base
{
    template <typename Reader, typename EC>
    static constexpr bool recover(Reader&, EC)
    {
        return false;
    }
};
struct engine_parser_base
{
    template <typename Reader, typename EC>
    static constexpr bool recover(Reader&, EC)
    {
        return false;
    }
};

/// Whether or not the engine is a matcher.
template <typename Engine>
constexpr bool engine_is_matcher = std::is_base_of_v<engine_matcher_base, Engine>;
/// Whether or not the engine is a parser.
template <typename Engine>
constexpr bool engine_is_parser = std::is_base_of_v<engine_parser_base, Engine>;

/// Whether or not the engine can fail on the given input.
template <typename Engine, typename Reader>
constexpr bool engine_can_fail = true;

/// Whether or not the engine can succeed on the given input.
template <typename Engine, typename Reader>
constexpr bool engine_can_succeed = true;
} // namespace lexy

namespace lexy
{
/// Matches the `Matcher` returning a boolean.
/// Input is only consumed if the match was successful.
template <typename Matcher, typename Reader>
constexpr bool engine_try_match(Reader& reader)
{
    if constexpr (engine_can_fail<Matcher, Reader>)
    {
        auto save = reader;
        if (Matcher::match(reader) == typename Matcher::error_code())
            return true;
        else
        {
            reader = LEXY_MOV(save);
            return false;
        }
    }
    else
    {
        return Matcher::match(reader) == typename Matcher::error_code();
    }
}

/// Matches the `Matcher` consuming nothing.
template <typename Matcher, typename Reader>
constexpr bool engine_peek(Reader reader)
{
    return Matcher::match(reader) == typename Matcher::error_code();
}
} // namespace lexy

#endif // LEXY_ENGINE_BASE_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PRODUCTION_HPP_INCLUDED
#define LEXY_PRODUCTION_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED
#define LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED
#define LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED



namespace lexy::_detail
{
template <typename T, T... Indices>
struct integer_sequence
{
    using type = integer_sequence<T, Indices...>;
};
template <std::size_t... Indices>
using index_sequence = integer_sequence<std::size_t, Indices...>;

#if defined(__clang__)
template <std::size_t Size>
using make_index_sequence = __make_integer_seq<integer_sequence, std::size_t, Size>;
#elif defined(__GNUC__) && __GNUC__ >= 8
template <std::size_t Size>
using make_index_sequence = index_sequence<__integer_pack(Size)...>;
#elif defined(_MSC_VER)
template <std::size_t Size>
using make_index_sequence = __make_integer_seq<integer_sequence, std::size_t, Size>;
#else

// Adapted from https://stackoverflow.com/a/32223343.
template <class Sequence1, class Sequence2>
struct concat_seq;
template <std::size_t... I1, std::size_t... I2>
struct concat_seq<index_sequence<I1...>, index_sequence<I2...>>
{
    using type = index_sequence<I1..., (sizeof...(I1) + I2)...>;
};

template <size_t N>
struct _make_index_sequence : concat_seq<typename _make_index_sequence<N / 2>::type,
                                         typename _make_index_sequence<N - N / 2>::type>
{};
template <>
struct _make_index_sequence<0>
{
    using type = index_sequence<>;
};
template <>
struct _make_index_sequence<1>
{
    using type = index_sequence<0>;
};

template <std::size_t Size>
using make_index_sequence = typename _make_index_sequence<Size>::type;

#endif

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_STRING_VIEW_HPP_INCLUDED
#define LEXY_DETAIL_STRING_VIEW_HPP_INCLUDED





namespace lexy::_detail
{
struct null_terminated
{};

template <typename CharT>
class basic_string_view
{
    static constexpr CharT empty_string[] = {CharT()};

public:
    using char_type = CharT;

    //=== constructor ===//
    constexpr basic_string_view() noexcept : _ptr(empty_string), _size(0u), _null_terminated(true)
    {}

    constexpr basic_string_view(const char_type* str) noexcept
    : _ptr(str), _size(0u), _null_terminated(true)
    {
        while (*str++)
            ++_size;
    }

    constexpr basic_string_view(const char_type* ptr, std::size_t size) noexcept
    : _ptr(ptr), _size(size), _null_terminated(false)
    {}
    constexpr basic_string_view(null_terminated, const char_type* ptr, std::size_t size) noexcept
    : _ptr(ptr), _size(size), _null_terminated(true)
    {
        LEXY_PRECONDITION(_ptr[_size] == CharT());
    }

    constexpr basic_string_view(const char_type* begin, const char_type* end) noexcept
    : _ptr(begin), _size(std::size_t(end - begin)), _null_terminated(false)
    {
        LEXY_PRECONDITION(begin <= end);
    }

    //=== access ===//
    using iterator = const char_type*;

    constexpr iterator begin() const noexcept
    {
        return _ptr;
    }
    constexpr iterator end() const noexcept
    {
        return _ptr + _size;
    }

    constexpr bool empty() const noexcept
    {
        return _size == 0u;
    }
    constexpr std::size_t size() const noexcept
    {
        return _size;
    }
    constexpr std::size_t length() const noexcept
    {
        return _size;
    }

    constexpr char_type operator[](std::size_t i) const noexcept
    {
        LEXY_PRECONDITION(i <= _size);
        return _ptr[i];
    }
    constexpr char_type front() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return *_ptr;
    }
    constexpr char_type back() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return _ptr[_size - 1];
    }

    constexpr const char_type* data() const noexcept
    {
        return _ptr;
    }

    constexpr bool is_null_terminated() const noexcept
    {
        return _null_terminated;
    }

    constexpr const char_type* c_str() const noexcept
    {
        LEXY_PRECONDITION(is_null_terminated());
        return _ptr;
    }

    //=== operations ===//
    static constexpr std::size_t npos = std::size_t(-1);

    constexpr void remove_prefix(std::size_t n) noexcept
    {
        LEXY_PRECONDITION(n <= _size);
        _ptr += n;
        _size -= n;
    }
    constexpr void remove_suffix(std::size_t n) noexcept
    {
        LEXY_PRECONDITION(n <= _size);
        _size -= n;
        _null_terminated = false;
    }

    constexpr basic_string_view substr(std::size_t pos, std::size_t length = npos) const noexcept
    {
        LEXY_PRECONDITION(pos < _size);
        if (length >= _size - pos)
        {
            auto result             = basic_string_view(_ptr + pos, end());
            result._null_terminated = _null_terminated;
            return result;
        }
        else
        {
            // Note that we're loosing null-terminated-ness.
            return basic_string_view(_ptr + pos, length);
        }
    }

    constexpr bool starts_with(basic_string_view prefix) const
    {
        return substr(0, prefix.size()) == prefix;
    }

    constexpr std::size_t find(basic_string_view str, std::size_t pos = 0) const noexcept
    {
        for (auto i = pos; i < length(); ++i)
        {
            if (substr(i, str.length()) == str)
                return i;
        }

        return npos;
    }
    constexpr std::size_t find(CharT c, std::size_t pos = 0) const noexcept
    {
        return find(basic_string_view(&c, 1), pos);
    }

    //=== comparison ===//
    friend constexpr bool operator==(basic_string_view<CharT> lhs,
                                     basic_string_view<CharT> rhs) noexcept
    {
        if (lhs.size() != rhs.size())
            return false;

        for (auto a = lhs.begin(), b = rhs.begin(); a != lhs.end(); ++a, ++b)
            if (*a != *b)
                return false;

        return true;
    }

    friend constexpr bool operator!=(basic_string_view<CharT> lhs,
                                     basic_string_view<CharT> rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    const CharT* _ptr;
    std::size_t  _size;
    bool         _null_terminated;
};
using string_view = basic_string_view<char>;
} // namespace lexy::_detail

namespace lexy::_detail
{
template <auto FnPtr, typename Indices = make_index_sequence<FnPtr().size()>>
struct _string_view_holder;
template <auto FnPtr, std::size_t... Indices>
struct _string_view_holder<FnPtr, index_sequence<Indices...>>
{
    static constexpr auto view = FnPtr();

    static constexpr typename decltype(view)::char_type value[] = {view[Indices]..., {}};
};

template <auto FnPtr>
constexpr const auto* make_cstr = _string_view_holder<FnPtr>::value;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_STRING_VIEW_HPP_INCLUDED


namespace lexy::_detail
{
template <typename T>
using _detect_name_f = std::enable_if_t<std::is_convertible_v<decltype(T::name()), string_view>>;
template <typename T>
using _detect_name_v = decltype(T::name);

template <typename T>
constexpr auto _full_type_name()
{
#if defined(__clang__)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = string_view("auto lexy::_detail::_full_type_name() [T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    return function;

#elif defined(__GNUC__) && __GNUC__ > 8
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix
        = string_view("constexpr auto lexy::_detail::_full_type_name() [with T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    return function;

#elif defined(_MSC_VER)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = string_view("auto __cdecl lexy::_detail::_full_type_name<");
    constexpr auto suffix = string_view(">(void)");

    auto function = string_view(__FUNCSIG__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());

    if (auto s = string_view("struct "); function.starts_with(s))
        function.remove_prefix(s.length());
    else if (auto c = string_view("class "); function.starts_with(c))
        function.remove_prefix(c.length());

    return function;

#else
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 0

    return string_view("unknown-type");

#endif
}

template <typename T, int NsCount>
LEXY_CONSTEVAL string_view _type_name()
{
    auto name = _full_type_name<T>();
    LEXY_ASSERT(name.find('<') == string_view::npos || NsCount == 0,
                "cannot strip namespaces from template instantiations");

    for (auto namespace_count = NsCount; namespace_count > 0; --namespace_count)
    {
        auto pos = name.find("::");
        if (pos == string_view::npos)
            break;
        name.remove_prefix(pos + 2);
    }
    return name;
}

template <typename T, int NsCount = 1>
LEXY_CONSTEVAL const char* type_name()
{
    if constexpr (_detail::is_detected<_detect_name_f, T>)
        return T::name();
    else if constexpr (_detail::is_detected<_detect_name_v, T>)
        return T::name;
    else
        return make_cstr<_type_name<T, NsCount>>;
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED


#ifdef LEXY_IGNORE_DEPRECATED_LIST
#    define LEXY_DEPRECATED_LIST
#else
#    define LEXY_DEPRECATED_LIST                                                                   \
        [[deprecated("Production::list has been deprecated; use ::value instead.")]]
#endif

namespace lexy
{
template <typename Production>
using production_rule = std::decay_t<decltype(Production::rule)>;

template <typename Production>
constexpr bool is_production = _detail::is_detected<production_rule, Production>;

/// Base class to indicate that this production is conceptually a token.
/// This inhibits whitespace skipping inside the production.
///
/// When generating a parse tree, it will also merge tokens of the same kind into the same node.
struct token_production
{};

template <typename Production>
constexpr bool is_token_production = std::is_base_of_v<token_production, Production>;

/// Base class to indicate that this production is transparent for the parse tree generation.
/// It will not create a node in the tree, all children will be added to the its parent.
/// If parse tree generation is not used, it has no effect.
struct transparent_production
{};

template <typename Production>
constexpr bool is_transparent_production = std::is_base_of_v<transparent_production, Production>;
} // namespace lexy

namespace lexy
{
template <typename Production>
LEXY_CONSTEVAL const char* production_name()
{
    return _detail::type_name<Production>();
}

template <typename Production>
using _detect_whitespace = decltype(Production::whitespace);

template <typename Production, typename Root>
auto _production_whitespace()
{
    if constexpr (is_token_production<Production>)
        return; // void
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Production>)
        return Production::whitespace;
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Root>)
        return Root::whitespace;
    else
        return; // void
}
template <typename Production, typename Root>
using production_whitespace = decltype(_production_whitespace<Production, Root>());
} // namespace lexy

namespace lexy
{
template <typename Production>
using _detect_value = decltype(&Production::value);
template <typename Production>
using _detect_list = decltype(&Production::list);

template <typename Production, bool HasList = _detail::is_detected<_detect_list, Production>,
          bool HasValue = _detail::is_detected<_detect_value, Production>>
struct _prod_value;
template <typename Production>
struct _prod_value<Production, true, true>
{
    // Before:
    //    static constexpr auto list = sink;
    //    static constexpr auto value = callback;
    // After:
    //    static constexpr auto value = sink >> callback;
    // Define LEXY_IGNORE_DEPRECATED_LIST to fix later.
    LEXY_DEPRECATED_LIST
    static constexpr auto get = Production::list >> Production::value;
};
template <typename Production>
struct _prod_value<Production, true, false>
{
    // Before:
    //    static constexpr auto list = sink;
    // After:
    //    static constexpr auto value = sink;
    // Define LEXY_IGNORE_DEPRECATED_LIST to fix later.
    LEXY_DEPRECATED_LIST
    static constexpr auto get = Production::list;
};
template <typename Production>
struct _prod_value<Production, false, true>
{
    static constexpr auto get = Production::value;
};
template <typename Production>
struct _prod_value<Production, false, false>
{
    static_assert(_detail::error<Production>, "missing Production::value member");
    static constexpr auto get = Production::value;
};

template <typename Production>
struct production_value
{
    static constexpr auto get = _prod_value<Production>::get;
    using type                = std::decay_t<decltype(get)>;
};
} // namespace lexy

#endif // LEXY_PRODUCTION_HPP_INCLUDED


#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

#ifdef LEXY_IGNORE_DEPRECATED_ERROR
#    define LEXY_DEPRECATED_ERROR(msg)
#else
#    define LEXY_DEPRECATED_ERROR(msg) [[deprecated(msg)]]
#endif

//=== rule ===//
#if 0
struct Rule : rule_base
{
    static constexpr auto is_branch = false;
    static constexpr auto is_unconditional_branch = false; // Only set to true if is_branch is also true.

    template <typename NextParser>
    struct parser
    {
        // Only if `is_branch == true` and `is_unconditional_branch == false`.
        template <typename Context, typename Reader, typename ... Args>
        LEXY_DSL_FUNC rule_try_parse_result try_parse(Context& context, Reader& reader, Args&&... args)
        {
            if (/* check whether we would match without consuming */)
            {
                if (/* matched and consumed */)
                {
                    auto result = NextParser::parse(context, reader, LEXY_FWD(args)..., /* rule arguments */);
                    return static_cast<rule_try_parse_result>(result);
                }
                else
                {
                    context.error(/* error */);
                    return rule_try_parse_result::canceled;
                }
            }
            else
            {
                return rule_try_parse_result::backtracked;
            }
        }

        template <typename Context, typename Reader, typename ... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (/* matched and consumed */)
                return NextParser::parse(context, reader, LEXY_FWD(args)..., /* rule arguments */);
            else
            {
                context.error(/* error */);
                return false;
            }
        }
    };
};
#endif

namespace lexyd
{
struct rule_base
{
    static constexpr auto is_branch               = false;
    static constexpr auto is_unconditional_branch = false;
};

struct _token_base : rule_base
{};
} // namespace lexyd

namespace lexy
{
// We use a shorthand namespace to decrease symbol size.
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;
template <typename T>
constexpr bool is_token = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_branch = [] {
    if constexpr (is_rule<T>)
        return T::is_branch;
    else
        return false;
}();

template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;
} // namespace lexy

namespace lexy
{
enum class rule_try_parse_result
{
    ok          = true,
    canceled    = false,
    backtracked = 2,
};
}

//=== whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsr;
template <>
struct _wsr<void> : rule_base
{
    template <typename NextParser>
    using parser = NextParser;
};
} // namespace lexyd

namespace lexy
{
struct _whitespace_tag
{};

template <typename Context>
using _ws_rule = std::conditional_t<
    // We need to disable whitespace if the context is already currently parsing whitespace.
    Context::contains(_whitespace_tag{}), void,
    lexy::production_whitespace<typename Context::production, typename Context::root>>;

template <typename Context, typename NextParser>
using whitespace_parser = rule_parser<lexy::dsl::_wsr<_ws_rule<Context>>, NextParser>;
} // namespace lexy

//=== parse_context ===//
namespace lexy
{
/// Stores contextual information for parsing the given production.
template <typename Production, typename Handler, typename HandlerState, typename Root = Production>
class parse_context
{
    static_assert(!lexy::is_token_production<Production> || std::is_same_v<Production, Root>,
                  "don't specify Root argument explicitly");

    template <typename ChildProduction, typename Iterator>
    using _parse_context_for = parse_context<
        ChildProduction, Handler,
        decltype(LEXY_DECLVAL(Handler&).start_production(ChildProduction{}, Iterator{})),
        // If it's a token we need to re-root it.
        std::conditional_t<lexy::is_token_production<ChildProduction>, ChildProduction, Root>>;

public:
    template <typename Iterator>
    constexpr explicit parse_context(Production p, Handler& handler, Iterator begin)
    : _handler(&handler), _state(_handler->start_production(p, begin))
    {}

    constexpr Handler& handler() const noexcept
    {
        return *_handler;
    }

    template <typename ChildProduction, typename Iterator>
    constexpr auto production_context(ChildProduction p, Iterator position) const
    {
        return _parse_context_for<ChildProduction, Iterator>(p, *_handler, position);
    }

    template <typename Id, typename T>
    constexpr auto insert(Id, T&& value)
    {
        return _stateful_context<parse_context, Id, std::decay_t<T>>(*this, LEXY_FWD(value));
    }

    template <typename Id>
    static LEXY_CONSTEVAL bool contains(Id)
    {
        return false;
    }

    template <typename Id>
    constexpr auto get(Id)
    {
        static_assert(lexy::_detail::error<Id>, "context does not contain a state with that id");
        return 0;
    }

    using production  = Production;
    using root        = Root;
    using return_type = typename Handler::template return_type_for<Production>;

    constexpr auto sink() const
    {
        return _handler->get_sink(Production{});
    }

    template <typename Iterator>
    constexpr void whitespace(Iterator begin, Iterator end)
    {
        _handler->whitespace(begin, end);
    }

    template <typename TokenKind, typename Iterator>
    constexpr void token(TokenKind kind, Iterator begin, Iterator end)
    {
        _handler->token(kind, begin, end);
    }

    template <typename... Args>
    constexpr void value(Args&&... args)
    {
        if constexpr (std::is_void_v<return_type>)
        {
            _handler->finish_production(Production{}, LEXY_MOV(_state), LEXY_FWD(args)...);
            _value.emplace();
        }
        else
        {
            auto value
                = _handler->finish_production(Production{}, LEXY_MOV(_state), LEXY_FWD(args)...);
            _value.emplace(LEXY_MOV(value));
        }
    }

    template <typename Error>
    constexpr void error(Error&& error)
    {
        _handler->error(Production{}, LEXY_MOV(_state), LEXY_FWD(error));
    }

    // Precondition: Either finish() or backtrack() must be called on every created context.
    constexpr auto finish() &&
    {
        if constexpr (!std::is_void_v<return_type>)
            return LEXY_MOV(*_value);
    }
    constexpr void backtrack() &&
    {
        _handler->backtrack_production(Production{}, LEXY_MOV(_state));
    }

private:
    template <typename Parent, typename Id, typename State>
    class _stateful_context
    {
    public:
        template <typename T>
        constexpr explicit _stateful_context(Parent& parent, T&& value)
        : _parent(&parent), _state(LEXY_FWD(value))
        {}

        constexpr Handler& handler() const noexcept
        {
            return _parent->handler();
        }

        template <typename ChildProduction, typename Iterator>
        constexpr auto production_context(ChildProduction p, Iterator position) const
        {
            return _parse_context_for<ChildProduction, Iterator>(p, _parent->handler(), position);
        }

        template <typename Id2, typename T>
        constexpr auto insert(Id2, T&& value)
        {
            return _stateful_context<_stateful_context, Id2, std::decay_t<T>>(*this,
                                                                              LEXY_FWD(value));
        }

        template <typename Id2>
        static LEXY_CONSTEVAL bool contains(Id2 id)
        {
            (void)id;
            if constexpr (std::is_same_v<Id, Id2>)
                return true;
            else
                return Parent::contains(id);
        }

        template <typename Id2>
        constexpr auto& get(Id2 id)
        {
            (void)id;
            if constexpr (std::is_same_v<Id2, Id>)
                return _state;
            else
                return _parent->get(id);
        }

        using production  = Production;
        using root        = Root;
        using return_type = typename Handler::template return_type_for<Production>;

        constexpr auto sink() const
        {
            return _parent->sink();
        }

        template <typename Iterator>
        constexpr void whitespace(Iterator begin, Iterator end)
        {
            _parent->whitespace(begin, end);
        }

        template <typename TokenKind, typename Iterator>
        constexpr void token(TokenKind kind, Iterator begin, Iterator end)
        {
            _parent->token(kind, begin, end);
        }

        template <typename... Args>
        constexpr void value(Args&&... args)
        {
            _parent->value(LEXY_FWD(args)...);
        }

        template <typename Error>
        constexpr void error(Error&& error)
        {
            _parent->error(LEXY_FWD(error));
        }

        constexpr auto finish() &&
        {
            return LEXY_MOV(*_parent).finish();
        }
        constexpr void backtrack() &&
        {
            LEXY_MOV(*_parent).backtrack();
        }

    private:
        Parent*                 _parent;
        LEXY_EMPTY_MEMBER State _state;
    };

    lexy::_detail::lazy_init<return_type> _value;
    Handler*                              _handler;
    LEXY_EMPTY_MEMBER HandlerState        _state;
};

template <typename Production, typename Handler, typename Iterator>
parse_context(Production p, Handler& handler, Iterator position)
    -> parse_context<Production, Handler, decltype(handler.start_production(p, position))>;

/// A final parser that forwards all elements to the context.
struct context_value_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader&, Args&&... args)
    {
        context.value(LEXY_FWD(args)...);
        return true;
    }
};

/// A final parser that drops all arguments; creating an empty result.
template <typename Context>
struct context_discard_parser
{
    template <typename NewContext, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(NewContext&, Reader&, Args&&...)
    {
        static_assert(sizeof...(Args) == 0, "looped rule must not produce any values");
        static_assert(std::is_same_v<Context, NewContext>,
                      "looped rule cannot add state to the context");
        return true;
    }
};
} // namespace lexy

namespace lexy::_detail
{
template <typename Production, typename Rule = lexy::production_rule<Production>, typename Handler,
          typename Reader>
constexpr auto parse_impl(Handler& handler, Reader& reader)
{
    lexy::parse_context context(Production{}, handler, reader.cur());

    using return_type = typename decltype(context)::return_type;
    lexy::_detail::lazy_init<return_type> result;

    if (lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader))
    {
        if constexpr (std::is_void_v<return_type>)
        {
            LEXY_MOV(context).finish();
            result.emplace();
        }
        else
        {
            result.emplace(LEXY_MOV(context).finish());
        }
    }
    else
    {
        LEXY_MOV(context).backtrack();
    }

    return result;
}
} // namespace lexy::_detail

#endif // LEXY_DSL_BASE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ERROR_HPP_INCLUDED
#define LEXY_ERROR_HPP_INCLUDED





namespace lexy
{
/// Generic failure.
template <typename Reader, typename Tag>
class error
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator pos) noexcept : _pos(pos), _end(pos) {}
    constexpr explicit error(typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : _pos(begin), _end(end)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* message() const noexcept
    {
        return _detail::type_name<Tag>();
    }

    constexpr auto begin() const noexcept
    {
        return _pos;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

private:
    typename Reader::iterator _pos;
    typename Reader::iterator _end;
};

/// Expected the literal character sequence.
struct expected_literal
{};
template <typename Reader>
class error<Reader, expected_literal>
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator pos, const typename Reader::char_type* str,
                             std::size_t index) noexcept
    : _pos(pos), _str(str), _idx(index)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr auto string() const noexcept -> const typename Reader::char_type*
    {
        return _str;
    }

    constexpr std::size_t index() const noexcept
    {
        return _idx;
    }

    constexpr auto character() const noexcept
    {
        return _str[_idx];
    }

private:
    typename Reader::iterator         _pos;
    const typename Reader::char_type* _str;
    std::size_t                       _idx;
};

/// Expected the given keyword.
/// Unlike expected_literal, this one looks at the following characters as well.
struct expected_keyword
{};
template <typename Reader>
class error<Reader, expected_keyword>
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const typename Reader::char_type* str)
    : _begin(begin), _end(end), _str(str)
    {}

    constexpr auto position() const noexcept
    {
        return _begin;
    }

    constexpr auto begin() const noexcept
    {
        return _begin;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

    constexpr auto string() const noexcept -> const typename Reader::char_type*
    {
        return _str;
    }

private:
    typename Reader::iterator         _begin;
    typename Reader::iterator         _end;
    const typename Reader::char_type* _str;
};

/// Expected a character of the specified character class.
struct expected_char_class
{};
template <typename Reader>
class error<Reader, expected_char_class>
{
    static_assert(is_canonical_reader<Reader>);

public:
    constexpr explicit error(typename Reader::iterator pos, const char* name) noexcept
    : _pos(pos), _name(name)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* character_class() const noexcept
    {
        return _name;
    }

private:
    typename Reader::iterator _pos;
    const char*               _name;
};

template <typename Input, typename Tag>
using error_for = error<input_reader<Input>, Tag>;

template <typename Reader, typename Tag, typename... Args>
constexpr auto make_error(Args&&... args)
{
    return error<typename Reader::canonical_reader, Tag>(LEXY_FWD(args)...);
}
} // namespace lexy

namespace lexy
{
// Contains information about the context of an error.
template <typename Production, typename Input>
class error_context
{
public:
    constexpr explicit error_context(const Input&                           input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : _input(&input), _pos(pos)
    {}
    constexpr explicit error_context(Production, const Input& input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : error_context(input, pos)
    {}

    // The input.
    constexpr const Input& input() const noexcept
    {
        return *_input;
    }

    // The name of the production where the error occurred.
    static LEXY_CONSTEVAL const char* production()
    {
        return production_name<Production>();
    }

    // The starting position of the production.
    constexpr auto position() const noexcept
    {
        return _pos;
    }

private:
    const Input*                           _input;
    typename input_reader<Input>::iterator _pos;
};
} // namespace lexy

#endif // LEXY_ERROR_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_HPP_INCLUDED
#define LEXY_CALLBACK_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_INVOKE_HPP_INCLUDED
#define LEXY_DETAIL_INVOKE_HPP_INCLUDED



namespace lexy::_detail
{
template <typename MemberPtr, bool = std::is_member_object_pointer_v<MemberPtr>>
struct _mem_invoker;
template <typename R, typename ClassT>
struct _mem_invoker<R ClassT::*, true>
{
    static constexpr decltype(auto) invoke(R ClassT::*f, ClassT& object)
    {
        return object.*f;
    }
    static constexpr decltype(auto) invoke(R ClassT::*f, const ClassT& object)
    {
        return object.*f;
    }

    template <typename Ptr>
    static constexpr auto invoke(R ClassT::*f, Ptr&& ptr) -> decltype((*LEXY_FWD(ptr)).*f)
    {
        return (*LEXY_FWD(ptr)).*f;
    }
};
template <typename F, typename ClassT>
struct _mem_invoker<F ClassT::*, false>
{
    template <typename ObjectT, typename... Args>
    static constexpr auto _invoke(int, F ClassT::*f, ObjectT&& object, Args&&... args)
        -> decltype((LEXY_FWD(object).*f)(LEXY_FWD(args)...))
    {
        return (LEXY_FWD(object).*f)(LEXY_FWD(args)...);
    }
    template <typename PtrT, typename... Args>
    static constexpr auto _invoke(short, F ClassT::*f, PtrT&& ptr, Args&&... args)
        -> decltype(((*LEXY_FWD(ptr)).*f)(LEXY_FWD(args)...))
    {
        return ((*LEXY_FWD(ptr)).*f)(LEXY_FWD(args)...);
    }

    template <typename... Args>
    static constexpr auto invoke(F ClassT::*f, Args&&... args)
        -> decltype(_invoke(0, f, LEXY_FWD(args)...))
    {
        return _invoke(0, f, LEXY_FWD(args)...);
    }
};

template <typename ClassT, typename F, typename... Args>
constexpr auto invoke(F ClassT::*f, Args&&... args)
    -> decltype(_mem_invoker<F ClassT::*>::invoke(f, LEXY_FWD(args)...))
{
    return _mem_invoker<F ClassT::*>::invoke(f, LEXY_FWD(args)...);
}

template <typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args) -> decltype(LEXY_FWD(f)(LEXY_FWD(args)...))
{
    return LEXY_FWD(f)(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_INVOKE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MEMBER_HPP_INCLUDED
#define LEXY_DSL_MEMBER_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_STATELESS_LAMBDA_HPP_INCLUDED
#define LEXY_DETAIL_STATELESS_LAMBDA_HPP_INCLUDED



namespace lexy::_detail
{
template <typename Lambda>
struct stateless_lambda
{
    static_assert(std::is_class_v<Lambda>);
    static_assert(std::is_empty_v<Lambda>);

    static constexpr Lambda get()
    {
        if constexpr (std::is_default_constructible_v<Lambda>)
        {
            // We're using C++20, lambdas are default constructible.
            return Lambda();
        }
        else
        {
            // We're not having C++20; use a sequence of weird workarounds to legally construct a
            // Lambda object without invoking any constructors.
            // This works and is well-defined, but sadly not constexpr.
            // Taken from: https://www.youtube.com/watch?v=yTb6xz_FSkY

            // We're defining two standard layout types that have a char as a common initial
            // sequence (as the Lambda is empty, it doesn't add anymore members to B).
            struct A
            {
                char member;
            };
            struct B : Lambda
            {
                char member;
            };
            static_assert(std::is_standard_layout_v<A> && std::is_standard_layout_v<B>);

            // We put the two types in a union and initialize the a member, which we can do.
            union storage_t
            {
                A a;
                B b;
            } storage{};

            // We can now take the address of member via b, as it is in the common initial sequence.
            auto char_ptr = &storage.b.member;
            // char_ptr is a pointer to the first member of B, so we can reinterpret_cast it to a
            // pointer to B.
            auto b_ptr = reinterpret_cast<B*>(char_ptr);
            // Now we're having a pointer to a B object, which can we can cast to the base class
            // Lambda.
            auto lambda_ptr = static_cast<Lambda*>(b_ptr);
            // Dereference the pointer to get the lambda object.
            return *lambda_ptr;
        }
    }

    template <typename... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        return get()(LEXY_FWD(args)...);
    }
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_STATELESS_LAMBDA_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRANCH_HPP_INCLUDED
#define LEXY_DSL_BRANCH_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEQUENCE_HPP_INCLUDED
#define LEXY_DSL_SEQUENCE_HPP_INCLUDED



namespace lexyd
{
template <typename... R>
struct _seq_impl;
template <>
struct _seq_impl<>
{
    template <typename NextParser>
    struct parser : NextParser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto _try_parse(Context& context, Reader& reader, Reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)...));
        }
    };
};
template <typename H, typename... T>
struct _seq_impl<H, T...>
{
    template <typename NextParser>
    struct parser : lexy::rule_parser<H, lexy::rule_parser<_seq_impl<T...>, NextParser>>
    {
        // Called by another _seq_impl instantiation.
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto _try_parse(Context& context, Reader& reader, Reader save, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            // We can safely discard, token does not produce any values.
            using token_parser = lexy::rule_parser<H, lexy::context_discard_parser<Context>>;
            auto result        = token_parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::ok)
            {
                // Continue trying the branch.
                using continuation = lexy::rule_parser<_seq_impl<T...>, NextParser>;
                return continuation::_try_parse(context, reader, save, LEXY_FWD(args)...);
            }
            else if (result == lexy::rule_try_parse_result::backtracked)
            {
                // Backtrack.
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }
            else
            {
                // Canceled.
                return lexy::rule_try_parse_result::canceled;
            }
        }

        // Only needed in the first instantiation.
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto save = reader;
            return _try_parse(context, reader, save, LEXY_FWD(args)...);
        }
    };
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);

    static constexpr auto is_branch               = (lexy::is_token<R> && ...);
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    using parser = lexy::rule_parser<_seq_impl<R...>, NextParser>;
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator+(R, S)
{
    return _seq<R, S>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator+(_seq<R...>, S)
{
    return _seq<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator+(R, _seq<S...>)
{
    return _seq<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator+(_seq<R...>, _seq<S...>)
{
    return _seq<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEQUENCE_HPP_INCLUDED


namespace lexyd
{
template <typename Condition, typename... R>
struct _br : rule_base
{
    static_assert(sizeof...(R) >= 0);

    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = Condition::is_unconditional_branch;

    // We simple connect Condition with R... and then NextParser.
    // Condition has a try_parse() that will try to match Condition and then continue on with the
    // continuation.
    template <typename NextParser>
    using parser = lexy::rule_parser<Condition, lexy::rule_parser<_seq_impl<R...>, NextParser>>;
};

//=== operator>> ===//
/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
LEXY_CONSTEVAL auto operator>>(Condition, Then)
{
    static_assert(lexy::is_branch<Condition>, "condition must be a branch");
    return _br<Condition, Then>{};
}
template <typename Condition, typename... R>
LEXY_CONSTEVAL auto operator>>(Condition, _seq<R...>)
{
    static_assert(lexy::is_branch<Condition>, "condition must be a branch");
    return _br<Condition, R...>{};
}
template <typename Condition, typename C, typename... R>
LEXY_CONSTEVAL auto operator>>(Condition, _br<C, R...>)
{
    static_assert(lexy::is_branch<Condition>, "condition must be a branch");
    return _br<Condition, C, R...>{};
}

// Prevent nested branches in `_br`'s condition.
template <typename C, typename... R, typename Then>
LEXY_CONSTEVAL auto operator>>(_br<C, R...>, Then)
{
    return C{} >> _seq<R..., Then>{};
}
template <typename C, typename... R, typename... S>
LEXY_CONSTEVAL auto operator>>(_br<C, R...>, _seq<S...>)
{
    return C{} >> _seq<R..., S...>{};
}

// Disambiguation.
template <typename C1, typename... R, typename C2, typename... S>
LEXY_CONSTEVAL auto operator>>(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}

//=== operator+ ===//
// If we add something on the left to a branch, we loose the branchy-ness.
template <typename Rule, typename Condition, typename... R>
LEXY_CONSTEVAL auto operator+(Rule rule, _br<Condition, R...>)
{
    return rule + _seq<Condition, R...>{};
}
// Disambiguation.
template <typename... R, typename Condition, typename... S>
LEXY_CONSTEVAL auto operator+(_seq<R...>, _br<Condition, S...>)
{
    return _seq<R...>{} + _seq<Condition, S...>{};
}

// If we add something on the right to a branch, we extend the then.
template <typename Condition, typename... R, typename Rule>
LEXY_CONSTEVAL auto operator+(_br<Condition, R...>, Rule)
{
    return _br<Condition, R..., Rule>{};
}
// Disambiguation.
template <typename Condition, typename... R, typename... S>
LEXY_CONSTEVAL auto operator+(_br<Condition, R...>, _seq<S...>)
{
    return _br<Condition, R..., S...>{};
}

// If we add two branches, we use the condition of the first one and treat the second as sequence.
template <typename C1, typename... R, typename C2, typename... S>
LEXY_CONSTEVAL auto operator+(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}
} // namespace lexyd

namespace lexyd
{
struct _else : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    using parser = NextParser;
};

/// Takes the branch unconditionally.
inline constexpr auto else_ = _else{};
} // namespace lexyd

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED


namespace lexy
{
template <auto Ptr>
struct _mem_ptr_fn
{
    template <typename Object, typename Value>
    constexpr void operator()(Object& object, Value&& value) const
    {
        object.*Ptr = LEXY_FWD(value);
    }
};

template <typename Fn>
struct member
{};

template <auto Ptr>
using make_member_ptr = member<_mem_ptr_fn<Ptr>>;
} // namespace lexy

namespace lexyd
{
template <typename Fn, typename Rule>
struct _mem : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader,
                                                                  LEXY_FWD(args)...,
                                                                  lexy::member<Fn>{});
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...,
                                                              lexy::member<Fn>{});
        }
    };
};

template <typename Fn>
struct _mem_dsl
{
    LEXY_CONSTEVAL _mem_dsl(Fn = {}) {}

    template <typename Rule>
    LEXY_CONSTEVAL auto operator=(Rule) const
    {
        using lambda = std::conditional_t<std::is_default_constructible_v<Fn>, Fn,
                                          lexy::_detail::stateless_lambda<Fn>>;
        return _mem<lambda, Rule>{};
    }
};

/// Specifies that the output of the associated rule should be stored in the member pointer. Used
/// with `lexy::as_aggregate`.
template <auto MemPtr>
constexpr auto member = _mem_dsl<lexy::_mem_ptr_fn<MemPtr>>{};

#define LEXY_MEM(Name)                                                                             \
    ::lexyd::_mem_dsl([](auto& obj, auto&& value) { obj.Name = LEXY_FWD(value); })
} // namespace lexyd

#endif // LEXY_DSL_MEMBER_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_LEXEME_HPP_INCLUDED
#define LEXY_LEXEME_HPP_INCLUDED






namespace lexy
{
template <typename Reader>
class lexeme
{
    static_assert(is_canonical_reader<Reader>, "lexeme must take the canonical reader");

public:
    using encoding  = typename Reader::encoding;
    using char_type = typename encoding::char_type;
    using iterator  = typename Reader::iterator;

    constexpr lexeme() noexcept : _begin(), _end() {}
    constexpr lexeme(iterator begin, iterator end) noexcept : _begin(begin), _end(end) {}

    constexpr explicit lexeme(const Reader& reader, iterator begin) noexcept
    : _begin(begin), _end(reader.cur())
    {}

    constexpr bool empty() const noexcept
    {
        return _begin == _end;
    }

    constexpr iterator begin() const noexcept
    {
        return _begin;
    }
    constexpr iterator end() const noexcept
    {
        return _end;
    }

    constexpr const char_type* data() const noexcept
    {
        static_assert(std::is_pointer_v<iterator>);
        return _begin;
    }

    constexpr std::size_t size() const noexcept
    {
        return static_cast<std::size_t>(_end - _begin);
    }

    constexpr char_type operator[](std::size_t idx) const noexcept
    {
        LEXY_PRECONDITION(idx < size());
        return _begin[idx];
    }

private:
    iterator _begin, _end;
};

template <typename Reader>
lexeme(const Reader&, typename Reader::iterator) -> lexeme<typename Reader::canonical_reader>;

template <typename Input>
using lexeme_for = lexeme<input_reader<Input>>;
} // namespace lexy

namespace lexy::_detail
{
template <typename Reader>
constexpr bool equal_lexemes(lexeme<Reader> lhs, lexeme<Reader> rhs)
{
    if constexpr (std::is_pointer_v<typename Reader::iterator>)
    {
        if (lhs.size() != rhs.size())
            return false;
    }

    auto lhs_cur = lhs.begin();
    auto rhs_cur = rhs.begin();
    while (lhs_cur != lhs.end() && rhs_cur != rhs.end())
    {
        if (*lhs_cur != *rhs_cur)
            return false;
        ++lhs_cur;
        ++rhs_cur;
    }
    return lhs_cur == lhs.end() && rhs_cur == rhs.end();
}
} // namespace lexy::_detail

#endif // LEXY_LEXEME_HPP_INCLUDED


//=== implementation ===//
namespace lexy
{
template <typename Fn>
struct _fn_holder
{
    Fn fn;

    constexpr explicit _fn_holder(Fn fn) : fn(fn) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype(_detail::invoke(fn, LEXY_FWD(args)...))
    {
        return _detail::invoke(fn, LEXY_FWD(args)...);
    }
};

template <typename Fn>
using _fn_as_base = std::conditional_t<std::is_class_v<Fn>, Fn, _fn_holder<Fn>>;
} // namespace lexy

//=== traits ===//
namespace lexy
{
template <typename T>
using _detect_callback = typename T::return_type;
template <typename T>
constexpr bool is_callback = _detail::is_detected<_detect_callback, T>;

template <typename T, typename... Args>
using _detect_callback_for = decltype(LEXY_DECLVAL(T)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_callback_for
    = _detail::is_detected<_detect_callback_for, std::decay_t<T>, Args...>;

/// Returns the type of the `.sink()` function.
template <typename Sink>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink());

template <typename T>
using _detect_sink = decltype(LEXY_DECLVAL(T).sink().finish());
template <typename T>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T>;
} // namespace lexy

//=== adapters ===//
namespace lexy
{
template <typename ReturnType, typename... Fns>
struct _callback : _fn_as_base<Fns>...
{
    using return_type = ReturnType;

    constexpr explicit _callback(Fns... fns) : _fn_as_base<Fns>(fns)... {}

    using _fn_as_base<Fns>::operator()...;
};

/// Creates a callback.
template <typename ReturnType = void, typename... Fns>
constexpr auto callback(Fns&&... fns)
{
    return _callback<ReturnType, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}

template <typename T, typename Callback>
class _sink_cb
{
public:
    using return_type = T;

    constexpr explicit _sink_cb(Callback cb) : _value(), _cb(cb) {}

    template <typename... Args>
    constexpr void operator()(Args&&... args)
    {
        // We pass the value and other arguments to the internal callback.
        _cb(_value, LEXY_FWD(args)...);
    }

    constexpr T&& finish() &&
    {
        return LEXY_MOV(_value);
    }

private:
    T                          _value;
    LEXY_EMPTY_MEMBER Callback _cb;
};

template <typename T, typename... Fns>
class _sink
{
public:
    constexpr explicit _sink(Fns... fns) : _cb(fns...) {}

    constexpr auto sink() const
    {
        return _sink_cb<T, _callback<void, Fns...>>(_cb);
    }

private:
    LEXY_EMPTY_MEMBER _callback<void, Fns...> _cb;
};

/// Creates a sink callback.
template <typename T, typename... Fns>
constexpr auto sink(Fns&&... fns)
{
    return _sink<T, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}

template <typename Container, typename Callback>
class _collect_sink
{
public:
    constexpr explicit _collect_sink(Callback callback) : _callback(LEXY_MOV(callback)) {}

    using return_type = Container;

    template <typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(void(LEXY_DECLVAL(Callback)(LEXY_FWD(args)...)))
    {
        _result.push_back(_callback(LEXY_FWD(args)...));
    }

    constexpr auto finish() &&
    {
        return LEXY_MOV(_result);
    }

private:
    Container                  _result;
    LEXY_EMPTY_MEMBER Callback _callback;
};
template <typename Callback>
class _collect_sink<void, Callback>
{
public:
    constexpr explicit _collect_sink(Callback callback) : _count(0), _callback(LEXY_MOV(callback))
    {}

    using return_type = std::size_t;

    template <typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(void(LEXY_DECLVAL(Callback)(LEXY_FWD(args)...)))
    {
        _callback(LEXY_FWD(args)...);
        ++_count;
    }

    constexpr auto finish() &&
    {
        return _count;
    }

private:
    std::size_t                _count;
    LEXY_EMPTY_MEMBER Callback _callback;
};

template <typename Container, typename Callback>
class _collect
{
public:
    constexpr explicit _collect(Callback callback) : _callback(LEXY_MOV(callback)) {}

    constexpr auto sink() const
    {
        return _collect_sink<Container, Callback>(_callback);
    }

private:
    LEXY_EMPTY_MEMBER Callback _callback;
};

/// Returns a sink that invokes the void-returning callback multiple times, resulting in the number
/// of times it was invoked.
template <typename Callback>
constexpr auto collect(Callback&& callback)
{
    using callback_t = std::decay_t<Callback>;
    static_assert(std::is_void_v<typename callback_t::return_type>,
                  "need to specify a container to collect into for non-void callbacks");
    return _collect<void, callback_t>(LEXY_FWD(callback));
}

/// Returns a sink that invokes the callback multiple times, storing each result in the container.
template <typename Container, typename Callback>
constexpr auto collect(Callback&& callback)
{
    using callback_t = std::decay_t<Callback>;
    static_assert(!std::is_void_v<typename callback_t::return_type>,
                  "cannot collect a void callback into a container");
    return _collect<Container, callback_t>(LEXY_FWD(callback));
}
} // namespace lexy

//=== composition ===//
namespace lexy
{
template <typename First, typename Second>
struct _compose_cb
{
    LEXY_EMPTY_MEMBER First  _first;
    LEXY_EMPTY_MEMBER Second _second;

    using return_type = typename Second::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::decay_t<decltype(_first(LEXY_FWD(args)...), LEXY_DECLVAL(return_type))>
    {
        return _second(_first(LEXY_FWD(args)...));
    }
};

template <typename Sink, typename Callback>
struct _compose_s
{
    LEXY_EMPTY_MEMBER Sink     _sink;
    LEXY_EMPTY_MEMBER Callback _callback;

    using return_type = typename Callback::return_type;

    constexpr auto sink() const
    {
        return _sink.sink();
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_callback(LEXY_FWD(args)...))
    {
        return _callback(LEXY_FWD(args)...);
    }
};

/// Composes two callbacks.
template <typename First, typename Second, typename = _detect_callback<First>,
          typename = _detect_callback<Second>>
constexpr auto operator|(First first, Second second)
{
    return _compose_cb<First, Second>{LEXY_MOV(first), LEXY_MOV(second)};
}
template <typename S, typename Cb, typename Second>
constexpr auto operator|(_compose_s<S, Cb> composed, Second second)
{
    auto cb = LEXY_MOV(composed._callback) | LEXY_MOV(second);
    return _compose_s<S, decltype(cb)>{LEXY_MOV(composed._sink), LEXY_MOV(cb)};
}

/// Composes a sink with a callback.
template <typename Sink, typename Callback, typename = _detect_sink<Sink>,
          typename = _detect_callback<Callback>>
constexpr auto operator>>(Sink sink, Callback cb)
{
    return _compose_s<Sink, Callback>{LEXY_MOV(sink), LEXY_MOV(cb)};
}
} // namespace lexy

//=== pre-defined callbacks ===//
namespace lexy
{
struct _noop
{
    using return_type = void;

    constexpr auto sink() const
    {
        // We don't need a separate type, noop itself can have the required functions.
        return *this;
    }

    template <typename... Args>
    constexpr void operator()(const Args&...) const
    {}

    constexpr void finish() && {}
};

/// A callback with sink that does nothing.
inline constexpr auto noop = _noop{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _fwd
{
    using return_type = T;

    constexpr T operator()(T&& t) const
    {
        return LEXY_MOV(t);
    }
    constexpr T operator()(const T& t) const
    {
        return t;
    }
};

/// A callback that just forwards an existing object.
template <typename T>
constexpr auto forward = _fwd<T>{};

template <typename T>
struct _construct
{
    using return_type = T;

    constexpr T operator()(T&& t) const
    {
        return LEXY_MOV(t);
    }
    constexpr T operator()(const T& t) const
    {
        return t;
    }

    template <typename... Args>
    constexpr T operator()(Args&&... args) const
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
            return T(LEXY_FWD(args)...);
        else
            return T{LEXY_FWD(args)...};
    }
};

/// A callback that constructs an object of type T by forwarding the arguments.
template <typename T>
constexpr auto construct = _construct<T>{};

template <typename T, typename PtrT>
struct _new
{
    using return_type = PtrT;

    constexpr PtrT operator()(T&& t) const
    {
        auto ptr = new T(LEXY_MOV(t));
        return PtrT(ptr);
    }
    constexpr PtrT operator()(const T& t) const
    {
        auto ptr = new T(t);
        return PtrT(ptr);
    }

    template <typename... Args>
    constexpr PtrT operator()(Args&&... args) const
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
        {
            auto ptr = new T(LEXY_FWD(args)...);
            return PtrT(ptr);
        }
        else
        {
            auto ptr = new T{LEXY_FWD(args)...};
            return PtrT(ptr);
        }
    }
};

/// A callback that constructs an object of type T on the heap by forwarding the arguments.
template <typename T, typename PtrT = T*>
constexpr auto new_ = _new<T, PtrT>{};
} // namespace lexy

namespace lexy
{
template <typename T>
struct _list
{
    struct _sink
    {
        T _result{};

        using return_type = T;

        template <typename U>
        auto operator()(U&& obj) -> decltype(_result.push_back(LEXY_FWD(obj)))
        {
            return _result.push_back(LEXY_FWD(obj));
        }

        template <typename... Args>
        void operator()(Args&&... args)
        {
            _result.emplace_back(LEXY_FWD(args)...);
        }

        T&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates a list of things (e.g. a `std::vector`, `std::list`, etc.).
/// As a callback, it forwards the arguments to the initializer list constructor.
/// As a sink, it repeatedly calls `push_back()` and `emplace_back()`.
template <typename T>
constexpr auto as_list = _list<T>{};

template <typename T>
struct _collection
{
    struct _sink
    {
        T _result{};

        using return_type = T;

        template <typename U>
        auto operator()(U&& obj) -> decltype(_result.insert(LEXY_FWD(obj)))
        {
            return _result.insert(LEXY_FWD(obj));
        }

        template <typename... Args>
        void operator()(Args&&... args)
        {
            _result.emplace(LEXY_FWD(args)...);
        }

        T&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates an unordered collection of things (e.g. a `std::set`,
/// `std::unordered_map`, etc.). As a callback, it forwards the arguments to the initializer list
/// constructor. As a sink, it repeatedly calls `insert()` and `emplace()`.
template <typename T>
constexpr auto as_collection = _collection<T>{};
} // namespace lexy

namespace lexy
{
template <typename MemPtr>
struct _mem_ptr_type_impl;
template <typename T, typename ClassT>
struct _mem_ptr_type_impl<T ClassT::*>
{
    using class_type  = ClassT;
    using member_type = T;
};

template <auto MemPtr>
using _mem_ptr_class_type = typename _mem_ptr_type_impl<decltype(MemPtr)>::class_type;
template <auto MemPtr>
using _mem_ptr_member_type = typename _mem_ptr_type_impl<decltype(MemPtr)>::member_type;

template <typename T>
struct _as_aggregate
{
    using return_type = T;
    static_assert(std::is_aggregate_v<return_type>);

    template <typename Fn, typename H, typename... Tail>
    constexpr void _set(T& result, lexy::member<Fn>, H&& value, Tail&&... tail) const
    {
        Fn()(result, LEXY_FWD(value));
        if constexpr (sizeof...(Tail) > 0)
            _set(result, LEXY_FWD(tail)...);
    }

    template <typename Fn, typename... Args>
    constexpr auto operator()(lexy::member<Fn> member, Args&&... args) const
    {
        static_assert(sizeof...(Args) % 2 == 1, "missing dsl::member rules");

        T result{};
        _set(result, member, LEXY_FWD(args)...);
        return result;
    }
    template <typename... Args>
    constexpr auto operator()(return_type&& result, Args&&... args) const
        -> std::enable_if_t<(sizeof...(Args) > 0), return_type>
    {
        static_assert(sizeof...(Args) % 2 == 0, "missing dsl::member rules");

        _set(result, LEXY_FWD(args)...);
        return LEXY_MOV(result);
    }

    struct _sink
    {
        T _result{};

        using return_type = T;

        template <typename Fn, typename Value>
        constexpr void operator()(lexy::member<Fn>, Value&& value)
        {
            Fn()(_result, LEXY_FWD(value));
        }

        constexpr auto&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates an aggregate.
template <typename T>
constexpr auto as_aggregate = _as_aggregate<T>{};
} // namespace lexy

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

namespace lexy
{
template <typename T>
struct _int
{
    using return_type = T;

    template <typename Integer>
    constexpr T operator()(const Integer& value) const
    {
        return T(value);
    }
    template <typename Integer>
    constexpr T operator()(int sign, const Integer& value) const
    {
        return T(sign * value);
    }
};

// A callback that takes an optional sign and an integer and produces the signed integer.
template <typename T>
constexpr auto as_integer = _int<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_HPP_INCLUDED




namespace lexy
{
class match_handler
{
public:
    constexpr match_handler() : _failed(false) {}

    constexpr explicit operator bool() const noexcept
    {
        return !_failed;
    }

    //=== handler functions ===//
    template <typename Production>
    using return_type_for = void;

    template <typename Production>
    constexpr auto get_sink(Production)
    {
        return noop.sink();
    }

    struct state
    {};

    template <typename Production, typename Iterator>
    constexpr state start_production(Production, Iterator)
    {
        return {};
    }

    template <typename Kind, typename Iterator>
    constexpr void token(Kind, Iterator, Iterator)
    {}

    template <typename Production, typename... Args>
    constexpr void finish_production(Production, state, Args&&...)
    {}
    template <typename Production>
    constexpr void backtrack_production(Production, state)
    {}

    template <typename Production, typename Error>
    constexpr void error(Production, state, Error&&)
    {
        _failed = true;
    }

private:
    bool _failed;
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto handler = match_handler{};
    auto reader  = input.reader();

    lexy::_detail::parse_impl<Production>(handler, reader);

    // We only match the production if no error was logged.
    return static_cast<bool>(handler);
}
} // namespace lexy

#endif // LEXY_MATCH_HPP_INCLUDED


namespace lexy
{
struct missing_token
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing token";
    }
};
} // namespace lexy

//=== token_base ===//
namespace lexyd
{
template <typename Tag, typename Token>
struct _toke;
template <auto Kind, typename Token>
struct _tokk;

template <typename Derived>
struct token_base : _token_base
{
    using token_type = Derived;

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Derived{};
    }

    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using token_engine = typename Derived::token_engine;

            auto begin = reader.cur();
            if (!lexy::engine_try_match<token_engine>(reader))
                return lexy::rule_try_parse_result::backtracked;
            auto end = reader.cur();
            context.token(Derived::token_kind(), begin, end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)...));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using token_engine = typename Derived::token_engine;

            auto position = reader.cur();
            if constexpr (lexy::engine_can_fail<token_engine, Reader>)
            {
                if (auto ec = token_engine::match(reader);
                    ec != typename token_engine::error_code())
                {
                    Derived::token_error(context, reader, ec, position);
                    return false;
                }
            }
            else
            {
                token_engine::match(reader);
            }
            context.token(Derived::token_kind(), position, reader.cur());

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _toke<Tag, Derived> error = _toke<Tag, Derived>{};

    template <auto Kind>
    static constexpr _tokk<Kind, Derived> kind = _tokk<Kind, Derived>{};
};

template <auto Kind, typename Token>
struct _tokk : token_base<_tokk<Kind, Token>>
{
    using token_engine = typename Token::token_engine;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        Token::token_error(context, reader, ec, pos);
    }

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Kind;
    }
};

template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>>
{
    using token_engine = typename Token::token_engine;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, Tag>(pos, reader.cur());
        context.error(err);
    }

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Token::token_kind();
    }

    LEXY_DEPRECATED_ERROR("replace `token.error<Tag>()` by `token.error<Tag>`")
    constexpr auto operator()() const
    {
        return *this;
    }
};
} // namespace lexyd

//=== token rule ===//
namespace lexyd
{
struct _token_dummy_production
{};

template <typename Rule>
struct _token : token_base<_token<Rule>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            auto handler = lexy::match_handler();
            lexy::_detail::parse_impl<_token_dummy_production, Rule>(handler, reader);
            return handler ? error_code() : error_code::error;
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::missing_token>(pos);
        context.error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Turns the arbitrary rule into a token by matching it without producing any values.
template <typename Rule>
LEXY_CONSTEVAL auto token(Rule)
{
    if constexpr (lexy::is_token<Rule>)
        return Rule{};
    else
        return _token<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TOKEN_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_FAILURE_HPP_INCLUDED
#define LEXY_ENGINE_FAILURE_HPP_INCLUDED



namespace lexy
{
/// Matches nothing, i.e. always fails without consuming anything.
struct engine_failure : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader&)
    {
        return error_code::error;
    }
};

template <typename Reader>
inline constexpr bool engine_can_succeed<engine_failure, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_FAILURE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_TRIE_HPP_INCLUDED
#define LEXY_ENGINE_TRIE_HPP_INCLUDED




namespace lexy
{
template <typename CharT, std::size_t NodeCount, std::size_t TransitionCount>
struct _trie
{
    static constexpr auto invalid_value = std::size_t(-1);

    LEXY_CONSTEVAL bool empty() const
    {
        return NodeCount == 1 && _node_value[0] == invalid_value;
    }
    LEXY_CONSTEVAL bool accepts_empty() const
    {
        return _node_value[0] != invalid_value;
    }

    LEXY_CONSTEVAL std::size_t node_value(std::size_t node) const
    {
        return _node_value[node];
    }

    LEXY_CONSTEVAL auto transition_count(std::size_t node) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        auto end   = _node_transition_idx[node];
        return end - begin;
    }

    LEXY_CONSTEVAL CharT transition_char(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        return _transition_char[begin + transition];
    }
    LEXY_CONSTEVAL std::size_t transition_next(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        return _transition_node[begin + transition];
    }

    // Arrays indexed by nodes.
    // The node has the transitions in the range [_node_transition_idx[node] - 1,
    // _node_transition_idx[node]].
    std::size_t _node_value[NodeCount];
    std::size_t _node_transition_idx[NodeCount];

    // Shared array for all transitions.
    CharT       _transition_char[TransitionCount == 0 ? 1 : TransitionCount];
    std::size_t _transition_node[TransitionCount == 0 ? 1 : TransitionCount];
};

template <typename CharT, typename... Strings, std::size_t... Idxs>
LEXY_CONSTEVAL auto _make_trie(lexy::_detail::index_sequence<Idxs...>)
{
    // We can estimate the number of nodes in the trie by adding all strings together.
    // This is the worst case where the strings don't share any nodes.
    // The plus one comes from the additional root node.
    constexpr auto node_count_upper_bound = (Strings::get().size() + ... + 1);

    // We cannot construct the `_trie` directly as we don't know how many transitions each node has.
    // So we use this temporary representation using an adjacency matrix.
    struct builder_t
    {
        std::size_t node_count       = 1;
        std::size_t transition_count = 0;

        std::size_t node_value[node_count_upper_bound] = {std::size_t(-1)};
        CharT       node_transition[node_count_upper_bound][node_count_upper_bound] = {};

        constexpr void insert(std::size_t value, const CharT* str, std::size_t size)
        {
            auto cur_node = std::size_t(0);
            for (auto ptr = str; ptr != str + size; ++ptr)
            {
                auto c = *ptr;
                LEXY_PRECONDITION(c);

                auto found = false;
                for (auto next_node = cur_node + 1; next_node < node_count; ++next_node)
                {
                    if (node_transition[cur_node][next_node] == c)
                    {
                        // The transition from `cur_node` to `next_node` using `c` is already in
                        // the trie. Follow it.
                        cur_node = next_node;
                        found    = true;
                        break;
                    }
                }

                if (!found)
                {
                    // We haven't found the transition, need to create a new node.
                    auto next_node                       = node_count++;
                    node_value[next_node]                = std::size_t(-1);
                    node_transition[cur_node][next_node] = c;
                    transition_count++;

                    cur_node = next_node;
                }
            }

            LEXY_PRECONDITION(node_value[cur_node]
                              == std::size_t(-1)); // duplicate string in alternative
            node_value[cur_node] = value;
        }
    };
    // We build the trie by inserting all strings.
    constexpr auto builder = [] {
        builder_t builder;
        (builder.insert(Idxs, Strings::get().data(), Strings::get().size()), ...);
        return builder;
    }();

    // Now we also now the exact number of nodes and transitions in the trie.
    _trie<CharT, builder.node_count, builder.transition_count> result{};

    // Translate the adjacency matrix representation into the actual trie representation.
    auto transition_idx = 0u;
    for (auto node = 0u; node != builder.node_count; ++node)
    {
        result._node_value[node] = builder.node_value[node];

        for (auto next_node = node + 1; next_node != builder.node_count; ++next_node)
            if (auto c = builder.node_transition[node][next_node])
            {
                // We've found a transition, add it to the shared transition array.
                result._transition_char[transition_idx] = c;
                result._transition_node[transition_idx] = next_node;
                ++transition_idx;
            }

        // The node transition end at the current transition index.
        result._node_transition_idx[node] = transition_idx;
    }

    return result;
}

/// A trie containing `Strings::get()` for every string.
template <typename CharT, typename... Strings>
constexpr auto trie
    = _make_trie<CharT, Strings...>(lexy::_detail::index_sequence_for<Strings...>{});

/// Matches one of the strings contained in the trie.
template <const auto& Trie>
struct engine_trie : engine_matcher_base, engine_parser_base
{
    enum class error_code
    {
        error = 1,
    };

    template <std::size_t Node>
    using _transition_sequence = lexy::_detail::make_index_sequence<Trie.transition_count(Node)>;

    template <std::size_t Node, typename Transitions>
    struct _node                              // Base case if we pass void as transitions.
    : _node<Node, _transition_sequence<Node>> // Compute transition and forward.
    {};
    template <std::size_t Node, std::size_t... Transitions>
    struct _node<Node, lexy::_detail::index_sequence<Transitions...>>
    {
        template <std::size_t Transition>
        using transition = _node<Trie.transition_next(Node, Transition), void>;

        template <typename Reader>
        static constexpr auto parse(Reader& reader)
        {
            using encoding = typename Reader::encoding;
            auto save      = reader;
            auto cur       = reader.peek();

            auto result = Trie.invalid_value;
            // Check the character of each transition.
            // If it matches, we advance by one and go to that node.
            // As soon as we do that, we return true to short circuit the search.
            (void)((cur == _char_to_int_type<encoding>(Trie.transition_char(Node, Transitions))
                        ? (reader.bump(), result = transition<Transitions>::parse(reader), true)
                        : false)
                   || ...);
            (void)cur;

            if constexpr (Trie.node_value(Node) != Trie.invalid_value)
            {
                // The current node accepts.
                // Check if we have a longer match.
                if (result != Trie.invalid_value)
                    return result;

                // We were unable to find a longer match, but the current node accepts anyway.
                // Return that match.
                reader = LEXY_MOV(save);
                return Trie.node_value(Node);
            }
            else
            {
                // The current node does not accept, so the result is completely determined by the
                // actual result.
                return result;
            }
        }
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We begin in the root node of the trie.
        return _node<0, void>::parse(reader) == Trie.invalid_value ? error_code::error
                                                                   : error_code();
    }

    template <typename Reader>
    static constexpr std::size_t parse(error_code& ec, Reader& reader)
    {
        auto result = _node<0, void>::parse(reader);
        if (result == Trie.invalid_value)
            ec = error_code::error;
        else
            ec = error_code();
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader&, error_code)
    {
        return false;
    }
};

template <const auto& Trie, typename Reader>
inline constexpr bool engine_can_fail<engine_trie<Trie>, Reader> = !Trie.accepts_empty();
template <const auto& Trie, typename Reader>
inline constexpr bool engine_can_succeed<engine_trie<Trie>, Reader> = !Trie.empty();
} // namespace lexy

#endif // LEXY_ENGINE_TRIE_HPP_INCLUDED


namespace lexy
{
struct exhausted_alternatives
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted alternatives";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Token>
using _detect_string = typename Token::string;
template <typename Token>
constexpr bool _can_use_trie = lexy::_detail::is_detected<_detect_string, Token>;

// Just a type_list type.
template <typename... Tokens>
struct _alt_impl
{};

template <typename... Tokens>
struct _alt_trie
{
    using _char_type           = std::common_type_t<typename Tokens::string::char_type...>;
    static constexpr auto trie = lexy::trie<_char_type, typename Tokens::string...>;
};

template <typename Trie, typename Manual, typename... Tokens>
struct _alt_engine;
template <typename... Lits, typename... Tokens>
struct _alt_engine<_alt_impl<Lits...>, _alt_impl<Tokens...>>
{
    struct engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            auto success        = false;
            auto longest_reader = reader;
            auto longest_match  = std::size_t(0);
            auto try_engine     = [&](auto engine) {
                // Match each engine on a fresh reader and determine the length of the match.
                auto copy = reader;
                if (!lexy::engine_try_match<decltype(engine)>(copy))
                    return false;
                auto length = lexy::_detail::range_size(reader.cur(), copy.cur());

                // Update previous maximum.
                if (length > longest_match)
                {
                    longest_match  = length;
                    longest_reader = LEXY_MOV(copy);
                }
                // We've succeeded in either case.
                success = true;

                // We can exit early if we've reached EOF -- there can't be a longer match.
                return copy.eof();
            };

            // Match each rule in some order.
            // We trie the trie first as it is more optimized and gives a longer initial maximum.
            [[maybe_unused]] auto done = false;
            if constexpr (sizeof...(Lits) > 0)
                done = try_engine(lexy::engine_trie<_alt_trie<Lits...>::trie>{});
            if constexpr (sizeof...(Tokens) > 0)
                (done || ... || try_engine(typename Tokens::token_engine{}));

            if (!success)
                return error_code::error;

            reader = LEXY_MOV(longest_reader);
            return error_code();
        }
    };
};
template <typename... Lits, typename... Tokens, typename H, typename... T>
struct _alt_engine<_alt_impl<Lits...>, _alt_impl<Tokens...>, H, T...>
{
    static auto _engine()
    {
        // Insert H into either the trie or the manual version.
        if constexpr (_can_use_trie<H>)
            return
                typename _alt_engine<_alt_impl<Lits..., H>, _alt_impl<Tokens...>, T...>::engine{};
        else
            return
                typename _alt_engine<_alt_impl<Lits...>, _alt_impl<Tokens..., H>, T...>::engine{};
    }
    using engine = decltype(_engine());
};

template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    struct token_engine : _alt_engine<_alt_impl<>, _alt_impl<>, Tokens...>::engine
    {};

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::exhausted_alternatives>(pos);
        context.error(err);
    }
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator/(R, S)
{
    static_assert(lexy::is_token<R> && lexy::is_token<S>);
    return _alt<R, S>{};
}

template <typename... R, typename S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token<S>);
    return _alt<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token<R>);
    return _alt<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ANY_HPP_INCLUDED
#define LEXY_DSL_ANY_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_ANY_HPP_INCLUDED
#define LEXY_ENGINE_ANY_HPP_INCLUDED



namespace lexy
{
/// Matches the entire input.
struct engine_any : engine_matcher_base
{
    enum class error_code
    {
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!reader.eof())
            reader.bump();
        return error_code();
    }
};

template <typename Reader>
inline constexpr bool engine_can_fail<engine_any, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_ANY_HPP_INCLUDED


namespace lexyd
{
struct _any : token_base<_any>
{
    using token_engine = lexy::engine_any;
};

/// Matches anything and consumes all remaining characters.
constexpr auto any = _any{};
} // namespace lexyd

#endif // LEXY_DSL_ANY_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ASCII_HPP_INCLUDED
#define LEXY_DSL_ASCII_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED
#define LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED
#define LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

#include <climits>



namespace lexy
{
/// Matches the inclusive range of characters (code units).
template <auto Min, auto Max>
struct engine_char_range : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        using encoding = typename Reader::encoding;

        auto cur = reader.peek();
        if (_char_to_int_type<encoding>(Min) <= cur && cur <= _char_to_int_type<encoding>(Max))
        {
            reader.bump();
            return error_code();
        }
        else
            return error_code::error;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code)
    {
        // Consume bad character.
        if (!reader.eof())
            reader.bump();
        return true;
    }
};
} // namespace lexy

namespace lexy
{
template <typename CharT, std::size_t TransitionCount>
struct _strie
{
    LEXY_CONSTEVAL auto transition_sequence() const
    {
        return lexy::_detail::make_index_sequence<TransitionCount>{};
    }

    template <typename Encoding>
    LEXY_CONSTEVAL auto transition(std::size_t transition) const
    {
        return _char_to_int_type<Encoding>(_transition[transition]);
    }

    CharT _transition[TransitionCount == 0 ? 1 : TransitionCount];
};

template <typename String>
LEXY_CONSTEVAL auto _make_strie()
{
    constexpr auto string = String::get();

    using strie_t = _strie<typename decltype(string)::char_type, string.size()>;
    strie_t result{};

    for (auto idx = 0u; idx != string.size(); ++idx)
        result._transition[idx] = string[idx];

    return result;
}

/// Produces a shallow trie that contains a transition to an accepting node for each character of
/// the string.
template <typename String>
constexpr auto shallow_trie = _make_strie<String>();

/// Matches one of the specified characters in the trie.
template <const auto& STrie>
struct engine_char_set : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader, std::size_t... Transitions>
    static constexpr auto _transition(Reader& reader, lexy::_detail::index_sequence<Transitions...>)
    {
        using encoding = typename Reader::encoding;

        auto cur     = reader.peek();
        auto matches = ((cur == STrie.template transition<encoding>(Transitions)) || ...);
        if (!matches)
            return error_code::error;

        reader.bump();
        return error_code();
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        return _transition(reader, STrie.transition_sequence());
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code)
    {
        // Consume bad character.
        if (!reader.eof())
            reader.bump();
        return true;
    }
};

} // namespace lexy

namespace lexy
{
template <std::size_t Bits>
auto _int_n_t_impl()
{
    if constexpr (Bits <= CHAR_BIT)
        return static_cast<unsigned char>(0);
    else if constexpr (Bits <= sizeof(unsigned short) * CHAR_BIT)
        return static_cast<unsigned short>(0);
    else if constexpr (Bits <= sizeof(unsigned int) * CHAR_BIT)
        return static_cast<unsigned int>(0);
    else if constexpr (Bits <= sizeof(unsigned long) * CHAR_BIT)
        return static_cast<unsigned long>(0);
    else if constexpr (Bits <= sizeof(unsigned long long) * CHAR_BIT)
        return static_cast<unsigned long long>(0);
    else
        static_assert(Bits <= sizeof(unsigned long long) * CHAR_BIT, "ASCII table to big");
}
template <std::size_t Bits>
using _int_n_t = decltype(_int_n_t_impl<Bits>());

/// A lookup table for ASCII characters.
template <std::size_t CategoryCount>
class ascii_table
{
    using int_n = _int_n_t<CategoryCount>;

public:
    LEXY_CONSTEVAL ascii_table() : _table() {}

    /// Adds the character to the given category.
    template <typename CharT>
    LEXY_CONSTEVAL ascii_table& insert(CharT c, std::size_t category)
    {
        auto as_unsigned = static_cast<unsigned char>(c);
        LEXY_PRECONDITION(as_unsigned <= 0x7F);
        LEXY_PRECONDITION(category < CategoryCount);

        // Set the given bit.
        _table[as_unsigned] |= int_n(1 << category);

        return *this;
    }

    /// Checks whether the character is in the given categories.
    template <typename Encoding, std::size_t... Categories>
    constexpr bool contains(typename Encoding::int_type i) const
    {
        static_assert(((Categories < CategoryCount) && ...));
        constexpr auto mask = ((1 << Categories) | ...);

        if (_char_to_int_type<Encoding>(0x00) <= i && i <= _char_to_int_type<Encoding>(0x7F))
        {
            auto index = static_cast<std::size_t>(i);
            return (_table[index] & mask) != 0;
        }
        else
        {
            return false;
        }
    }

private:
    int_n _table[0x80];
};

/// Matches one of the specified categories of the ASCII table.
template <const auto& Table, std::size_t... Categories>
struct engine_ascii_table : engine_matcher_base
{
    static_assert(sizeof...(Categories) > 0);

    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        auto cur = reader.peek();
        if (Table.template contains<typename Reader::encoding, Categories...>(cur))
        {
            reader.bump();
            return error_code();
        }
        else
        {
            return error_code::error;
        }
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code)
    {
        // Consume bad character.
        if (!reader.eof())
            reader.bump();
        return true;
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED


namespace lexy::_detail
{
// The categories in the lookup table.
enum ascii_table_categories
{
    ascii_table_control,
    ascii_table_space,
    ascii_table_alpha,
    ascii_table_alpha_underscore,
    ascii_table_digit, // 0-9 only
    ascii_table_hex_lower,
    ascii_table_hex_upper,
    ascii_table_punct,

    _ascii_table_count,
};
static_assert(_ascii_table_count <= CHAR_BIT);

// The actual table the lookup code uses.
constexpr auto dsl_ascii_table = [] {
    lexy::ascii_table<_ascii_table_count> result;

    for (auto c = 0x00; c <= 0x1F; ++c)
        result.insert(c, ascii_table_control);
    result.insert(0x7F, ascii_table_control);

    result.insert(' ', ascii_table_space);
    result.insert('\t', ascii_table_space);
    result.insert('\n', ascii_table_space);
    result.insert('\r', ascii_table_space);
    result.insert('\f', ascii_table_space);
    result.insert('\v', ascii_table_space);

    for (auto c = 'A'; c <= 'Z'; ++c)
    {
        result.insert(c, ascii_table_alpha);
        result.insert(c, ascii_table_alpha_underscore);
    }
    for (auto c = 'a'; c <= 'z'; ++c)
    {
        result.insert(c, ascii_table_alpha);
        result.insert(c, ascii_table_alpha_underscore);
    }
    result.insert('_', ascii_table_alpha_underscore);

    for (auto c = '0'; c <= '9'; ++c)
    {
        result.insert(c, ascii_table_digit);
        result.insert(c, ascii_table_hex_lower);
        result.insert(c, ascii_table_hex_upper);
    }
    for (auto c = 'A'; c <= 'F'; ++c)
        result.insert(c, ascii_table_hex_upper);
    for (auto c = 'a'; c <= 'f'; ++c)
        result.insert(c, ascii_table_hex_lower);

    result.insert('!', ascii_table_punct);
    result.insert('"', ascii_table_punct);
    result.insert('#', ascii_table_punct);
    result.insert('$', ascii_table_punct);
    result.insert('%', ascii_table_punct);
    result.insert('&', ascii_table_punct);
    result.insert('\'', ascii_table_punct);
    result.insert('(', ascii_table_punct);
    result.insert(')', ascii_table_punct);
    result.insert('*', ascii_table_punct);
    result.insert('+', ascii_table_punct);
    result.insert(',', ascii_table_punct);
    result.insert('-', ascii_table_punct);
    result.insert('.', ascii_table_punct);
    result.insert('/', ascii_table_punct);
    result.insert(':', ascii_table_punct);
    result.insert(';', ascii_table_punct);
    result.insert('<', ascii_table_punct);
    result.insert('=', ascii_table_punct);
    result.insert('>', ascii_table_punct);
    result.insert('?', ascii_table_punct);
    result.insert('@', ascii_table_punct);
    result.insert('[', ascii_table_punct);
    result.insert('\\', ascii_table_punct);
    result.insert(']', ascii_table_punct);
    result.insert('^', ascii_table_punct);
    result.insert('_', ascii_table_punct);
    result.insert('`', ascii_table_punct);
    result.insert('{', ascii_table_punct);
    result.insert('|', ascii_table_punct);
    result.insert('}', ascii_table_punct);
    result.insert('~', ascii_table_punct);

    return result;
}();
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED
#define LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED





#if LEXY_HAS_NTTP // string NTTP implementation



namespace lexy::_detail
{
template <std::size_t N, typename CharT>
struct string_literal
{
    CharT string[N + 1];

    using char_type = CharT;

    LEXY_CONSTEVAL string_literal(CharT c) : string{c, CharT()} {}
    LEXY_CONSTEVAL string_literal(const CharT* str) : string{}
    {
        for (auto i = 0u; i != N; ++i)
            string[i] = str[i];
    }

    LEXY_CONSTEVAL auto size() const
    {
        return N;
    }
};
template <std::size_t N, typename CharT>
string_literal(const CharT (&)[N]) -> string_literal<N - 1, CharT>;
template <typename CharT>
string_literal(CharT) -> string_literal<1, CharT>;

template <auto Str>
struct type_string
{
    using char_type            = typename decltype(Str)::char_type;
    static constexpr auto size = Str.size();

    template <typename CharT, typename Seq>
    struct _lazy;
    template <typename CharT, std::size_t... I>
    struct _lazy<CharT, index_sequence<I...>>
    {
        static inline constexpr CharT str[] = {CharT(Str.string[I])..., CharT()};
    };

    template <typename CharT = char_type>
    static LEXY_CONSTEVAL auto get()
    {
        using lazy = _lazy<CharT, make_index_sequence<Str.size()>>;
        return basic_string_view<CharT>(null_terminated{}, lazy::str, Str.size());
    }
};

template <auto C>
using type_char = type_string<string_literal<1, std::decay_t<decltype(C)>>(C)>;
} // namespace lexy::_detail

#    define LEXY_NTTP_STRING(Str) ::lexy::_detail::type_string<::lexy::_detail::string_literal(Str)>

#else // string<Cs...> implementation

namespace lexy::_detail
{
template <typename CharT, CharT... Cs>
struct type_string
{
    using char_type            = CharT;
    static constexpr auto size = sizeof...(Cs);

    template <typename OtherCharT>
    struct _lazy
    {
        static inline constexpr OtherCharT str[] = {OtherCharT(Cs)..., OtherCharT()};
    };

    template <typename OtherCharT = char_type>
    static LEXY_CONSTEVAL auto get()
    {
        return basic_string_view<OtherCharT>(null_terminated{}, _lazy<OtherCharT>::str,
                                             sizeof...(Cs));
    }
};

template <auto C>
using type_char = type_string<std::decay_t<decltype(C)>, C>;
} // namespace lexy::_detail

#    if defined(__GNUC__) // string<Cs...> literal implementation

#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wpedantic"
#        ifdef __clang__
#            pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#        endif

template <typename CharT, CharT... Cs>
constexpr ::lexy::_detail::type_string<CharT, Cs...> operator""_lexy_string_udl()
{
    return {};
}

#        define LEXY_NTTP_STRING(Str) decltype(Str##_lexy_string_udl)

#        pragma GCC diagnostic pop

#    else // string<Cs...> macro implementation

namespace lexy::_detail
{
template <typename A, typename B>
struct cat_;
template <typename CharT, CharT... C1, CharT... C2>
struct cat_<type_string<CharT, C1...>, type_string<CharT, C2...>>
{
    using type = type_string<CharT, C1..., C2...>;
};
template <typename A, typename B>
using cat = typename cat_<A, B>::type;

template <typename T, std::size_t Size, std::size_t MaxSize>
struct check_size
{
    static_assert(Size <= MaxSize, "string out of range");
    using type = T;
};

} // namespace lexy::_detail

#        define LEXY_NTTP_STRING_LENGTH(Str) (sizeof(Str) / sizeof(Str[0]) - 1)

// extract Ith character if not out of bounds
#        define LEXY_NTTP_STRING1(Str, I)                                                          \
            ::std::conditional_t<(I < LEXY_NTTP_STRING_LENGTH(Str)),                               \
                                 ::lexy::_detail::type_string<                                     \
                                     ::std::decay_t<decltype(Str[0])>,                             \
                                     (I >= LEXY_NTTP_STRING_LENGTH(Str) ? Str[0] : Str[I])>,       \
                                 ::lexy::_detail::type_string<::std::decay_t<decltype(Str[0])>>>

// recursively split the string in two
#        define LEXY_NTTP_STRING2(Str, I)                                                          \
            ::lexy::_detail::cat<LEXY_NTTP_STRING1(Str, I), LEXY_NTTP_STRING1(Str, I + 1)>
#        define LEXY_NTTP_STRING4(Str, I)                                                          \
            ::lexy::_detail::cat<LEXY_NTTP_STRING2(Str, I), LEXY_NTTP_STRING2(Str, I + 2)>
#        define LEXY_NTTP_STRING8(Str, I)                                                          \
            ::lexy::_detail::cat<LEXY_NTTP_STRING4(Str, I), LEXY_NTTP_STRING4(Str, I + 4)>
#        define LEXY_NTTP_STRING16(Str, I)                                                         \
            ::lexy::_detail::cat<LEXY_NTTP_STRING8(Str, I), LEXY_NTTP_STRING8(Str, I + 8)>
#        define LEXY_NTTP_STRING32(Str, I)                                                         \
            ::lexy::_detail::cat<LEXY_NTTP_STRING16(Str, I), LEXY_NTTP_STRING16(Str, I + 16)>

// instantiate with overflow check
#        define LEXY_NTTP_STRING(Str)                                                              \
            ::lexy::_detail::check_size<LEXY_NTTP_STRING32(Str, 0), LEXY_NTTP_STRING_LENGTH(Str),  \
                                        32>::type

#    endif

#endif

#endif // LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED





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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_LITERAL_HPP_INCLUDED
#define LEXY_ENGINE_LITERAL_HPP_INCLUDED




namespace lexy
{
template <typename CharT, std::size_t NodeCount>
struct _ltrie
{
    LEXY_CONSTEVAL bool empty() const
    {
        return NodeCount == 0;
    }

    LEXY_CONSTEVAL auto node_sequence() const
    {
        return lexy::_detail::make_index_sequence<NodeCount>{};
    }

    template <typename Encoding>
    LEXY_CONSTEVAL auto transition(std::size_t node) const
    {
        return _char_to_int_type<Encoding>(_transition[node]);
    }

    CharT _transition[NodeCount == 0 ? 1 : NodeCount];
};

template <typename String>
LEXY_CONSTEVAL auto _make_ltrie()
{
    constexpr auto string = String::get();

    using ltrie_t = _ltrie<typename decltype(string)::char_type, string.size()>;
    ltrie_t result{};

    for (auto idx = 0u; idx != string.size(); ++idx)
        result._transition[idx] = string[idx];

    return result;
}

/// Produces a linear trie, i.e. one that consists of only one string.
template <typename String>
constexpr auto linear_trie = _make_ltrie<String>();

/// Matches the linear trie.
template <const auto& LTrie>
struct engine_literal : engine_matcher_base
{
    enum class error_code
    {
    };

    static LEXY_CONSTEVAL error_code index_to_error(std::size_t idx)
    {
        return error_code(idx + 1);
    }

    static constexpr std::size_t index_from_error(error_code ec)
    {
        return std::size_t(ec) - 1;
    }

    template <typename Reader, std::size_t... Nodes>
    static constexpr auto _transition(Reader& reader, lexy::_detail::index_sequence<Nodes...>)
    {
        using encoding = typename Reader::encoding;

        auto result = error_code();
        (void)((reader.peek() == LTrie.template transition<encoding>(Nodes)
                    ? (reader.bump(), true)
                    : (result = index_to_error(Nodes), false))
               && ...);
        return result;
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        return _transition(reader, LTrie.node_sequence());
    }
};

template <const auto& LTrie, typename Reader>
inline constexpr bool engine_can_fail<engine_literal<LTrie>, Reader> = !LTrie.empty();
} // namespace lexy

#endif // LEXY_ENGINE_LITERAL_HPP_INCLUDED


namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom_impl
{
    static_assert(Endianness != lexy::encoding_endianness::bom,
                  "bom with BOM-endianness doesn't make sense");

    static constexpr auto name = "";

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view<unsigned char>();
    }
};
template <lexy::encoding_endianness DontCare>
struct _bom_impl<lexy::utf8_encoding, DontCare>
{
    static constexpr auto name = "BOM.UTF-8";

    static constexpr unsigned char value[] = {0xEF, 0xBB, 0xBF};
    static constexpr auto          length  = 3u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-16-LE";

    static constexpr unsigned char value[] = {0xFF, 0xFE};
    static constexpr auto          length  = 2u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-16-BE";

    static constexpr unsigned char value[] = {0xFE, 0xFF};
    static constexpr auto          length  = 2u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-32-LE";

    static constexpr unsigned char value[] = {0xFF, 0xFE, 0x00, 0x00};
    static constexpr auto          length  = 4u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-32-BE";

    static constexpr unsigned char value[] = {0x00, 0x00, 0xFE, 0xFF};
    static constexpr auto          length  = 4u;

    static constexpr auto get()
    {
        return lexy::_detail::basic_string_view(value, length);
    }
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom : token_base<_bom<Encoding, Endianness>>
{
    using string                = _bom_impl<Encoding, Endianness>;
    static constexpr auto _trie = lexy::linear_trie<string>;

    using token_engine = lexy::engine_literal<_trie>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, string::name);
        context.error(err);
    }
};

/// The BOM for that particular encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = _bom<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BRACKETS_HPP_INCLUDED
#define LEXY_DSL_BRACKETS_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED







namespace lexyd
{
template <typename String>
struct _lit : token_base<_lit<String>>
{
    using string = String;

    static constexpr auto _trie = lexy::linear_trie<String>;
    using token_engine          = lexy::engine_literal<_trie>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context&                          context, const Reader&,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        using reader_char_type = typename Reader::encoding::char_type;
        constexpr auto string  = String::template get<reader_char_type>();

        auto err
            = lexy::make_error<Reader, lexy::expected_literal>(pos, string.c_str(),
                                                               token_engine::index_from_error(ec));
        context.error(err);
    }

    //=== dsl ===//
    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

template <auto C>
constexpr auto lit_c = _lit<lexy::_detail::type_char<C>>{};

#if LEXY_HAS_NTTP
/// Matches the literal string.
template <lexy::_detail::string_literal Str>
constexpr auto lit = _lit<lexy::_detail::type_string<Str>>{};
#endif

#define LEXY_LIT(Str)                                                                              \
    ::lexyd::_lit<LEXY_NTTP_STRING(Str)> {}
} // namespace lexyd

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TERMINATOR_HPP_INCLUDED
#define LEXY_DSL_TERMINATOR_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED




namespace lexyd
{
template <typename Tag, typename Token>
struct _err : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&...)
        {
            auto begin = reader.cur();
            auto end   = reader.cur();
            if constexpr (!std::is_same_v<Token, void>)
            {
                auto copy = reader;
                Token::token_engine::match(copy);
                end = copy.cur();
            }

            auto err = lexy::make_error<Reader, Tag>(begin, end);
            context.error(err);
            return false;
        }
    };

    /// Adds a rule whose match will be part of the error location.
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        auto t = token(rule);
        return _err<Tag, decltype(t)>{};
    }
};

/// Matches nothing, produces an error with the given tag.
template <typename Tag>
constexpr auto error = _err<Tag, void>{};
} // namespace lexyd

namespace lexyd
{
template <typename Token, typename Tag>
struct _require : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (lexy::engine_peek<typename Token::token_engine>(reader))
            {
                // Token did match as we want, so continue normally.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // Token did not match, report the correct error, but continue normally.
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                context.error(err);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };
};

template <typename Token, typename Tag>
struct _prevent : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.cur();
            if (lexy::engine_try_match<typename Token::token_engine>(reader))
            {
                // Token did match what we don't want, and we've consumed it.
                // Report an error, but continue parsing.
                auto err = lexy::make_error<Reader, Tag>(begin, reader.cur());
                context.error(err);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // Token did not match, so we didn't consume it.
                // Continue normally.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };
};

template <typename Token>
struct _require_dsl
{
    template <typename Tag>
    static constexpr _require<Token, Tag> error = _require<Token, Tag>{};
};
template <typename Token>
struct _prevent_dsl
{
    template <typename Tag>
    static constexpr _prevent<Token, Tag> error = _prevent<Token, Tag>{};
};

/// Requires that lookahead will match a rule at a location.
template <typename Rule>
LEXY_CONSTEVAL auto require(Rule rule)
{
    auto t = token(rule);
    return _require_dsl<decltype(t)>{};
}

/// Requires that lookahead does not match a rule at a location.
template <typename Rule>
LEXY_CONSTEVAL auto prevent(Rule rule)
{
    auto t = token(rule);
    return _prevent_dsl<decltype(t)>{};
}

template <typename Tag, typename Rule>
LEXY_DEPRECATED_ERROR("replace `require<Tag>(rule)` by `require(rule).error<Tag>`")
LEXY_CONSTEVAL auto require(Rule rule)
{
    return require(rule).template error<Tag>;
}
template <typename Tag, typename Rule>
LEXY_DEPRECATED_ERROR("replace `prevent<Tag>(rule)` by `prevent(rule).error<Tag>`")
LEXY_CONSTEVAL auto prevent(Rule rule)
{
    return prevent(rule).template error<Tag>;
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_RECOVER_HPP_INCLUDED
#define LEXY_DSL_RECOVER_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CHOICE_HPP_INCLUDED
#define LEXY_DSL_CHOICE_HPP_INCLUDED




namespace lexy
{
struct exhausted_choice
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted choice";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename NextParser, typename... R>
struct _chc_parser;
template <typename NextParser>
struct _chc_parser<NextParser>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context&, Reader&, Args&&...) -> lexy::rule_try_parse_result
    {
        // We couldn't match any alternative of the choice, so as a branch we backtrack.
        return lexy::rule_try_parse_result::backtracked;
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&...)
    {
        auto err = lexy::make_error<Reader, lexy::exhausted_choice>(reader.cur());
        context.error(err);
        return false;
    }
};
template <typename NextParser, typename H, typename... T>
struct _chc_parser<NextParser, H, T...>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
        -> lexy::rule_try_parse_result
    {
        if constexpr (H::is_unconditional_branch)
        {
            // We know that we must take H, parse it and translate the result.
            if (lexy::rule_parser<H, NextParser>::parse(context, reader, LEXY_FWD(args)...))
                return lexy::rule_try_parse_result::ok;
            else
                return lexy::rule_try_parse_result::canceled;
        }
        else
        {
            // Try to parse H.
            auto result
                = lexy::rule_parser<H, NextParser>::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Try the next branch of the choice instead.
                return _chc_parser<NextParser, T...>::try_parse(context, reader, LEXY_FWD(args)...);
            else
                // We've taken H, return its result.
                return result;
        }
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if constexpr (H::is_unconditional_branch)
        {
            // We know that we must take H, so parse it.
            return lexy::rule_parser<H, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Try to parse H.
            auto result
                = lexy::rule_parser<H, NextParser>::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Try the next branch of the choice instead.
                return _chc_parser<NextParser, T...>::parse(context, reader, LEXY_FWD(args)...);
            else
                // We've taken H, return its translated result.
                return static_cast<bool>(result);
        }
    }
};

template <typename... R>
struct _chc : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = (R::is_unconditional_branch || ...);

    template <typename NextParser>
    using parser = _chc_parser<NextParser, R...>;
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator|(R, S)
{
    static_assert(lexy::is_branch<R>, "choice requires a branch condition");
    static_assert(lexy::is_branch<S>, "choice requires a branch condition");
    return _chc<R, S>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator|(_chc<R...>, S)
{
    static_assert(lexy::is_branch<S>, "choice requires a branch condition");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator|(R, _chc<S...>)
{
    static_assert(lexy::is_branch<R>, "choice requires a branch condition");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_EOF_HPP_INCLUDED
#define LEXY_ENGINE_EOF_HPP_INCLUDED



namespace lexy
{
/// Matches EOF.
struct engine_eof : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        return reader.eof() ? error_code() : error_code::error;
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_EOF_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_TOKEN_HPP_INCLUDED
#define LEXY_TOKEN_HPP_INCLUDED

#include <climits>
#include <cstdint>






namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind              = UINT_LEAST16_MAX,
    eof_token_kind                  = UINT_LEAST16_MAX - 1,
    position_token_kind             = UINT_LEAST16_MAX - 2,
    identifier_token_kind           = UINT_LEAST16_MAX - 3,
    _smallest_predefined_token_kind = identifier_token_kind,
};

constexpr const char* _kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";
    case eof_token_kind:
        return "EOF";
    case position_token_kind:
        return "position";
    case identifier_token_kind:
        return "identifier";
    }

    return ""; // unreachable
}
} // namespace lexy

namespace lexy
{
template <typename Token, auto Kind, typename Next>
struct _tk_map
{
    template <typename T>
    static LEXY_CONSTEVAL auto lookup(T)
    {
        if constexpr (std::is_same_v<typename T::token_type, Token>)
            return Kind;
        else
            return Next::lookup(T{});
    }

    template <decltype(Kind) NewKind, typename T>
    LEXY_CONSTEVAL auto map(T) const
    {
        static_assert(lexy::is_token<T>, "cannot map non-token to token kind");
        static_assert(!std::is_same_v<typename T::token_type, Token>, "already inserted");
        return _tk_map<typename T::token_type, NewKind, _tk_map>{};
    }
};

struct _tk_map_empty
{
    template <typename T>
    static LEXY_CONSTEVAL auto lookup(T)
    {
        return unknown_token_kind;
    }

    template <auto TokenKind, typename T>
    LEXY_CONSTEVAL auto map(T) const
    {
        static_assert(lexy::is_token<T>, "cannot map non-token to token kind");
        return _tk_map<typename T::token_type, TokenKind, _tk_map_empty>{};
    }
};

inline constexpr auto token_kind_map = _tk_map_empty{};

/// A mapping of token rule to token kind; specialize for your own kinds.
template <typename TokenKind>
constexpr auto token_kind_map_for = token_kind_map;
} // namespace lexy

namespace lexy
{
template <typename TokenKind>
using _detect_token_kind_name = decltype(token_kind_name(TokenKind{}));

/// What sort of token it is.
template <typename TokenKind = void>
class token_kind
{
    static_assert(std::is_enum_v<TokenKind>, "invalid type for TokenKind");

public:
    //=== constructors ===//
    /// Creates an unknown token kind.
    constexpr token_kind() noexcept : token_kind(unknown_token_kind) {}

    /// Creates a predefined token kind.
    constexpr token_kind(predefined_token_kind value) noexcept : _value(value) {}

    /// Creates the token kind with the specified value.
    constexpr token_kind(TokenKind value) noexcept : _value(static_cast<std::uint_least16_t>(value))
    {
        auto as_int = std::underlying_type_t<TokenKind>(value);
        LEXY_PRECONDITION(0 <= as_int && as_int < _smallest_predefined_token_kind);
    }

    /// Creates the token kind of a token rule.
    template <typename TokenRule, typename = std::enable_if_t<lexy::is_token<TokenRule>>>
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        // Look for internal mapping first.
        auto token_rule_kind = TokenRule::token_kind();
        if constexpr (std::is_same_v<decltype(token_rule_kind), TokenKind>)
            // The token has an associated kind.
            *this = token_kind(token_rule_kind);
        else
        {
            // Look for an external mapping.
            auto result = token_kind_map_for<TokenKind>.lookup(TokenRule{});
            *this       = token_kind(result);
        }
    }

    //=== access ===//
    constexpr explicit operator bool() const noexcept
    {
        return _value != unknown_token_kind;
    }

    constexpr bool is_predefined() const noexcept
    {
        return _value >= _smallest_predefined_token_kind;
    }

    constexpr const char* name() const noexcept
    {
        if (is_predefined())
            return _kind_name(static_cast<predefined_token_kind>(_value));
        else if constexpr (lexy::_detail::is_detected<_detect_token_kind_name, TokenKind>)
            return token_kind_name(get());
        else
            // We only have a generic name.
            return "token";
    }

    constexpr TokenKind get() const noexcept
    {
        return static_cast<TokenKind>(_value);
    }

    //=== comparision ===//
    friend constexpr bool operator==(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

    //=== raw access ===//
    static constexpr std::uint_least16_t to_raw(token_kind<TokenKind> kind) noexcept
    {
        return kind._value;
    }
    static constexpr token_kind<TokenKind> from_raw(std::uint_least16_t kind) noexcept
    {
        return token_kind<TokenKind>(kind);
    }

private:
    constexpr explicit token_kind(std::uint_least16_t kind) noexcept : _value(kind) {}

    std::uint_least16_t _value;
};
template <>
class token_kind<void>
{
public:
    /// Creates an unknown token kind.
    constexpr token_kind() noexcept : token_kind(unknown_token_kind) {}

    constexpr token_kind(predefined_token_kind value) noexcept : _value(value) {}

    /// Creates the token kind with the specified value.
    constexpr token_kind(int value) noexcept : _value(static_cast<std::uint_least16_t>(value))
    {
        LEXY_PRECONDITION(0 <= value && value < _smallest_predefined_token_kind);
    }

    /// Creates the token kind of a token rule.
    template <typename TokenRule, typename = std::enable_if_t<lexy::is_token<TokenRule>>>
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        auto token_rule_kind = TokenRule::token_kind();
        if constexpr (std::is_integral_v<decltype(token_rule_kind)>)
            // The token has an associated kind.
            *this = token_kind(int(token_rule_kind));
    }

    //=== access ===//
    constexpr explicit operator bool() const noexcept
    {
        return _value != unknown_token_kind;
    }

    constexpr bool is_predefined() const noexcept
    {
        return _value >= _smallest_predefined_token_kind;
    }

    constexpr const char* name() const noexcept
    {
        if (is_predefined())
            return _kind_name(static_cast<predefined_token_kind>(_value));
        else
            // We only have a generic name.
            return "token";
    }

    constexpr int get() const noexcept
    {
        if (is_predefined())
            return -1 - (UINT_LEAST16_MAX - _value);
        else
            return _value;
    }

    //=== comparision ===//
    friend constexpr bool operator==(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(token_kind lhs, token_kind rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

    //=== raw access ===//
    static constexpr std::uint_least16_t to_raw(token_kind<void> kind) noexcept
    {
        return kind._value;
    }
    static constexpr token_kind<void> from_raw(std::uint_least16_t kind) noexcept
    {
        return token_kind<void>(kind);
    }

private:
    constexpr explicit token_kind(std::uint_least16_t kind) noexcept : _value(kind) {}

    std::uint_least16_t _value;
};

template <typename TokenKind, typename = std::enable_if_t<std::is_integral_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<void>;
template <typename TokenKind, typename = std::enable_if_t<std::is_enum_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<TokenKind>;
template <typename TokenRule, typename = std::enable_if_t<lexy::is_token<TokenRule>>>
token_kind(TokenRule)
    -> token_kind<std::conditional_t<std::is_enum_v<decltype(TokenRule::token_kind())>,
                                     decltype(TokenRule::token_kind()), void>>;
} // namespace lexy

namespace lexy
{
/// A parsed token, i.e. its kind and its lexeme.
template <typename Reader, typename TokenKind = void>
class token
{
public:
    explicit constexpr token(token_kind<TokenKind> kind, lexy::lexeme<Reader> lex) noexcept
    : _lexeme(lex), _kind(kind)
    {
        LEXY_PRECONDITION(lex.begin() != typename Reader::iterator());
    }
    explicit constexpr token(token_kind<TokenKind> kind, typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : token(kind, lexy::lexeme<Reader>(begin, end))
    {}

    constexpr token_kind<TokenKind> kind() const noexcept
    {
        return _kind;
    }

    constexpr auto name() const noexcept
    {
        return _kind.name();
    }

    constexpr auto position() const noexcept -> typename Reader::iterator
    {
        return _lexeme.begin();
    }

    constexpr auto lexeme() const noexcept
    {
        return _lexeme;
    }

private:
    lexy::lexeme<Reader>  _lexeme;
    token_kind<TokenKind> _kind;
};

template <typename TokenKind, typename Reader>
token(token_kind<TokenKind>, lexy::lexeme<Reader>) -> token<Reader, TokenKind>;
template <typename TokenKind, typename Reader,
          typename = std::enable_if_t<std::is_integral_v<TokenKind>>>
token(TokenKind, lexy::lexeme<Reader>) -> token<Reader, void>;
template <typename TokenKind, typename Reader,
          typename = std::enable_if_t<std::is_enum_v<TokenKind>>>
token(TokenKind, lexy::lexeme<Reader>) -> token<Reader, TokenKind>;
template <typename TokenRule, typename Reader,
          typename = std::enable_if_t<lexy::is_token<TokenRule>>>
token(TokenRule, lexy::lexeme<Reader>)
    -> token<Reader, std::conditional_t<std::is_enum_v<decltype(TokenRule::token_kind())>,
                                        decltype(TokenRule::token_kind()), void>>;

template <typename Input, typename TokenKind = void>
using token_for = token<lexy::input_reader<Input>, TokenKind>;
} // namespace lexy

#endif // LEXY_TOKEN_HPP_INCLUDED


namespace lexyd
{
struct _eof : token_base<_eof>
{
    using token_engine = lexy::engine_eof;

    static LEXY_CONSTEVAL auto token_kind()
    {
        return lexy::eof_token_kind;
    }

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "EOF");
        context.error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

#endif // LEXY_DSL_EOF_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_FIND_HPP_INCLUDED
#define LEXY_ENGINE_FIND_HPP_INCLUDED



namespace lexy
{
/// Matches everything until and excluding Condition.
template <typename Condition>
struct engine_find : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition>);

    enum class error_code
    {
        not_found = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_peek<Condition>(reader))
        {
            if (reader.eof())
                return error_code::not_found;
            else
                reader.bump();
        }

        return error_code();
    }
};

/// Matches everything until and excluding Condition.
/// Fails when it matches Limit.
template <typename Condition, typename Limit>
struct engine_find_before : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition> && lexy::engine_is_matcher<Limit>);

    enum class error_code
    {
        not_found_eof = 1,
        not_found_limit,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_peek<Condition>(reader))
        {
            if (reader.eof())
                return error_code::not_found_eof;
            else if (engine_peek<Limit>(reader))
                return error_code::not_found_limit;
            else
                reader.bump();
        }

        return error_code();
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_FIND_HPP_INCLUDED


namespace lexyd
{
template <typename Token, typename Limit>
struct _find : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using engine = lexy::engine_find_before<typename Token::token_engine,
                                                    typename Limit::token_engine>;
            if (engine::match(reader) != typename engine::error_code())
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting token is found first.
    template <typename... Tokens>
    LEXY_CONSTEVAL auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));

        auto l = (Limit{} / ... / tokens);
        return _find<Token, decltype(l)>{};
    }

    LEXY_CONSTEVAL auto get_limit() const
    {
        return Limit{};
    }
};
template <typename Token>
struct _find<Token, void> : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using engine = lexy::engine_find<typename Token::token_engine>;
            if (engine::match(reader) != typename engine::error_code())
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting token is found first.
    template <typename... Tokens>
    LEXY_CONSTEVAL auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));

        auto l = (tokens / ...);
        return _find<Token, decltype(l)>{};
    }

    LEXY_CONSTEVAL auto get_limit() const
    {
        return eof;
    }
};

/// Recovers once it finds one of the given tokens (without consuming them).
template <typename... Tokens>
LEXY_CONSTEVAL auto find(Tokens... tokens)
{
    static_assert(sizeof...(Tokens) > 0);
    static_assert((lexy::is_token<Tokens> && ...));
    return _find<decltype((tokens / ...)), void>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Limit, typename... R>
struct _reco : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                // Try to match the recovery rules.
                using recovery = lexy::rule_parser<_chc<R...>, NextParser>;
                auto result    = recovery::try_parse(context, reader, LEXY_FWD(args)...);
                if (result != lexy::rule_try_parse_result::backtracked)
                    // We've succesfully recovered; return the recovered result.
                    return static_cast<bool>(result);

                // Cancel recovery when we've reached the limit.
                if (lexy::engine_peek<typename Limit::token_engine>(reader))
                    return false;

                // Consume one character and try again.
                reader.bump();
            }

            return false; // unreachable
        }
    };

    //=== dsl ===//
    /// Fail error recovery if Token is found before any of R.
    template <typename... Tokens>
    LEXY_CONSTEVAL auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));

        auto l = (Limit{} / ... / tokens);
        return _reco<decltype(l), R...>{};
    }

    LEXY_CONSTEVAL auto get_limit() const
    {
        return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
LEXY_CONSTEVAL auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    static_assert((lexy::is_branch<Branches> && ...));
    return _reco<lexyd::_eof, Branches...>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule, typename Recover>
struct _tryr : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (lexy::rule_parser<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...))
                // The rule was parsed succesfully, we're done here.
                return true;
            else
            {
                if constexpr (std::is_void_v<Recover>)
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                    return lexy::rule_parser<Recover, NextParser>::parse(context, reader,
                                                                         LEXY_FWD(args)...);
            }
        }
    };
};

/// Pares Rule, if that fails, continues immediately.
template <typename Rule>
LEXY_CONSTEVAL auto try_(Rule)
{
    return _tryr<Rule, void>{};
}

/// Parses Rule, if that fails, parses recovery rule.
template <typename Rule, typename Recover>
LEXY_CONSTEVAL auto try_(Rule, Recover)
{
    return _tryr<Rule, Recover>{};
}
} // namespace lexyd

#endif // LEXY_DSL_RECOVER_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOP_HPP_INCLUDED
#define LEXY_DSL_LOOP_HPP_INCLUDED



namespace lexyd
{
struct _break : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader&, Args&&...)
        {
            static_assert(sizeof...(Args) == 0, "looped rule must not add any values");

            // We set loop break on the member with the specified id.
            context.get(_break{}).loop_break = true;
            return true;
        }
    };
};

/// Exits a loop().
constexpr auto break_ = _break{};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _loop : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            struct flag
            {
                bool loop_break = false;
            };

            auto loop_context = context.insert(_break{}, flag{});
            while (!loop_context.get(_break{}).loop_break)
            {
                using parser
                    = lexy::rule_parser<Rule, lexy::context_discard_parser<decltype(loop_context)>>;
                if (!parser::parse(loop_context, reader))
                    return false;
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeatedly matches the rule until a break rule matches.
template <typename Rule>
LEXY_CONSTEVAL auto loop(Rule)
{
    return _loop<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOP_HPP_INCLUDED


// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_WHILE_HPP_INCLUDED
#define LEXY_ENGINE_WHILE_HPP_INCLUDED



namespace lexy
{
/// Matches `Matcher` as often as possible.
template <typename Matcher>
struct engine_while : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Matcher>);

    enum class error_code
    {
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (engine_try_match<Matcher>(reader))
        {}

        return error_code();
    }
};

template <typename Matcher, typename Reader>
inline constexpr bool engine_can_fail<engine_while<Matcher>, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_WHILE_HPP_INCLUDED


#ifdef LEXY_IGNORE_DEPRECATED_WHITESPACE
#    define LEXY_DEPRECATED_WHITESPACE
#else
#    define LEXY_DEPRECATED_WHITESPACE                                                             \
        [[deprecated("operator[] has been deprecated; use dsl::whitespace(ws) instead.")]]
#endif

namespace lexyd
{
template <typename Rule>
struct _wsr : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (lexy::is_token<Rule>)
            {
                // Parsing a token repeatedly cannot fail, so we can optimize it using an engine.
                using engine = lexy::engine_while<typename Rule::token_engine>;

                auto begin = reader.cur();
                engine::match(reader);
                auto end = reader.cur();

                context.token(Rule::token_kind(), begin, end);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // We need to mark the context with the tag to prevent infinite recursion.
                auto ws_context = context.insert(lexy::_whitespace_tag{}, lexy::_whitespace_tag{});

                // We can then parse the rule repeatedly using the special context.
                using loop_parser
                    = lexy::rule_parser<decltype(loop(Rule{} | break_)),
                                        lexy::context_discard_parser<decltype(ws_context)>>;
                if (!loop_parser::parse(ws_context, reader))
                    return false;

                // And continue with normal parsing.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    template <typename R>
    friend LEXY_CONSTEVAL auto operator|(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} | r)>{};
    }
    template <typename R>
    friend LEXY_CONSTEVAL auto operator|(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r | Rule{})>{};
    }

    template <typename R>
    friend LEXY_CONSTEVAL auto operator/(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} / r)>{};
    }
    template <typename R>
    friend LEXY_CONSTEVAL auto operator/(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r / Rule{})>{};
    }
};

struct _ws : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    /// Overrides implicit whitespace detection.
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule) const
    {
        return _wsr<Rule>{};
    }
};

/// Matches whitespace.
constexpr auto whitespace = _ws{};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _wsn : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        struct _cont
        {
            template <typename WsContext, typename Reader, typename Context, typename... Args>
            LEXY_DSL_FUNC bool parse(WsContext&, Reader& reader, Context& context, Args&&... args)
            {
                // Continue with the normal context, after skipping whitespace.
                return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                           LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            // Parse the rule using the context that doesn't allow inner whitespace.
            auto ws_context = context.insert(lexy::_whitespace_tag{}, lexy::_whitespace_tag{});
            return lexy::rule_parser<Rule, _cont>::try_parse(ws_context, reader, context,
                                                             LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the rule using the context that doesn't allow inner whitespace.
            auto ws_context = context.insert(lexy::_whitespace_tag{}, lexy::_whitespace_tag{});
            return lexy::rule_parser<Rule, _cont>::parse(ws_context, reader, context,
                                                         LEXY_FWD(args)...);
        }
    };
};

/// Disables automatic skipping of whitespace for all tokens of the given rule.
template <typename Rule>
LEXY_CONSTEVAL auto no_whitespace(Rule)
{
    if constexpr (lexy::is_token<Rule>)
        return Rule{}; // Token already behaves that way.
    else
        return _wsn<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule, typename Whitespace>
struct _wsd : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using ws = decltype(token(loop(Whitespace{} | break_)));
            lexy::engine_try_match<typename ws::token_engine>(reader);

            return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader,
                                                                  LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using rule = decltype(loop(token(Whitespace{}) | break_) + Rule{});
            return lexy::rule_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Tag>
    LEXY_CONSTEVAL auto error() const
    {
        static_assert(lexy::is_token<Rule>);
        return Rule{}.template error<Tag>();
    }
};

/// Matches whitespace before parsing rule.
template <typename Rule, typename Whitespace>
LEXY_DEPRECATED_WHITESPACE LEXY_CONSTEVAL auto whitespaced(Rule, Whitespace)
{
    return _wsd<Rule, Whitespace>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED


namespace lexyd
{
template <typename Terminator, typename R, typename Recover>
struct _optt;
template <typename Terminator, typename R, typename Recover>
struct _whlt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _lstt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _olstt;

template <typename Terminator, typename... RecoveryLimit>
struct _term
{
    /// Adds the tokens to the recovery limit.
    template <typename... Tokens>
    LEXY_CONSTEVAL auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));
        return _term<Terminator, RecoveryLimit..., Tokens...>{};
    }

    //=== rules ===//
    /// Matches rule followed by the terminator.
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        return rule + terminator();
    }

    /// Matches rule followed by the terminator, recovering on error.
    template <typename Rule>
    LEXY_CONSTEVAL auto try_(Rule rule) const
    {
        return lexyd::try_(rule + terminator(), recovery_rule());
    }

    /// Matches rule as long as terminator isn't matched.
    template <typename Rule>
    LEXY_CONSTEVAL auto while_(Rule) const
    {
        return _whlt<Terminator, Rule, decltype(recovery_rule())>{};
    }
    /// Matches rule as long as terminator isn't matched, but at least once.
    template <typename Rule>
    LEXY_CONSTEVAL auto while_one(Rule rule) const
    {
        if constexpr (lexy::is_branch<Rule>)
            return rule >> while_(rule);
        else
            return rule + while_(rule);
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt(R) const
    {
        return _optt<Terminator, R, decltype(recovery_rule())>{};
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto list(R) const
    {
        return _lstt<Terminator, R, void, decltype(recovery_rule())>{};
    }
    template <typename R, typename Sep>
    LEXY_CONSTEVAL auto list(R, Sep) const
    {
        return _lstt<Terminator, R, Sep, decltype(recovery_rule())>{};
    }

    /// Matches `opt_list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt_list(R) const
    {
        return _olstt<Terminator, R, void, decltype(recovery_rule())>{};
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto opt_list(R, S) const
    {
        return _olstt<Terminator, R, S, decltype(recovery_rule())>{};
    }

    //=== access ===//
    /// Matches the terminator alone.
    LEXY_CONSTEVAL auto terminator() const
    {
        return Terminator{};
    }

    /// Matches the recovery rule alone.
    LEXY_CONSTEVAL auto recovery_rule() const
    {
        if constexpr (sizeof...(RecoveryLimit) == 0)
            return recover(terminator());
        else
            return recover(terminator()).limit(RecoveryLimit{}...);
    }

    //=== deprecated ===//
    /// Sets the whitespace that will be skipped before the terminator.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws ws) const
    {
        auto branch = whitespaced(Terminator{}, ws);
        return _term<decltype(branch)>{};
    }
};

/// Creates a terminator using the given branch.
template <typename Branch>
LEXY_CONSTEVAL auto terminator(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _term<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TERMINATOR_HPP_INCLUDED


namespace lexyd
{
template <typename Open, typename Close, typename... RecoveryLimit>
struct _brackets
{
    /// Adds the tokens to the recovery limit.
    template <typename... Tokens>
    LEXY_CONSTEVAL auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));
        return _brackets<Open, Close, RecoveryLimit..., Tokens...>{};
    }

    //=== rules ===//
    /// Matches the rule surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto operator()(R r) const
    {
        return open() >> as_terminator()(r);
    }

    /// Matches the rule surrounded by brackets, recovering on error.
    template <typename R>
    LEXY_CONSTEVAL auto try_(R r) const
    {
        return open() >> as_terminator().try_(r);
    }

    /// Matches rule as often as possible, surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto while_(R r) const
    {
        return open() >> as_terminator().while_(r);
    }
    /// Matches rule as often as possible but at least once, surrounded by brackets.
    template <typename R>
    LEXY_CONSTEVAL auto while_one(R r) const
    {
        return open() >> r + as_terminator().while_(r);
    }

    /// Matches `opt(r)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt(R r) const
    {
        return open() >> as_terminator().opt(r);
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto list(R r) const
    {
        return open() >> as_terminator().list(r);
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto list(R r, S sep) const
    {
        return open() >> as_terminator().list(r, sep);
    }

    /// Matches `opt_list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    LEXY_CONSTEVAL auto opt_list(R r) const
    {
        return open() >> as_terminator().opt_list(r);
    }
    template <typename R, typename S>
    LEXY_CONSTEVAL auto opt_list(R r, S sep) const
    {
        return open() >> as_terminator().opt_list(r, sep);
    }

    //=== access ===//
    /// Matches the open bracket.
    LEXY_CONSTEVAL auto open() const
    {
        return Open{};
    }
    /// Matches the closing bracket.
    LEXY_CONSTEVAL auto close() const
    {
        return Close{};
    }

    /// Returns an equivalent terminator.
    LEXY_CONSTEVAL auto as_terminator() const
    {
        return _term<Close, RecoveryLimit...>{};
    }

    LEXY_CONSTEVAL auto recovery_rule() const
    {
        return as_terminator().recovery_rule();
    }

    //=== deprecated ===//
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws ws) const
    {
        auto open  = whitespaced(Open{}, ws);
        auto close = whitespaced(Close{}, ws);
        return _brackets<decltype(open), decltype(close)>{};
    }
};

/// Defines open and close brackets.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto brackets(Open, Close)
{
    static_assert(lexy::is_branch<Open> && lexy::is_branch<Close>);
    return _brackets<Open, Close>{};
}

constexpr auto round_bracketed  = brackets(lit_c<'('>, lit_c<')'>);
constexpr auto square_bracketed = brackets(lit_c<'['>, lit_c<']'>);
constexpr auto curly_bracketed  = brackets(lit_c<'{'>, lit_c<'}'>);
constexpr auto angle_bracketed  = brackets(lit_c<'<'>, lit_c<'>'>);

constexpr auto parenthesized = round_bracketed;
} // namespace lexyd

#endif // LEXY_DSL_BRACKETS_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED




namespace lexyd
{
template <typename Rule>
struct _cap : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename... PrevArgs>
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args,
                                     typename Reader::iterator begin, Args&&... args)
            {
                return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                         lexy::lexeme(reader, begin), LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto begin = reader.cur();

            using continuation = _continuation<Args...>;
            return lexy::rule_parser<Rule, continuation>::try_parse(context, reader,
                                                                    LEXY_FWD(args)..., begin);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.cur();

            using continuation = _continuation<Args...>;
            return lexy::rule_parser<Rule, continuation>::parse(context, reader, LEXY_FWD(args)...,
                                                                begin);
        }
    };

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Captures whatever the rule matches as a lexeme.
template <typename Rule>
LEXY_CONSTEVAL auto capture(Rule)
{
    return _cap<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_CODE_POINT_HPP_INCLUDED
#define LEXY_ENGINE_CODE_POINT_HPP_INCLUDED




namespace lexy
{
enum class _cp_error_code
{
    eof = 1,
    leads_with_trailing,
    missing_trailing,
    surrogate,
    overlong_sequence,
    out_of_range,
};

/// Parses an ASCII code point.
struct engine_cp_ascii : engine_matcher_base, engine_parser_base
{
    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, ascii_encoding>);

        if (reader.eof())
        {
            ec = error_code::eof;
            return code_point();
        }

        auto cur = reader.peek();
        reader.bump();

        auto cp = code_point(static_cast<char32_t>(cur));
        if (!cp.is_ascii())
            ec = error_code::out_of_range;
        return cp;
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader&, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::out_of_range:
            // Invalid code unit, but already consumed.
            return true;

        case error_code::leads_with_trailing:
        case error_code::missing_trailing:
        case error_code::surrogate:
        case error_code::overlong_sequence:
            LEXY_ASSERT(false, "unreachable");
        }
        return false;
    }
};

/// Matches a UTF-8 code point.
struct engine_cp_utf8 : engine_matcher_base, engine_parser_base
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

    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, utf8_encoding>);

        auto first = reader.peek();
        if ((first & ~payload_lead1) == pattern_lead1)
        {
            // ASCII character.
            reader.bump();
            return code_point(first);
        }
        else if ((first & ~payload_cont) == pattern_cont)
        {
            ec = error_code::leads_with_trailing;
            return code_point();
        }
        else if ((first & ~payload_lead2) == pattern_lead2)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto result = char32_t(first & payload_lead2);
            result <<= 6;
            result |= char32_t(second & payload_cont);

            // C0 and C1 are overlong ASCII.
            if (first == 0xC0 || first == 0xC1)
                ec = error_code::overlong_sequence;

            return code_point(result);
        }
        else if ((first & ~payload_lead3) == pattern_lead3)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto result = char32_t(first & payload_lead3);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);

            auto cp = code_point(result);
            if (cp.is_surrogate())
                ec = error_code::surrogate;
            else if (first == 0xE0 && second < 0xA0)
                ec = error_code::overlong_sequence;
            return cp;
        }
        else if ((first & ~payload_lead4) == pattern_lead4)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto fourth = reader.peek();
            if ((fourth & ~payload_cont) != pattern_cont)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            auto result = char32_t(first & payload_lead4);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);
            result <<= 6;
            result |= char32_t(fourth & payload_cont);

            auto cp = code_point(result);
            if (!cp.is_valid())
                ec = error_code::out_of_range;
            else if (first == 0xF0 && second < 0x90)
                ec = error_code::overlong_sequence;
            return cp;
        }
        else // FE or FF
        {
            ec = error_code::eof;
            return code_point();
        }
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::leads_with_trailing:
            // Invalid code unit, consume to recover.
            reader.bump();
            return true;
        case error_code::missing_trailing:
        case error_code::surrogate:
        case error_code::out_of_range:
        case error_code::overlong_sequence:
            // We've already consumed the invalid unit, recovered.
            return true;
        }

        return false; // unreachable
    }
};

/// Matches a UTF-16 code point.
struct engine_cp_utf16 : engine_matcher_base, engine_parser_base
{
    static constexpr auto payload1 = 0b0000'0011'1111'1111;
    static constexpr auto payload2 = payload1;

    static constexpr auto pattern1 = 0b110110 << 10;
    static constexpr auto pattern2 = 0b110111 << 10;

    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, utf16_encoding>);

        if (reader.eof())
        {
            ec = error_code::eof;
            return code_point();
        }

        auto first = char16_t(reader.peek());
        if ((first & ~payload1) == pattern1)
        {
            reader.bump();
            if (reader.eof())
            {
                ec = error_code::missing_trailing;
                return code_point();
            }

            auto second = char16_t(reader.peek());
            if ((second & ~payload2) != pattern2)
            {
                ec = error_code::missing_trailing;
                return code_point();
            }
            reader.bump();

            // We've got a valid code point.
            auto result = char32_t(first & payload1);
            result <<= 10;
            result |= char32_t(second & payload2);
            result |= 0x10000;
            return code_point(result);
        }
        else if ((first & ~payload2) == pattern2)
        {
            ec = error_code::leads_with_trailing;
            return code_point();
        }
        else
        {
            // Single code unit code point; always valid.
            reader.bump();
            return code_point(first);
        }
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::leads_with_trailing:
            // Invalid code unit, consume to recover.
            reader.bump();
            return true;
        case error_code::missing_trailing:
            // We've already consumed the invalid unit, recovered.
            return true;

        case error_code::surrogate:
        case error_code::overlong_sequence:
        case error_code::out_of_range:
            LEXY_ASSERT(false, "unreachable");
        }
        return false;
    }
};

/// Matches a UTF-32 code point.
struct engine_cp_utf32 : engine_matcher_base, engine_parser_base
{
    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        static_assert(std::is_same_v<typename Reader::encoding, utf32_encoding>);

        if (reader.eof())
        {
            ec = error_code::eof;
            return code_point();
        }

        auto cur = reader.peek();
        reader.bump();

        auto cp = code_point(cur);
        if (!cp.is_valid())
            ec = error_code::out_of_range;
        else if (cp.is_surrogate())
            ec = error_code::surrogate;

        return cp;
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};
        parse(result, reader);
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader&, error_code ec)
    {
        switch (ec)
        {
        case error_code::eof:
            // We cannot recover after EOF.
            return false;
        case error_code::surrogate:
        case error_code::out_of_range:
            // Invalid code unit, but already consumed.
            return true;

        case error_code::leads_with_trailing:
        case error_code::missing_trailing:
        case error_code::overlong_sequence:
            LEXY_ASSERT(false, "unreachable");
        }
        return false;
    }
};

/// Matches a code point according to the inputs encoding.
struct engine_cp_auto : lexy::engine_matcher_base, lexy::engine_parser_base
{
    using error_code = _cp_error_code;

    template <typename Reader>
    static constexpr code_point parse(error_code& ec, Reader& reader)
    {
        using encoding = typename Reader::encoding;
        if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
            return engine_cp_ascii::parse(ec, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
            return engine_cp_utf8::parse(ec, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
            return engine_cp_utf16::parse(ec, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
            return engine_cp_utf32::parse(ec, reader);
        else
            static_assert(lexy::_detail::error<encoding>,
                          "no code point defined for this encoding");
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        error_code result{};

        using encoding = typename Reader::encoding;
        if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
            engine_cp_ascii::parse(result, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
            engine_cp_utf8::parse(result, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
            engine_cp_utf16::parse(result, reader);
        else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
            engine_cp_utf32::parse(result, reader);
        else
            static_assert(lexy::_detail::error<encoding>,
                          "no code point defined for this encoding");

        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        using encoding = typename Reader::encoding;
        if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
            return engine_cp_ascii::recover(reader, ec);
        else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
            return engine_cp_utf8::recover(reader, ec);
        else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
            return engine_cp_utf16::recover(reader, ec);
        else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
            return engine_cp_utf32::recover(reader, ec);
        else
            static_assert(lexy::_detail::error<encoding>,
                          "no code point defined for this encoding");
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_CODE_POINT_HPP_INCLUDED


namespace lexyd
{
template <typename Encoding>
constexpr auto _cp_name()
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
        return "ASCII.code_point";
    else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
        return "UTF-8.code_point";
    else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
        return "UTF-16.code_point";
    else if constexpr (std::is_same_v<Encoding, lexy::utf32_encoding>)
        return "UTF-32.code_point";
    else
        return "code_point";
}

struct _cp_cap : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto save = reader;

            lexy::engine_cp_auto::error_code ec{};
            auto                             result = lexy::engine_cp_auto::parse(ec, reader);
            if (ec == lexy::engine_cp_auto::error_code())
            {
                LEXY_PRECONDITION(result.is_scalar());
                return NextParser::parse(context, reader, LEXY_FWD(args)..., result);
            }
            else
            {
                reader = LEXY_MOV(save);

                auto name = _cp_name<typename Reader::encoding>();
                auto e    = lexy::make_error<Reader, lexy::expected_char_class>(reader.cur(), name);
                context.error(e);
                return false;
            }
        }
    };
};

struct _cp : token_base<_cp>
{
    using token_engine = lexy::engine_cp_auto;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto name = _cp_name<typename Reader::encoding>();
        auto err  = lexy::make_error<Reader, lexy::expected_char_class>(pos, name);
        context.error(err);
    }

    LEXY_CONSTEVAL auto capture() const
    {
        return _cp_cap{};
    }
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp{};
} // namespace lexyd

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED





// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LABEL_HPP_INCLUDED
#define LEXY_DSL_LABEL_HPP_INCLUDED





namespace lexy
{
template <typename T, typename = void>
struct label
{};
template <typename T>
struct label<T, decltype(void(T::value))>
{
    LEXY_CONSTEVAL operator decltype(T::value)() const
    {
        return T::value;
    }
};

template <auto Id>
using id = label<std::integral_constant<int, Id>>;
} // namespace lexy

namespace lexyd
{
template <typename Label, typename Rule>
struct _labr;

template <typename Label>
struct _lab : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::label<Label>{});
        }
    };

    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule) const
    {
        return _labr<Label, Rule>{};
    }
};

template <typename Label, typename Rule>
struct _labr : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader,
                                                                  LEXY_FWD(args)...,
                                                                  lexy::label<Label>{});
        }
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...,
                                                              lexy::label<Label>{});
        }
    };
};

/// Matches with the specified label.
template <typename Label>
constexpr auto label = _lab<Label>{};

/// Matches with the specified id.
template <auto Id>
constexpr auto id = _lab<std::integral_constant<int, Id>>{};
} // namespace lexyd

#endif // LEXY_DSL_LABEL_HPP_INCLUDED




namespace lexy
{
struct combination_duplicate
{
    static LEXY_CONSTEVAL auto name()
    {
        return "combination duplicate";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Sink>
struct _comb_state
{
    // The sink to store values of the item.
    Sink& sink;
    // Write the index of the item in here.
    int idx = 0;
    // Whether or not we should break.
    bool loop_break = false;
};

// Final parser for one item in the combination.
struct _comb_it
{
    template <typename Context, typename Reader, int Idx, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader&, lexy::id<Idx>, Args&&... args)
    {
        auto& state = context.get(_break{});
        state.idx   = Idx;
        if constexpr (sizeof...(Args) > 0)
            state.sink(LEXY_FWD(args)...);
        return true;
    }
};

template <typename DuplicateError, typename ElseRule, typename... R>
struct _comb : rule_base
{
    template <std::size_t... Idx>
    static auto _comb_choice_(lexy::_detail::index_sequence<Idx...>)
    {
        if constexpr (std::is_void_v<ElseRule>)
            return (id<Idx>(R{}) | ...);
        else
            return (id<Idx>(R{}) | ... | ElseRule{});
    }
    using _comb_choice = decltype(_comb_choice_(lexy::_detail::index_sequence_for<R...>{}));

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto N = sizeof...(R);

            auto  sink         = context.sink();
            bool  handled[N]   = {};
            auto  comb_context = context.insert(_break{}, _comb_state<decltype(sink)>{sink});
            auto& state        = comb_context.get(_break{});

            // Parse all iterations of the choice.
            for (std::size_t count = 0; count < N; ++count)
            {
                auto begin = reader.cur();

                using parser = lexy::rule_parser<_comb_choice, _comb_it>;
                if (!parser::parse(comb_context, reader))
                    return false;
                else if (state.loop_break)
                    break; // Partial combination and we're done.

                if (handled[state.idx])
                {
                    using tag = std::conditional_t<std::is_void_v<DuplicateError>,
                                                   lexy::combination_duplicate, DuplicateError>;
                    auto err  = lexy::make_error<Reader, tag>(begin, reader.cur());
                    context.error(err);
                    return false;
                }
                else
                {
                    handled[state.idx] = true;
                }
            }

            // Obtain the final result and continue.
            if constexpr (std::is_void_v<typename decltype(sink)::return_type>)
            {
                LEXY_MOV(sink).finish();
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         LEXY_MOV(sink).finish());
            }
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _comb<Tag, ElseRule, R...> duplicate_error = {};

    template <typename Tag>
    static constexpr _comb<DuplicateError, _err<Tag, void>, R...> missing_error = {};
};

/// Matches each of the rules in an arbitrary order.
/// Only matches each rule exactly once.
template <typename... R>
LEXY_CONSTEVAL auto combination(R...)
{
    static_assert((lexy::is_branch<R> && ...), "combination() requires a branch rule");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
LEXY_CONSTEVAL auto partial_combination(R...)
{
    static_assert((lexy::is_branch<R> && ...), "partial_combination() requires a branch rule");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
}

template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `combination<Tag>(r...)` by `combination(r...).duplicate_error<Tag>`")
LEXY_CONSTEVAL auto combination(R... r)
{
    return combination(r...).template duplicate_error<Tag>;
}
template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `partial_combination<Tag>(r...)` by `partial_combination(r...).duplicate_error<Tag>`")
LEXY_CONSTEVAL auto partial_combination(R... r)
{
    return partial_combination(r...).template duplicate_error<Tag>;
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_HPP_INCLUDED





#ifdef LEXY_IGNORE_DEPRECATED_CONTEXT
#    define LEXY_DEPRECATED_CONTEXT
#else
#    define LEXY_DEPRECATED_CONTEXT                                                                \
        [[deprecated("old context_* has been replaced by `dsl::context_lexeme`")]]
#endif

namespace lexyd
{
template <template <typename Reader> typename Lexeme, typename NextParser, typename... PrevArgs>
struct _cap_cont
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args,
                             typename Reader::iterator begin, Args&&... args)
    {
        auto end = reader.cur();
        return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                 Lexeme<typename Reader::canonical_reader>(begin, end),
                                 LEXY_FWD(args)...);
    }
};

template <template <typename Reader> typename Lexeme, typename Rule, typename NextParser>
struct _cap_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        using continuation = _cap_cont<Lexeme, NextParser, Args...>;
        return lexy::rule_parser<Rule, continuation>::parse(context, reader, LEXY_FWD(args)...,
                                                            reader.cur());
    }
};

template <typename Reader>
struct _context
{
    lexy::lexeme<Reader> lexeme;

    constexpr _context() noexcept = default;
    constexpr _context(typename Reader::iterator begin, typename Reader::iterator end) noexcept
    : lexeme(begin, end)
    {}

    template <typename U>
    static constexpr bool is = std::is_same_v<std::decay_t<U>, _context>;

    // Returns the last context argument.
    template <typename... Args>
    static constexpr auto get(Args&&... args)
    {
        _context<Reader> result;
        // We're immediately invoking a lambda that sets result if the type matches for each
        // argument.
        auto lambda = [&](auto&& arg) {
            if constexpr (is<decltype(arg)>)
                result = arg;
        };
        (lambda(args), ...);
        return result;
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _ctx_push : rule_base
{
    template <typename NextParser>
    using parser = _cap_parser<_context, Rule, NextParser>;
};

/// Pushes whatever the rule captures onto the context stack.
template <typename Rule>
LEXY_DEPRECATED_CONTEXT LEXY_CONSTEVAL auto context_push(Rule)
{
    return _ctx_push<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
struct _ctx_drop : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Context, typename Reader, typename Head, typename... Tail>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args, Head&& head,
                                     Tail&&... tail)
            {
                using context_t                = _context<typename Reader::canonical_reader>;
                constexpr auto head_is_context = context_t::template is<Head>;
                constexpr auto tail_is_context = (context_t::template is<Tail> || ...);

                if constexpr (head_is_context && !tail_is_context)
                {
                    // Head is a context argument and it is the last one; remove it and we're done.
                    return NextParser::parse(context, reader, LEXY_FWD(args)..., LEXY_FWD(tail)...);
                }
                else
                {
                    // Either Head is a context, but there is a later one, or Head isn't a context.
                    // In either case, we're keeping Head.
                    static_assert(sizeof...(Tail) > 0, "missing previous context_push()");
                    return _continuation<Args..., Head>::parse(context, reader, LEXY_FWD(args)...,
                                                               LEXY_FWD(head), LEXY_FWD(tail)...);
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Initially no argument is kept.
            return _continuation<>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Removes the last pushed context from the stack.
constexpr auto context_drop = _ctx_drop{};
} // namespace lexyd

namespace lexy
{
/// Error when the previously pushed context does not match the popped one.
struct context_mismatch
{
    static LEXY_CONSTEVAL auto name()
    {
        return "context mismatch";
    }
};
} // namespace lexy

namespace lexyd
{
/// Checks that the pushed and popped context are exactly equal.
struct context_eq
{
    template <typename Reader>
    constexpr bool operator()(lexy::lexeme<Reader> lhs, lexy::lexeme<Reader> rhs) const
    {
        return lexy::_detail::equal_lexemes(lhs, rhs);
    }
};

/// Checks that the pushed and popped contexts have the same length.
struct context_eq_length
{
    template <typename Reader>
    constexpr bool operator()(lexy::lexeme<Reader> lhs, lexy::lexeme<Reader> rhs) const
    {
        return lexy::_detail::range_size(lhs.begin(), lhs.end())
               == lexy::_detail::range_size(rhs.begin(), rhs.end());
    }
};

template <typename Rule, typename Eq, typename Error>
struct _ctx_top
{
    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            // We're ignoring any values created by the rule.
            //
            template <typename Context, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args,
                                     const _context<typename Reader::canonical_reader>& popped,
                                     RuleArgs&&...)
            {
                auto pushed = _context<typename Reader::canonical_reader>::get(LEXY_FWD(args)...);
                if (Eq{}(pushed.lexeme, popped.lexeme))
                    // We've parsed the same argument that we've pushed, continue.
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                {
                    auto err = lexy::make_error<Reader, Error>(popped.lexeme.begin(),
                                                               popped.lexeme.end());
                    context.error(err);
                    return false;
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // We capture the rule and pass it to the continuation to process.
            return _cap_parser<_context, Rule, _continuation<Args...>>::parse(context, reader,
                                                                              LEXY_FWD(args)...);
        }
    };

    /// Sets the error if the context doesn't match.
    template <typename E>
    LEXY_CONSTEVAL auto error()
    {
        return _ctx_top<Rule, Eq, E>{};
    }
};

/// Captures what the Rule matches and checks that it is equal to the last context pushed onto the
/// stack.
/// The context is kept on the stack.
template <typename Eq = context_eq, typename Rule>
LEXY_DEPRECATED_CONTEXT LEXY_CONSTEVAL auto context_top(Rule)
{
    return _ctx_top<Rule, Eq, lexy::context_mismatch>{};
}

template <typename Rule, typename Eq, typename Error>
struct _ctx_pop : decltype(_ctx_top<Rule, Eq, Error>{} + context_drop)
{
    /// Sets the error if the context doesn't match.
    template <typename E>
    LEXY_CONSTEVAL auto error()
    {
        return _ctx_pop<Rule, Eq, E>{};
    }
};

/// Captures what the Rule matches and checks that it is equal to the last context pushed onto the
/// stack.
/// The context is removed on the stack.
template <typename Eq = context_eq, typename Rule>
LEXY_DEPRECATED_CONTEXT LEXY_CONSTEVAL auto context_pop(Rule)
{
    return _ctx_pop<Rule, Eq, lexy::context_mismatch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED




namespace lexyd
{
template <typename Id, int InitialValue>
struct _ctx_ccreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Add the counter to the context.
            auto counter_ctx = context.insert(Id{}, InitialValue);
            return NextParser::parse(counter_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Delta>
struct _ctx_cadd : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Add the flag to the context.
            context.get(Id{}) += Delta;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Rule, int Sign>
struct _ctx_cpush : rule_base
{
    template <typename NextParser>
    struct parser
    {
        struct _cont
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args)
            {
                auto end    = reader.cur();
                auto length = lexy::_detail::range_size(begin, end);

                context.get(Id{}) += int(length) * Sign;

                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value, typename R, typename S, typename T>
struct _ctx_ccompare : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (context.get(Id{}) < Value)
                return lexy::rule_parser<R, NextParser>::parse(context, reader, LEXY_FWD(args)...);
            else if (context.get(Id{}) == Value)
                return lexy::rule_parser<S, NextParser>::parse(context, reader, LEXY_FWD(args)...);
            else // context.get(Id{}) > Value
                return lexy::rule_parser<T, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Tag, int Value>
struct _ctx_crequire : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (context.get(Id{}) == Value)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                context.error(err);
                return false;
            }
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, int Value>
struct _ctx_counter_require
{
    template <typename Tag>
    static constexpr _ctx_crequire<Id, Tag, Value> error = {};
};

template <typename Id>
struct _ctx_counter
{
    struct id
    {};

    template <int InitialValue = 0>
    LEXY_CONSTEVAL auto create() const
    {
        return _ctx_ccreate<id, InitialValue>{};
    }

    LEXY_CONSTEVAL auto inc() const
    {
        return _ctx_cadd<id, +1>{};
    }
    LEXY_CONSTEVAL auto dec() const
    {
        return _ctx_cadd<id, -1>{};
    }

    template <typename Rule>
    LEXY_CONSTEVAL auto push(Rule) const
    {
        return _ctx_cpush<id, Rule, +1>{};
    }
    template <typename Rule>
    LEXY_CONSTEVAL auto pop(Rule) const
    {
        return _ctx_cpush<id, Rule, -1>{};
    }

    template <int Value, typename R, typename S, typename T>
    LEXY_CONSTEVAL auto compare(R, S, T) const
    {
        return _ctx_ccompare<id, Value, R, S, T>{};
    }

    template <int Value = 0>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_counter_require<id, Value>{};
    }

    template <typename Tag>
    LEXY_DEPRECATED_ERROR("replace `counter.require<Tag>()` by `counter.require().error<Tag>`")
    LEXY_CONSTEVAL auto require() const
    {
        return require().template error<Tag>;
    }
    template <int Value, typename Tag>
    LEXY_DEPRECATED_ERROR(
        "replace `counter.require<Value, Tag>()` by `counter.require<Value>().error<Tag>`")
    LEXY_CONSTEVAL auto require() const
    {
        return require<Value>().template error<Tag>;
    }
};

/// Declares an integer counter that is added to the parsing context.
template <typename Id>
constexpr auto context_counter = _ctx_counter<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED




namespace lexyd
{
template <typename Id, bool InitialValue>
struct _ctx_fcreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Add the flag to the context.
            auto flag_ctx = context.insert(Id{}, InitialValue);
            return NextParser::parse(flag_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fset : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.get(Id{}) = Value;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id>
struct _ctx_ftoggle : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.get(Id{}) = !context.get(Id{});
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename R, typename S>
struct _ctx_fselect : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (context.get(Id{}))
                return lexy::rule_parser<R, NextParser>::parse(context, reader, LEXY_FWD(args)...);
            else
                return lexy::rule_parser<S, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Tag, bool Value>
struct _ctx_frequire : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (context.get(Id{}) == Value)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                context.error(err);
                return false;
            }
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, bool Value>
struct _ctx_flag_require
{
    template <typename Tag>
    static constexpr _ctx_frequire<Id, Tag, Value> error = {};
};

template <typename Id>
struct _ctx_flag
{
    struct id
    {};

    template <bool InitialValue = false>
    LEXY_CONSTEVAL auto create() const
    {
        return _ctx_fcreate<id, InitialValue>{};
    }

    LEXY_CONSTEVAL auto set() const
    {
        return _ctx_fset<id, true>{};
    }
    LEXY_CONSTEVAL auto reset() const
    {
        return _ctx_fset<id, false>{};
    }

    LEXY_CONSTEVAL auto toggle() const
    {
        return _ctx_ftoggle<id>{};
    }

    template <typename R, typename S>
    LEXY_CONSTEVAL auto select(R, S) const
    {
        return _ctx_fselect<id, R, S>{};
    }

    template <bool Value = true>
    LEXY_CONSTEVAL auto require() const
    {
        return _ctx_flag_require<id, Value>{};
    }

    template <typename Tag>
    LEXY_DEPRECATED_ERROR("replace `flag.require<Tag>()` by `flag.require().error<Tag>`")
    LEXY_CONSTEVAL auto require() const
    {
        return require().template error<Tag>;
    }
    template <bool Value, typename Tag>
    LEXY_DEPRECATED_ERROR(
        "replace `flag.require<false, Tag>()` by `flag.require<false>().error<Tag>`")
    LEXY_CONSTEVAL auto require() const
    {
        return require<Value>().template error<Tag>;
    }
};

/// Declares a flag.
template <typename Id>
constexpr auto context_flag = _ctx_flag<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_LEXEME_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_LEXEME_HPP_INCLUDED





namespace lexyd
{
template <typename Id>
struct _ctx_lcreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Add the lexeme to the context.
            auto lex_ctx = context.insert(Id{}, lexy::lexeme<Reader>());
            return NextParser::parse(lex_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Rule>
struct _ctx_lcapture : rule_base
{
    template <typename NextParser>
    struct parser
    {
        struct _cont
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args)
            {
                context.get(Id{}) = lexy::lexeme(reader, begin);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Tag, typename Rule>
struct _ctx_lrequire : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _cont
        {
            template <typename Context, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args, RuleArgs&&...)
            {
                auto lhs = context.get(Id{});
                auto rhs = lexy::lexeme(reader, begin);

                if (lexy::_detail::equal_lexemes(lhs, rhs))
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                {
                    auto err = lexy::make_error<Reader, Tag>(rhs.begin(), rhs.end());
                    context.error(err);
                    return false;
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont<Args...>>::parse(context, reader, reader.cur(),
                                                                  LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, typename Rule>
struct _ctx_lexeme_require
{
    template <typename Tag>
    static constexpr _ctx_lrequire<Id, Tag, Rule> error = {};
};

template <typename Id>
struct _ctx_lexeme
{
    struct id
    {};

    LEXY_CONSTEVAL auto create() const
    {
        return _ctx_lcreate<id>{};
    }

    template <typename Rule>
    LEXY_CONSTEVAL auto capture(Rule) const
    {
        return _ctx_lcapture<id, Rule>{};
    }

    template <typename Rule>
    LEXY_CONSTEVAL auto require(Rule) const
    {
        return _ctx_lexeme_require<id, Rule>{};
    }

    template <typename Tag, typename Rule>
    LEXY_DEPRECATED_ERROR(
        "replace `lexeme.require<tag>(rule)` by `lexeme.require(rule).error<tag>`")
    LEXY_CONSTEVAL auto require(Rule rule) const
    {
        return require(rule).template error<Tag>;
    }
};

/// Declares a lexeme.
template <typename Id>
constexpr auto context_lexeme = _ctx_lexeme<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_LEXEME_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED






// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_IF_HPP_INCLUDED
#define LEXY_DSL_IF_HPP_INCLUDED




namespace lexyd
{
template <typename Branch>
struct _if : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using branch_parser = lexy::rule_parser<Branch, NextParser>;

            auto result = branch_parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Branch wasn't taken, continue anyway.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                // Return true/false depending on result.
                return static_cast<bool>(result);
        }
    };
};

/// If the branch condition matches, matches the branch then.
template <typename Branch>
LEXY_CONSTEVAL auto if_(Branch)
{
    static_assert(lexy::is_branch<Branch>, "if_() requires a branch condition");
    if constexpr (Branch::is_unconditional_branch)
        // Branch is always taken, so don't wrap in if_().
        return Branch{};
    else
        return _if<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_IF_HPP_INCLUDED



#ifdef LEXY_IGNORE_DEPRECATED_SEP
#    define LEXY_DEPRECATED_SEP
#else
#    define LEXY_DEPRECATED_SEP                                                                    \
        [[deprecated("no_trailing_sep() has been deprecated; use sep() instead")]]
#endif

namespace lexy
{
struct unexpected_trailing_separator
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected trailing separator";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Branch, typename Tag>
struct _sep
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{} >> lexyd::error<Tag>));

    template <typename Context, typename Reader>
    LEXY_DSL_FUNC void report_trailing_error(Context&                  context, Reader&,
                                             typename Reader::iterator sep_pos)
    {
        auto err = lexy::make_error<Reader, Tag>(sep_pos);
        context.error(err);
    }

    //=== dsl ===//
    template <typename NewTag>
    static constexpr _sep<Branch, NewTag> trailing_error = {};
};

/// Defines a separator for a list.
template <typename Branch>
LEXY_CONSTEVAL auto sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _sep<Branch, lexy::unexpected_trailing_separator>{};
}

template <typename Branch>
struct _tsep
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{}));

    template <typename Context, typename Reader>
    LEXY_DSL_FUNC void report_trailing_error(Context&, Reader&, typename Reader::iterator)
    {}
};

/// Defines a separator for a list that can be trailing.
template <typename Branch>
LEXY_CONSTEVAL auto trailing_sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _tsep<Branch>{};
}

template <typename Branch>
LEXY_DEPRECATED_SEP LEXY_CONSTEVAL auto no_trailing_sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _sep<Branch, lexy::unexpected_trailing_separator>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED


namespace lexyd
{
// Final parser for the list.
// It will create the final value.
template <typename NextParser, typename... PrevArgs>
struct _list_finish
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args, Sink& sink,
                             Args&&... args)
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return NextParser::parse(context, reader, LEXY_FWD(prev_args)..., LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                     LEXY_MOV(sink).finish(), LEXY_FWD(args)...);
        }
    }
};

// Final parser for a list item.
// It will forward the arguments to the sink.
struct _list_sink
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC bool parse(Context&, Reader&, Sink& sink, Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0)
            sink(LEXY_FWD(args)...);
        return true;
    }
};

// Loop to parse all remaining items (after the initial one).
template <typename Item, typename Sep, typename NextParser, typename... PrevArgs>
struct _list_loop
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the separator.
            auto sep_pos     = reader.cur();
            using sep_parser = lexy::rule_parser<typename Sep::rule, _list_sink>;
            if (auto result = sep_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;

            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if constexpr (Item::is_branch)
            {
                // Try parsing the next item.
                if (auto result = item_parser::try_parse(context, reader, sink);
                    result == lexy::rule_try_parse_result::backtracked)
                {
                    // If trailing seperators are allowed, this does nothing.
                    // Otherwise, we report the error but can trivially recover.
                    Sep::report_trailing_error(context, reader, sep_pos);
                    break;
                }
                else if (result == lexy::rule_try_parse_result::canceled)
                    return false;
            }
            else
            {
                (void)sep_pos;
                // If the item isn't a branch, we parse it unconditionally.
                // This only happens with `sep()`.
                if (!item_parser::parse(context, reader, sink))
                    return false;
            }
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};
template <typename Item, typename NextParser, typename... PrevArgs>
struct _list_loop<Item, void, NextParser, PrevArgs...>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        while (true)
        {
            // Try parsing the item.
            using item_parser = lexy::rule_parser<Item, _list_sink>;
            if (auto result = item_parser::try_parse(context, reader, sink);
                result == lexy::rule_try_parse_result::backtracked)
                break;
            else if (result == lexy::rule_try_parse_result::canceled)
                return false;
        }

        return _list_finish<NextParser, PrevArgs...>::parse(context, reader, LEXY_FWD(args)...,
                                                            sink);
    }
};

// Workaround for MSVC, which can't handle typedefs inside if constexpr.
template <typename Sep>
struct _sep_parser : lexy::rule_parser<typename Sep::rule, _list_sink>
{};
template <>
struct _sep_parser<void> : _list_sink
{};

// Loop to parse all remaining list items when we have a terminator.
template <typename Term, typename Item, typename Sep, typename RecoveryLimit, typename NextParser,
          typename... PrevArgs>
struct _list_loop_term
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... args, Sink& sink)
    {
        // We're using an enum together with a switch to compensate a lack of goto in constexpr.
        // The simple state machine goes as follows on well-formed input:
        // terminator -> separator -> separator_trailing_check -> item -> terminator
        //
        // The interesting case is error recovery.
        // There we skip over characters until we either found the terminator, separator or item.
        // We then set the enum to jump to the appropriate state of the state machine.
        enum class state
        {
            terminator,
            separator,
            separator_trailing_check,
            item,
            recovery,
        } state
            = state::terminator;

        auto sep_pos      = reader.cur();
        using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
        using item_parser = lexy::rule_parser<Item, _list_sink>;
        using sep_parser  = _sep_parser<Sep>;
        while (true)
        {
            switch (state)
            {
            case state::terminator:
                if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had the terminator, so the list is done.
                    return static_cast<bool>(result);
                }
                // Parse the following list separator next.
                state = state::separator;
                break;

            case state::separator:
                if constexpr (!std::is_void_v<Sep>)
                {
                    sep_pos = reader.cur();

                    if (sep_parser::parse(context, reader, sink))
                    {
                        // Check for a trailing separator next.
                        state = state::separator_trailing_check;
                        break;
                    }
                    else if (reader.cur() == sep_pos)
                    {
                        // We didn't have a separator at all.
                        // Check whether we have an item instead (if that's possible).
                        if constexpr (Item::is_branch)
                        {
                            if (auto result = item_parser::try_parse(context, reader, sink);
                                result == lexy::rule_try_parse_result::ok)
                            {
                                // It was just a missing separator, continue with the normal
                                // terminator check after parsing an item.
                                state = state::terminator;
                                break;
                            }
                            else
                            {
                                // It is a mistyped separator, not just a missing one.
                                // Enter generic recovery.
                                state = state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            // We can't check whether we have an item.
                            // This means we can't distinguish between "missing separator" and
                            // "mistyped separator". So let's just pretend the separator is missing
                            // and parse an item.
                            state = state::item;
                            break;
                        }
                    }
                    else
                    {
                        // We did have something that looked like a separator initially, but wasn't
                        // one on closer inspection. Enter generic recovery as we've already
                        // consumed input. (If we ignore the case where the item and separator share
                        // a common prefix, we know it wasn't the start of an item so can't just
                        // pretend that there is one).
                        state = state::recovery;
                        break;
                    }
                }
                else
                {
                    // No separator, immediately parse item next.
                    (void)sep_pos;
                    state = state::item;
                    break;
                }
                break;
            case state::separator_trailing_check:
                if constexpr (!std::is_void_v<Sep>)
                {
                    if (auto result
                        = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                        result != lexy::rule_try_parse_result::backtracked)
                    {
                        // If trailing seperators are allowed, this does nothing.
                        // Otherwise, we report the error but can trivially recover.
                        Sep::report_trailing_error(context, reader, sep_pos);
                        // We had the terminator, so the list is done.
                        return static_cast<bool>(result);
                    }

                    // Now parse the item next.
                    state = state::item;
                }
                break;

            case state::item:
                if (item_parser::parse(context, reader, sink))
                    // Loop back and try again for the next item.
                    state = state::terminator;
                else
                    // Recover the error.
                    state = state::recovery;
                break;

            case state::recovery:
                while (true)
                {
                    // Recovery succeeds when we reach the next separator.
                    if constexpr (!std::is_void_v<Sep>)
                    {
                        sep_pos = reader.cur();

                        if (auto result = sep_parser::try_parse(context, reader, sink);
                            result == lexy::rule_try_parse_result::ok)
                        {
                            // Continue the list with the trailing separator check.
                            state = state::separator_trailing_check;
                            break;
                        }

                        // Here we either try something else or start recovering from a failed
                        // separator.
                    }
                    // When we don't have a separator, but the item is a branch, we also succeed
                    // when we reach the next item.
                    //
                    // Note that we're doing this check only if we don't have a separator.
                    // If we do have one, the heuristic "end of the invalid item" is better than
                    // "beginning of the next one".
                    else if constexpr (Item::is_branch)
                    {
                        if (auto result = item_parser::try_parse(context, reader, sink);
                            result == lexy::rule_try_parse_result::ok)
                        {
                            // Continue the list with the next terminator check.
                            state = state::terminator;
                            break;
                        }

                        // Here we either try something else or start recovering from yet another
                        // failed item.
                    }

                    // Recovery succeeds when we reach the terminator.
                    if (auto result
                        = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                        result != lexy::rule_try_parse_result::backtracked)
                    {
                        // We're now done with the entire list.
                        return static_cast<bool>(result);
                    }

                    // Recovery fails when we reach the limit.
                    using limit = typename decltype(RecoveryLimit{}.get_limit())::token_engine;
                    if (lexy::engine_peek<limit>(reader))
                        return false;

                    // Consume one character and try again.
                    reader.bump();
                }
                break;
            }
        }

        return false; // unreachable
    }
};
} // namespace lexyd

namespace lexyd
{
template <typename Item, typename Sep>
struct _lst : rule_base
{
    static constexpr auto is_branch               = Item::is_branch;
    static constexpr auto is_unconditional_branch = Item::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto sink = context.sink();

            // Try parsing the initial item.
            using item_parser = lexy::rule_parser<Item, _list_sink>;
            if (auto result = item_parser::try_parse(context, reader, sink);
                result != lexy::rule_try_parse_result::ok)
                // Either we backtracked the item, in which case we also backtrack.
                // Or we failed parsing the item, in which case we also fail.
                return result;

            // Continue with the rest of the items.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., sink));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Parse the initial item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;

            // Continue with the rest of the items.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };
};

/// Parses a list of items without a separator.
template <typename Item>
LEXY_CONSTEVAL auto list(Item)
{
    static_assert(lexy::is_branch<Item>, "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Parses a list of items with the specified separator.
template <typename Item, typename Sep, typename Tag>
LEXY_CONSTEVAL auto list(Item, _sep<Sep, Tag>)
{
    return _lst<Item, _sep<Sep, Tag>>{};
}

/// Parses a list of items with the specified separator that can be trailing.
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto list(Item, _tsep<Sep>)
{
    static_assert(lexy::is_branch<Item>,
                  "list() without a trailing separator requires a branch condition");
    return _lst<Item, _tsep<Sep>>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Item, typename Sep>
struct _olst : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using list_parser = lexy::rule_parser<_lst<Item, Sep>, NextParser>;

            // Try parsing the list.
            if (auto result = list_parser::try_parse(context, reader, LEXY_FWD(args)...);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We didn't backtrack, so its result is our result.
                return static_cast<bool>(result);
            }
            else
            {
                // We don't have a list: construct a sink and immediately finish it.
                auto sink = context.sink();
                return _list_finish<NextParser, Args...>::parse(context, reader, LEXY_FWD(args)...,
                                                                sink);
            }
        }
    };
};

/// Parses a list that might be empty.
template <typename Item>
LEXY_CONSTEVAL auto opt_list(Item)
{
    static_assert(lexy::is_branch<Item>, "opt_list() requires a branch condition");
    return _olst<Item, void>{};
}
template <typename Item, typename Sep>
LEXY_CONSTEVAL auto opt_list(Item, Sep)
{
    static_assert(lexy::is_branch<Item>, "opt_list() requires a branch condition");
    return _olst<Item, Sep>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Item, typename Sep, typename Recover>
struct _lstt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Parse initial item.
            using item_parser = typename lexy::rule_parser<Item, _list_sink>;
            if (!item_parser::parse(context, reader, sink))
                return false;

            // Parse the remaining items.
            using continuation = _list_loop_term<Term, Item, Sep, Recover, NextParser, Args...>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
        }
    };
};

template <typename Term, typename Item, typename Sep, typename Recover>
struct _olstt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.sink();

            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, Args...>>;
            if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, and thus created an empty list.
                return static_cast<bool>(result);
            }
            else
            {
                // Parse initial item.
                using item_parser = typename lexy::rule_parser<Item, _list_sink>;
                if (!item_parser::parse(context, reader, sink))
                    return false;

                // Parse the remaining items.
                using continuation = _list_loop_term<Term, Item, Sep, Recover, NextParser, Args...>;
                return continuation::parse(context, reader, LEXY_FWD(args)..., sink);
            }
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SYMBOL_HPP_INCLUDED
#define LEXY_DSL_SYMBOL_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_ITERATOR_HPP_INCLUDED
#define LEXY_DETAIL_ITERATOR_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_STD_HPP_INCLUDED
#define LEXY_DETAIL_STD_HPP_INCLUDED



#if defined(__GLIBCXX__)

namespace std
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#elif defined(_LIBCPP_VERSION)

_LIBCPP_BEGIN_NAMESPACE_STD
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
_LIBCPP_END_NAMESPACE_STD

#else

// Forward declaring things in std is not allowed, but I'm willing to take the risk.

namespace std
{
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
} // namespace std

#endif

#endif // LEXY_DETAIL_STD_HPP_INCLUDED


namespace lexy::_detail
{
template <typename T>
struct _proxy_pointer
{
    T value;

    constexpr T* operator->() noexcept
    {
        return &value;
    }
};

template <typename Derived, typename T, typename Reference = T&, typename Pointer = const T*>
struct forward_iterator_base
{
    using value_type = std::remove_cv_t<T>;
    using reference  = Reference;
    using pointer
        = std::conditional_t<std::is_void_v<Pointer>, _proxy_pointer<value_type>, Pointer>;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    constexpr reference operator*() const noexcept
    {
        return static_cast<const Derived&>(*this).deref();
    }
    constexpr pointer operator->() const noexcept
    {
        if constexpr (std::is_void_v<Pointer>)
            return pointer{**this};
        else
            return &**this;
    }

    constexpr Derived& operator++() noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        derived.increment();
        return derived;
    }
    constexpr Derived operator++(int) noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        auto  copy    = derived;
        derived.increment();
        return copy;
    }

    friend constexpr bool operator==(const Derived& lhs, const Derived& rhs)
    {
        return lhs.equal(rhs);
    }
    friend constexpr bool operator!=(const Derived& lhs, const Derived& rhs)
    {
        return !lhs.equal(rhs);
    }
};

template <typename Derived, typename T, typename Reference = T&, typename Pointer = const T*>
struct bidirectional_iterator_base : forward_iterator_base<Derived, T, Reference, Pointer>
{
    using iterator_category = std::bidirectional_iterator_tag;

    constexpr Derived& operator--() noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        derived.decrement();
        return derived;
    }
    constexpr Derived operator--(int) noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        auto  copy    = derived;
        derived.decrement();
        return copy;
    }
};

template <typename Derived, typename Iterator>
struct sentinel_base
{
    friend constexpr bool operator==(const Iterator& lhs, Derived) noexcept
    {
        return lhs.is_end();
    }
    friend constexpr bool operator!=(const Iterator& lhs, Derived) noexcept
    {
        return !(lhs == Derived{});
    }
    friend constexpr bool operator==(Derived, const Iterator& rhs) noexcept
    {
        return rhs == Derived{};
    }
    friend constexpr bool operator!=(Derived, const Iterator& rhs) noexcept
    {
        return !(rhs == Derived{});
    }
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ITERATOR_HPP_INCLUDED








namespace lexy
{
#define LEXY_SYMBOL(Str) LEXY_NTTP_STRING(Str)

template <typename T, typename... Strings>
class _symbol_table
{
    static auto _char_type()
    {
        if constexpr (sizeof...(Strings) == 0)
            return;
        else
            return std::common_type_t<typename Strings::char_type...>{};
    }

public:
    using char_type   = decltype(_char_type());
    using key_type    = char_type;
    using mapped_type = T;

    struct value_type
    {
        const char_type*   symbol;
        const mapped_type& value;
    };

    //=== modifiers ===//
    LEXY_CONSTEVAL _symbol_table() : _data{} {}

    template <typename SymbolString, typename... Args>
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        using next_table = _symbol_table<T, Strings..., SymbolString>;
        if constexpr (empty())
            return next_table(lexy::_detail::make_index_sequence<0>{}, nullptr, LEXY_FWD(args)...);
        else
            return next_table(lexy::_detail::make_index_sequence<size()>{}, _data,
                              LEXY_FWD(args)...);
    }

#if LEXY_HAS_NTTP
    template <lexy::_detail::string_literal SymbolString, typename... Args>
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        return map<lexy::_detail::type_string<SymbolString>>(LEXY_FWD(args)...);
    }
#else
#    if (defined(__clang__) && __clang_major__ <= 7)                                               \
        || (defined(__clang__) && defined(__apple_build_version__) && __clang_major__ <= 10)
    template <char C, typename... Args> // Sorry, compiler bug.
#    else
    template <auto C, typename... Args>
#    endif
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        return map<lexy::_detail::type_char<C>>(LEXY_FWD(args)...);
    }
#endif

    //=== access ===//
    static constexpr bool empty() noexcept
    {
        return size() == 0;
    }

    static constexpr std::size_t size() noexcept
    {
        return sizeof...(Strings);
    }

    class iterator
    : public lexy::_detail::bidirectional_iterator_base<iterator, value_type, value_type, void>
    {
    public:
        constexpr iterator() noexcept : _table(nullptr), _idx(0) {}

        constexpr value_type deref() const noexcept
        {
            if constexpr (empty())
            {
                LEXY_PRECONDITION(false);
                return value_type{"", LEXY_DECLVAL(T)};
            }
            else
            {
                LEXY_PRECONDITION(_table);
                constexpr const char_type* strings[] = {Strings::get().c_str()...};
                return value_type{strings[_idx], _table->_data[_idx]};
            }
        }

        constexpr void increment() noexcept
        {
            LEXY_PRECONDITION(_idx != sizeof...(Strings));
            ++_idx;
        }
        constexpr void decrement() noexcept
        {
            LEXY_PRECONDITION(_idx != 0);
            --_idx;
        }

        constexpr bool equal(iterator rhs) const noexcept
        {
            LEXY_PRECONDITION(_table == rhs._table);
            return _idx == rhs._idx;
        }

    private:
        constexpr iterator(const _symbol_table* table, std::size_t idx) noexcept
        : _table(table), _idx(idx)
        {}

        const _symbol_table* _table;
        std::size_t          _idx;

        friend _symbol_table;
    };

    constexpr iterator begin() const noexcept
    {
        return iterator(this, 0);
    }
    constexpr iterator end() const noexcept
    {
        return iterator(this, size());
    }

    struct key_index
    {
        std::size_t _value;

        constexpr key_index() noexcept : _value(std::size_t(-1)) {}
        constexpr explicit key_index(std::size_t idx) noexcept : _value(idx)
        {
            LEXY_PRECONDITION(_value < size());
        }

        constexpr explicit operator bool() const noexcept
        {
            return _value < size();
        }

        friend constexpr bool operator==(key_index lhs, key_index rhs) noexcept
        {
            return lhs._value == rhs._value;
        }
        friend constexpr bool operator!=(key_index lhs, key_index rhs) noexcept
        {
            return lhs._value != rhs._value;
        }
    };

    template <typename Reader>
    constexpr key_index try_parse(Reader& reader) const
    {
        static_assert(!empty(), "symbol table must not be empty");
        using engine = lexy::engine_trie<_lazy::trie>;

        typename engine::error_code ec{};
        auto                        idx = engine::parse(ec, reader);
        if (ec == typename engine::error_code())
            return key_index(idx);
        else
            return key_index();
    }

    constexpr const T& operator[](key_index idx) const noexcept
    {
        LEXY_PRECONDITION(idx);
        return _data[idx._value];
    }

private:
    struct _lazy
    {
        static constexpr auto trie = lexy::trie<char_type, Strings...>;
    };

    template <std::size_t... Idx, typename... Args>
    constexpr explicit _symbol_table(lexy::_detail::index_sequence<Idx...>, const T* data,
                                     Args&&... args)
    // New data is appended at the end.
    : _data{data[Idx]..., T(LEXY_FWD(args)...)}
    {}

    std::conditional_t<empty(), char, T> _data[empty() ? 1 : size()];

    template <typename, typename...>
    friend class _symbol_table;
};

template <typename T>
constexpr auto symbol_table = _symbol_table<T>{};
} // namespace lexy

namespace lexy
{
struct unknown_symbol
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unknown symbol";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Leading, typename Trailing>
struct _idp;
template <typename Leading, typename Trailing, typename... Reserved>
struct _id;

template <const auto& Table, typename Token, typename Tag>
struct _sym : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Reader save,
                                         Args&&... args) -> lexy::rule_try_parse_result
            {
                auto rule_content = lexy::partial_reader(save, reader.cur());

                // We now re-parse what the rule has consumed.
                auto idx = Table.try_parse(rule_content);
                if (!idx || !rule_content.eof())
                {
                    // Unknown symbol; backtrack.
                    reader = LEXY_MOV(save);
                    return lexy::rule_try_parse_result::backtracked;
                }

                // Succesfully parsed a symbol, produce value and continue.
                return static_cast<lexy::rule_try_parse_result>(
                    NextParser::parse(context, reader, LEXY_FWD(args)..., Table[idx]));
            }

            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&... args)
            {
                auto begin  = save.cur();
                auto end    = reader.cur();
                auto result = try_parse(context, reader, save, LEXY_FWD(args)...);
                if (result == lexy::rule_try_parse_result::backtracked)
                {
                    // Handle the error.
                    using tag = std::conditional_t<std::is_void_v<Tag>, lexy::unknown_symbol, Tag>;
                    auto err  = lexy::make_error<Reader, tag>(begin, end);
                    context.error(err);
                    return false;
                }
                else
                {
                    // Propagate result of the NextParser.
                    return static_cast<bool>(result);
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto save = reader;

            // We can safely discard; token does not produce any values.
            using token_parser = lexy::rule_parser<Token, lexy::context_discard_parser<Context>>;
            auto result        = token_parser::try_parse(context, reader);
            if (result != lexy::rule_try_parse_result::ok)
                return result;

            // Continue parsing with our special continuation.
            return _continuation::try_parse(context, reader, save, LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the token with our special continuation and remember the current reader.
            return lexy::rule_parser<Token, _continuation>::parse(context, reader, Reader(reader),
                                                                  LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, Token, ErrorTag> error = {};
};

// Optimization for identifiers: instead of parsing an entire identifier (which requires checking
// every character against the char class), parse a symbol and check whether the next character
// would continue the identifier. This is the same optimization that is done for keywords.
template <const auto& Table, typename L, typename T, typename Tag>
struct _sym<Table, _idp<L, T>, Tag> : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using trailing_engine = typename T::token_engine;

            // Try to parse the symbol.
            auto save = reader;
            auto idx  = Table.try_parse(reader);
            // We need a symbol and it must not be the prefix of an identifier.
            if (!idx || lexy::engine_peek<trailing_engine>(reader))
            {
                // We didn't have a symbol, so backtrack.
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.token(_idp<L, T>::token_kind(), save.cur(), reader.cur());
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using trailing_engine = typename T::token_engine;

            auto begin = reader.cur();
            auto idx   = Table.try_parse(reader);
            // We need a symbol and it must not be the prefix of an identifier.
            if (!idx || lexy::engine_peek<trailing_engine>(reader))
            {
                // We didn't have a symbol.
                // But before we can report the error, we need to parse an identifier.
                // Otherwise, we don't call `context.token()` or have the same end as the
                // non-optimized symbol parser.

                if (!idx)
                {
                    // We need to parse the entire identifier from scratch.
                    // The identifier pattern does not produce a value, so we can safely discard.
                    using id_parser
                        = lexy::rule_parser<_idp<L, T>, lexy::context_discard_parser<Context>>;
                    if (!id_parser::parse(context, reader))
                        // Didn't have an identifier, so different error.
                        return false;
                }
                else
                {
                    // We're having a prefix of a valid identifier.
                    // As an additional optimization, just need to parse the remaining characters.
                    lexy::engine_while<trailing_engine>::match(reader);
                    context.token(_idp<L, T>::token_kind(), begin, reader.cur());
                }
                auto end = reader.cur();

                // Now we can report the erorr.
                using tag = std::conditional_t<std::is_void_v<Tag>, lexy::unknown_symbol, Tag>;
                auto err  = lexy::make_error<Reader, tag>(begin, end);
                context.error(err);
                return false;
            }
            auto end = reader.cur();

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.token(_idp<L, T>::token_kind(), begin, end);
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, _idp<L, T>, ErrorTag> error = {};
};

/// Parses rule, then matches the resulting lexeme against the symbol table.
template <const auto& Table, typename Token>
LEXY_CONSTEVAL auto symbol(Token)
{
    static_assert(lexy::is_token<Token>);
    return _sym<Table, Token, void>{};
}
template <const auto& Table, typename L, typename T, typename... R>
LEXY_CONSTEVAL auto symbol(_id<L, T, R...> id)
{
    static_assert(sizeof...(R) == 0,
                  "symbol() must not be used in the presence of reserved identifiers");
    return _sym<Table, decltype(id.pattern()), void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SYMBOL_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_VALUE_HPP_INCLUDED
#define LEXY_DSL_VALUE_HPP_INCLUDED




namespace lexy
{
struct _match_context;
} // namespace lexy

namespace lexyd
{
template <auto Value>
struct _valc : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., Value);
        }
    };
};

/// Produces the specified value without parsing anything.
template <auto Value>
constexpr auto value_c = _valc<Value>{};
} // namespace lexyd

namespace lexyd
{
template <auto F>
struct _valf : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., F());
        }
    };
};

/// Produces the value returned by the function without parsing anything.
template <auto F>
constexpr auto value_f = _valf<F>{};
} // namespace lexyd

namespace lexyd
{
template <typename T>
struct _valt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., T());
        }
    };
};

/// Produces a default constructed value of the specified type without parsing anything.
template <typename T>
constexpr auto value_t = _valt<T>{};
} // namespace lexyd

namespace lexyd
{
template <typename String>
struct _vals : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_same_v<Context, lexy::_match_context>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                constexpr auto str = String::get();
                return NextParser::parse(context, reader, LEXY_FWD(args)..., str.data(),
                                         str.size());
            }
        }
    };
};

#if LEXY_HAS_NTTP
/// Produces the string value.
template <lexy::_detail::string_literal Str>
constexpr auto value_str = _vals<lexy::_detail::type_string<Str>>{};
#endif

#define LEXY_VALUE_STR(Str)                                                                        \
    ::lexyd::_vals<LEXY_NTTP_STRING(Str)> {}
} // namespace lexyd

#endif // LEXY_DSL_VALUE_HPP_INCLUDED




#ifdef LEXY_IGNORE_DEPRECATED_ESCAPE
#    define LEXY_DEPRECATED_ESCAPE
#else
#    define LEXY_DEPRECATED_ESCAPE [[deprecated("`.lit[_c]()` has been replaced by `.symbol()`")]]
#endif

namespace lexy
{
/// The reader ends before the closing delimiter was found.
struct missing_delimiter
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing delimiter";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Char, typename Context, typename Reader, typename Sink>
constexpr auto _del_parse_char(Context& context, Reader& reader, Sink& sink)
{
    using engine = typename Char::token_engine;
    if constexpr (lexy::engine_can_fail<engine, Reader>)
    {
        auto content_begin = reader.cur();
        if (auto ec = engine::match(reader); ec != typename engine::error_code())
        {
            Char::token_error(context, reader, ec, content_begin);
            if (!engine::recover(reader, ec))
                return false;
            else
                // We've recovered, repeat loop.
                return true;
        }
        auto content_end = reader.cur();

        context.token(Char::token_kind(), content_begin, content_end);
        sink(lexy::lexeme<Reader>(content_begin, content_end));
    }
    else
    {
        auto content_begin = reader.cur();
        engine::match(reader);
        auto content_end = reader.cur();

        context.token(Char::token_kind(), content_begin, content_end);
        sink(lexy::lexeme<Reader>(content_begin, content_end));
    }

    return true;
}

template <typename Close, typename Char, typename Escape, typename Limit>
struct _del : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink      = context.sink();
            auto del_begin = reader.cur();

            using close  = lexy::rule_parser<Close, _list_finish<NextParser, Args...>>;
            using escape = lexy::rule_parser<Escape, _list_sink>;
            while (true)
            {
                // Try to finish parsing the production.
                if (auto result = close::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had a closing delimiter, return that result.
                    return static_cast<bool>(result);
                }
                // Check for missing closing delimiter.
                else if (lexy::engine_peek<typename Limit::token_engine>(reader))
                {
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    context.error(err);
                    return false;
                }
                // Try to parse an escape sequence.
                else if (auto result = escape::try_parse(context, reader, sink);
                         result != lexy::rule_try_parse_result::backtracked)
                {
                    // If we just parsed an escape sequence, we just continue with the next
                    // character.
                    //
                    // If we had an invalid escape sequence, we also just continue as if
                    // nothing happened.
                    // The leading escape character will be skipped, as well as any valid prefixes.
                    // We could try and add them to the list, but it should be fine as-is.
                }
                // Parse the next character.
                else
                {
                    if (!_del_parse_char<Char>(context, reader, sink))
                        return false;
                }
            }

            return false; // unreachable
        }
    };
};
template <typename Close, typename Char, typename Limit>
struct _del<Close, Char, void, Limit>
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink      = context.sink();
            auto del_begin = reader.cur();

            using close = lexy::rule_parser<Close, _list_finish<NextParser, Args...>>;
            while (true)
            {
                // Try to finish parsing the production.
                if (auto result = close::try_parse(context, reader, LEXY_FWD(args)..., sink);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had a closing delimiter, return that result.
                    return static_cast<bool>(result);
                }
                // Check for missing closing delimiter.
                else if (lexy::engine_peek<typename Limit::token_engine>(reader))
                {
                    auto err = lexy::make_error<Reader, lexy::missing_delimiter>(del_begin,
                                                                                 reader.cur());
                    context.error(err);
                    return false;
                }
                // Parse the next character.
                else
                {
                    if (!_del_parse_char<Char>(context, reader, sink))
                        return false;
                }
            }

            return false; // unreachable
        }
    };
};

template <typename Open, typename Close, typename Limit>
struct _delim_dsl
{
    /// Add tokens that will limit the delimited to detect a missing terminator.
    template <typename... Tokens>
    LEXY_CONSTEVAL auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token<Tokens> && ...));
        return _delim_dsl<Open, Close, decltype((Limit{} / ... / Tokens{}))>{};
    }

    //=== rules ===//
    /// Sets the content.
    template <typename Char>
    LEXY_CONSTEVAL auto operator()(Char) const
    {
        static_assert(lexy::is_token<Char>);
        return no_whitespace(open() >> _del<Close, Char, void, Limit>{});
    }
    template <typename Char, typename Escape>
    LEXY_CONSTEVAL auto operator()(Char, Escape) const
    {
        static_assert(lexy::is_token<Char>);
        static_assert(lexy::is_branch<Escape>);
        return no_whitespace(open() >> _del<Close, Char, Escape, Limit>{});
    }

    //=== access ===//
    /// Matches the open delimiter.
    LEXY_CONSTEVAL auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    LEXY_CONSTEVAL auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }

    //=== deprecated ===//
    /// Sets the whitespace.
    template <typename Ws>
    LEXY_CONSTEVAL auto operator[](Ws ws) const
    {
        auto open = whitespaced(Open{}, ws);
        return _delim_dsl<decltype(open), Close, Limit>{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
LEXY_CONSTEVAL auto delimited(Open, Close)
{
    static_assert(lexy::is_branch<Open> && lexy::is_branch<Close>);
    return _delim_dsl<Open, Close, lexyd::_eof>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
LEXY_CONSTEVAL auto delimited(Delim)
{
    static_assert(lexy::is_branch<Delim>);
    return _delim_dsl<Delim, Delim, lexyd::_eof>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

namespace lexy
{
struct invalid_escape_sequence
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid escape sequence";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Escape, typename... Branches>
LEXY_CONSTEVAL auto _escape_rule(Branches... branches)
{
    if constexpr (sizeof...(Branches) == 0)
        return Escape{};
    else
        return Escape{} >> (branches | ... | error<lexy::invalid_escape_sequence>);
}

template <typename Engine>
struct _escape_cap : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto begin = reader.cur();
            if (!lexy::engine_try_match<Engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::lexeme(reader, begin)));
        }
    };
};

struct _escape_char : token_base<_escape_char>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (reader.eof())
                return error_code::error;
            reader.bump();
            return error_code();
        }
    };

    // Don't need error, it won't be called.
};

template <typename Escape, typename... Branches>
struct _escape : decltype(_escape_rule<Escape>(Branches{}...))
{
    /// Adds a generic escape rule.
    template <typename Branch>
    LEXY_CONSTEVAL auto rule(Branch) const
    {
        static_assert(lexy::is_branch<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the token.
    template <typename Token>
    LEXY_CONSTEVAL auto capture(Token) const
    {
        static_assert(lexy::is_token<Token>);
        return this->rule(_escape_cap<typename Token::token_engine>{});
    }

    /// Adds an escape rule that parses the symbol.
    template <const auto& Table, typename Rule>
    LEXY_CONSTEVAL auto symbol(Rule rule) const
    {
        return this->rule(lexyd::symbol<Table>(rule));
    }
    /// Adds an escape rule that parses the symbol from the next code unit.
    template <const auto& Table>
    LEXY_CONSTEVAL auto symbol() const
    {
        return this->symbol<Table>(_escape_char{});
    }

#if LEXY_HAS_NTTP
    /// Adds an escape rule that replaces the escaped string with the replacement.
    template <lexy::_detail::string_literal Str, typename Value>
    LEXY_DEPRECATED_ESCAPE LEXY_CONSTEVAL auto lit(Value value) const
    {
        return rule(lexyd::lit<Str> >> value);
    }
    /// Adds an escape rule that replaces the escaped string with itself.
    template <lexy::_detail::string_literal Str>
    LEXY_DEPRECATED_ESCAPE LEXY_CONSTEVAL auto lit() const
    {
        return lit<Str>(value_str<Str>);
    }
#endif

    /// Adds an escape rule that replaces the escaped character with the replacement.
    template <auto C, typename Value>
    LEXY_DEPRECATED_ESCAPE LEXY_CONSTEVAL auto lit_c(Value value) const
    {
        return rule(lexyd::lit_c<C> >> value);
    }
    /// Adds an escape rule that replaces the escape character with itself.
    template <auto C>
    LEXY_DEPRECATED_ESCAPE LEXY_CONSTEVAL auto lit_c() const
    {
        return lit_c<C>(value_c<C>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
LEXY_CONSTEVAL auto escape(EscapeToken)
{
    static_assert(lexy::is_token<EscapeToken>);
    return _escape<EscapeToken>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED






// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_DIGITS_HPP_INCLUDED
#define LEXY_ENGINE_DIGITS_HPP_INCLUDED



namespace lexy
{
/// Match one or more of the specified digits.
template <typename DigitSet>
struct engine_digits : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet>);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We need at least one digit.
        if (auto ec = DigitSet::match(reader); ec != error_code())
            return ec;

        // Match subsequent digits as often as possible.
        while (engine_try_match<DigitSet>(reader))
        {}

        return error_code();
    }
};

/// Match one or more of the specified digits with digit separator in between.
template <typename DigitSet, typename Sep>
struct engine_digits_sep : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet> && lexy::engine_is_matcher<Sep>);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We need at least one digit.
        if (auto ec = DigitSet::match(reader); ec != error_code())
            return ec;

        while (true)
        {
            if (engine_try_match<Sep>(reader))
            {
                // After the separator we need another digit.
                if (auto ec = DigitSet::match(reader); ec != error_code())
                    return ec;
            }
            else if (!engine_try_match<DigitSet>(reader))
                // If we didn't have a separator, digit is optional and terminates.
                break;
        }

        return error_code();
    }
};
} // namespace lexy

namespace lexy
{
/// Match one or more of the specified digits, trimmed from unnecessary leading zeroes.
template <typename DigitSet, typename Zero>
struct engine_digits_trimmed : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet> && lexy::engine_is_matcher<Zero>);

    enum class error_code
    {
        leading_zero = 1,
    };

    static constexpr error_code translate(typename DigitSet::error_code ec)
    {
        LEXY_PRECONDITION(ec != typename DigitSet::error_code());
        return error_code(int(ec) + 1);
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        if (engine_try_match<Zero>(reader))
        {
            // Check if it is followed by another digit, which is not allowed.
            if (engine_peek<DigitSet>(reader))
                return error_code::leading_zero;
            else
                return error_code();
        }
        else
        {
            // We need at least one digit.
            if (auto ec = DigitSet::match(reader); ec != typename DigitSet::error_code())
                return translate(ec);

            // Match subsequent digits as often as possible.
            while (engine_try_match<DigitSet>(reader))
            {}

            return error_code();
        }
    }
};

/// Match one or more of the specified digits optionally separated, trimmed from unnecessary leading
/// zeroes.
template <typename DigitSet, typename Zero, typename Sep>
struct engine_digits_trimmed_sep : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<
                      DigitSet> && lexy::engine_is_matcher<Zero> && lexy::engine_is_matcher<Sep>);

    enum class error_code
    {
        leading_zero = 1,
    };

    static constexpr error_code translate(typename DigitSet::error_code ec)
    {
        LEXY_PRECONDITION(ec != typename DigitSet::error_code());
        return error_code(int(ec) + 1);
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        if (engine_try_match<Zero>(reader))
        {
            // Check if it is followed by another digit, which is not allowed.
            if (engine_peek<DigitSet>(reader) || engine_peek<Sep>(reader))
                return error_code::leading_zero;
            else
                return error_code();
        }
        else
        {
            // We need at least one digit.
            if (auto ec = DigitSet::match(reader); ec != typename DigitSet::error_code())
                return translate(ec);

            while (true)
            {
                if (engine_try_match<Sep>(reader))
                {
                    // After the separator we need another digit.
                    if (auto ec = DigitSet::match(reader); ec != typename DigitSet::error_code())
                        return translate(ec);
                }
                else if (!engine_try_match<DigitSet>(reader))
                    // If we didn't have a separator, digit is optional and terminates.
                    break;
            }

            return error_code();
        }
    }
};
} // namespace lexy

namespace lexy
{
/// Matches exactly N digits.
template <std::size_t N, typename DigitSet>
struct engine_ndigits : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet>);
    static_assert(N > 0);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        switch (N)
        {
        case 4:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            // fallthrough
        case 3:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            // fallthrough
        case 2:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            // fallthrough
        case 1:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            break;

        default:
            for (auto i = 0u; i != N; ++i)
            {
                if (auto ec = DigitSet::match(reader); ec != error_code())
                    return ec;
            }
            break;
        }

        return error_code();
    }
};

/// Matches exactly N digits optionally separated.
template <std::size_t N, typename DigitSet, typename Sep>
struct engine_ndigits_sep : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<DigitSet> && lexy::engine_is_matcher<Sep>);
    static_assert(N > 0);

    using error_code = typename DigitSet::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        switch (N)
        {
        case 4:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            engine_try_match<Sep>(reader);
            // fallthrough
        case 3:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            engine_try_match<Sep>(reader);
            // fallthrough
        case 2:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            engine_try_match<Sep>(reader);
            // fallthrough
        case 1:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;
            break;

        default:
            if (auto ec = DigitSet::match(reader); ec != error_code())
                return ec;

            for (auto i = 1u; i != N; ++i)
            {
                engine_try_match<Sep>(reader);

                if (auto ec = DigitSet::match(reader); ec != error_code())
                    return ec;
            }

            break;
        }

        return error_code();
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_DIGITS_HPP_INCLUDED


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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "digit.zero");
        context.error(err);
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.error(err);
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
            auto err = lexy::make_error<Reader, lexy::forbidden_leading_zero>(pos, reader.cur());
            context.error(err);
        }
        else
        {
            auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
            context.error(err);
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.error(err);
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

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::leading_zero)
        {
            auto err = lexy::make_error<Reader, lexy::forbidden_leading_zero>(pos, reader.cur());
            context.error(err);
        }
        else
        {
            auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
            context.error(err);
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

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.error(err);
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

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.error(err);
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
        context.error(err);
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ENCODE_HPP_INCLUDED
#define LEXY_DSL_ENCODE_HPP_INCLUDED







namespace lexyd
{
template <typename Reader, typename Encoding, lexy::encoding_endianness Endianness>
struct _encoded_reader // Single byte encodings
{
    static_assert(sizeof(typename Encoding::char_type) == 1);

    using encoding         = Encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = typename Reader::iterator;
    using canonical_reader = _encoded_reader<Reader, Encoding, Endianness>;

    constexpr bool eof() const
    {
        return _reader.eof();
    }

    constexpr auto peek() const
    {
        if (_reader.eof())
            return encoding::eof();
        else
            return encoding::to_int_type(static_cast<char_type>(*_reader.cur()));
    }

    constexpr void bump()
    {
        _reader.bump();
    }

    constexpr iterator cur() const
    {
        return _reader.cur();
    }

    Reader& _reader;
};
template <typename Reader, lexy::encoding_endianness Endianness>
struct _encoded_reader<Reader, lexy::utf16_encoding, Endianness>
{
    using encoding         = lexy::utf16_encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = typename Reader::iterator;
    using canonical_reader = _encoded_reader<Reader, lexy::utf16_encoding, Endianness>;

    constexpr bool eof() const
    {
        return peek() == eof();
    }

    constexpr auto peek() const
    {
        auto copy = _reader;
        if (copy.eof())
            return encoding::eof();
        auto first = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto second = static_cast<char_type>(*copy.cur());

        if constexpr (Endianness == lexy::encoding_endianness::little)
            return encoding::to_int_type(static_cast<char_type>((second << 8) | first));
        else
            return encoding::to_int_type(static_cast<char_type>((first << 8) | second));
    }

    constexpr void bump()
    {
        _reader.bump();
        _reader.bump();
    }

    constexpr iterator cur() const
    {
        return _reader.cur();
    }

    Reader& _reader;
};
template <typename Reader, lexy::encoding_endianness Endianness>
struct _encoded_reader<Reader, lexy::utf32_encoding, Endianness>
{
    using encoding         = lexy::utf32_encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = typename Reader::iterator;
    using canonical_reader = _encoded_reader<Reader, lexy::utf32_encoding, Endianness>;

    constexpr bool eof() const
    {
        return peek() == eof();
    }

    constexpr auto peek() const
    {
        auto copy = _reader;
        if (copy.eof())
            return encoding::eof();
        auto first = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto second = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto third = static_cast<char_type>(*copy.cur());

        copy.bump();
        if (copy.eof())
            return encoding::eof();
        auto fourth = static_cast<char_type>(*copy.cur());

        if constexpr (Endianness == lexy::encoding_endianness::little)
        {
            auto c = (fourth << 24) | (third << 16) | (second << 8) | first;
            return encoding::to_int_type(static_cast<char_type>(c));
        }
        else
        {
            auto c = (first << 24) | (second << 16) | (third << 8) | fourth;
            return encoding::to_int_type(static_cast<char_type>(c));
        }
    }

    constexpr void bump()
    {
        _reader.bump();
        _reader.bump();
        _reader.bump();
        _reader.bump();
    }

    constexpr iterator cur() const
    {
        return _reader.cur();
    }

    Reader& _reader;
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _encode_begin : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using old_encoding = typename Reader::encoding;
            static_assert((std::is_same_v<old_encoding, lexy::default_encoding>)
                              || (std::is_same_v<old_encoding, lexy::byte_encoding>),
                          "cannot re-encode input");

            auto encoded_reader = _encoded_reader<Reader, Encoding, Endianness>{reader};
            return NextParser::parse(context, encoded_reader, LEXY_FWD(args)...);
        }
    };
};

struct _encode_end : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename Encoding,
                  lexy::encoding_endianness Endianness, typename... Args>
        LEXY_DSL_FUNC bool parse(Context&                                       context,
                                 _encoded_reader<Reader, Encoding, Endianness>& reader,
                                 Args&&... args)
        {
            return NextParser::parse(context, reader._reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _encode
{
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        if constexpr (Endianness == lexy::encoding_endianness::bom)
        {
            if constexpr (sizeof(typename Encoding::char_type) == 1)
            {
                // The Endianness doesn't matter, just parse a BOM.
                return if_(bom<Encoding, lexy::encoding_endianness::little>)
                       + _encode<Encoding, lexy::encoding_endianness::little>{}(rule);
            }
            else
            {
                auto encode_little = _encode<Encoding, lexy::encoding_endianness::little>{}(rule);
                auto encode_big    = _encode<Encoding, lexy::encoding_endianness::big>{}(rule);

                auto little   = bom<Encoding, lexy::encoding_endianness::little> >> encode_little;
                auto big      = bom<Encoding, lexy::encoding_endianness::big> >> encode_big;
                auto fallback = else_ >> encode_big;

                return little | big | fallback;
            }
        }
        else
            return _encode_begin<Encoding, Endianness>{} + rule + _encode_end{};
    }
};

/// Matches the rule using the specified encoding.
template <typename Encoding, lexy::encoding_endianness Endianness = lexy::encoding_endianness::bom>
constexpr auto encode = _encode<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_ENCODE_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_IDENTIFIER_HPP_INCLUDED













//=== identifier ===//
namespace lexy
{
/// Error when we matched a reserved.
struct reserved_identifier
{
    static LEXY_CONSTEVAL auto name()
    {
        return "reserved identifier";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Leading, typename Trailing>
struct _idp : token_base<_idp<Leading, Trailing>>
{
    static LEXY_CONSTEVAL auto token_kind()
    {
        return lexy::identifier_token_kind;
    }

    struct token_engine : lexy::engine_matcher_base
    {
        using error_code = typename Leading::token_engine::error_code;

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (auto ec = Leading::token_engine::match(reader); ec != error_code())
                return ec;

            lexy::engine_while<typename Trailing::token_engine>::match(reader);
            return error_code();
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, Reader reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        Leading::token_error(context, reader, ec, pos);
    }
};

// Not a full token, we only need ::token_engine to make it work.
template <typename R>
struct _contains : token_base<_contains<R>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            not_found = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            using condition = typename decltype(lexyd::token(R{}))::token_engine;
            if (!lexy::engine_try_match<lexy::engine_find<condition>>(reader))
                return error_code::not_found;

            lexy::engine_any::match(reader);
            return error_code();
        }
    };
};

template <typename String, typename Id>
struct _kw;

template <typename Leading, typename Trailing, typename... Reserved>
struct _id : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename... PrevArgs>
        struct _continuation
        {
            template <typename Context, typename Reader>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args,
                                     Reader old)
            {
                auto begin = old.cur();
                auto end   = reader.cur();

                // Check that we're not creating a reserved identifier.
                if constexpr (sizeof...(Reserved) > 0)
                {
                    using reserved = decltype((Reserved{} / ...));

                    auto id_reader = lexy::partial_reader(old, end);
                    if (lexy::engine_try_match<typename reserved::token_engine>(id_reader)
                        && id_reader.cur() == end)
                    {
                        // We found a reserved identifier.
                        auto err = lexy::make_error<Reader, lexy::reserved_identifier>(begin, end);
                        context.error(err);
                        // But we can trivially recover, as we've still matched a well-formed
                        // identifier.
                    }
                }

                // We're done, create the value and continue.
                return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                         lexy::lexeme<Reader>(begin, end));
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the pattern with a special continuation.
            using pattern = _idp<Leading, Trailing>;
            using cont    = _continuation<Args...>;
            return lexy::rule_parser<pattern, cont>::parse(context, reader, LEXY_FWD(args)...,
                                                           Reader(reader));
        }
    };

    template <typename R>
    LEXY_CONSTEVAL auto _make_reserve(R r) const
    {
        return lexyd::token(r);
    }
    template <typename String, typename Id>
    LEXY_CONSTEVAL auto _make_reserve(_kw<String, Id>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // We turn the keyword into a literal to be able to use a trie for matching.
        return _lit<String>{};
    }

    //=== dsl ===//
    /// Adds a set of reserved identifiers.
    template <typename... R>
    LEXY_CONSTEVAL auto reserve(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        return _id<Leading, Trailing, Reserved..., decltype(_make_reserve(r))...>{};
    }

    /// Reserves everything starting with the given rule.
    template <typename... R>
    LEXY_CONSTEVAL auto reserve_prefix(R... prefix) const
    {
        return reserve((prefix + lexyd::any)...);
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    LEXY_CONSTEVAL auto reserve_containing(R...) const
    {
        return reserve(_contains<R>{}...);
    }

    /// Matches every identifier, ignoring reserved ones.
    LEXY_CONSTEVAL auto pattern() const
    {
        return _idp<Leading, Trailing>{};
    }

    /// Matches the initial char set of an identifier.
    LEXY_CONSTEVAL auto leading_pattern() const
    {
        return Leading{};
    }

    /// Matches the trailing char set of an identifier.
    LEXY_CONSTEVAL auto trailing_pattern() const
    {
        return Trailing{};
    }
};

/// Creates an identifier that consists of one or more of the given tokens.
template <typename Token>
LEXY_CONSTEVAL auto identifier(Token)
{
    return _id<Token, Token>{};
}

/// Creates an identifier that consists of one leading token followed by zero or more trailing
/// tokens.
template <typename LeadingToken, typename TrailingToken>
LEXY_CONSTEVAL auto identifier(LeadingToken, TrailingToken)
{
    return _id<LeadingToken, TrailingToken>{};
}
} // namespace lexyd

//=== keyword ===//
namespace lexyd
{
template <typename String, typename Id>
struct _kw : token_base<_kw<String, Id>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            // Try to match the keyword.
            using literal_engine = typename _lit<String>::token_engine;
            if (auto ec = literal_engine::match(reader);
                ec != typename literal_engine::error_code())
                return error_code::error;

            // To qualify as a keyword, and not just the prefix of an identifier,
            // we must not have a trailing identifier character.
            using trailing_engine = typename decltype(Id{}.trailing_pattern())::token_engine;
            if (lexy::engine_peek<trailing_engine>(reader))
                return error_code::error;

            return error_code();
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, Reader reader,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        using reader_char_type = typename Reader::encoding::char_type;
        constexpr auto string  = String::template get<reader_char_type>();

        // Find the range of the identifier.
        auto begin = pos;
        if (begin == reader.cur())
        {
            // We failed at the first character, need to match the identifier as normal.
            using id_engine = typename decltype(Id{}.pattern())::token_engine;
            lexy::engine_try_match<id_engine>(reader);
        }
        else
        {
            // We have already moved past the initial character, consume trailing only.
            using trailing_engine = typename decltype(Id{}.trailing_pattern())::token_engine;
            lexy::engine_while<trailing_engine>::match(reader);
        }
        auto end = reader.cur();

        auto err = lexy::make_error<Reader, lexy::expected_keyword>(begin, end, string.c_str());
        context.error(err);
    }
};

template <typename String, typename L, typename T, typename... R>
LEXY_CONSTEVAL auto _keyword(_id<L, T, R...>)
{
    // We don't need the reserved words, remove them to keep type name short.
    static_assert(String::size > 0, "keyword must not be empty");
    return _kw<String, _id<L, T>>{};
}

#if LEXY_HAS_NTTP
/// Matches the keyword.
template <lexy::_detail::string_literal Str, typename L, typename T, typename... R>
LEXY_CONSTEVAL auto keyword(_id<L, T, R...> id)
{
    return _keyword<lexy::_detail::type_string<Str>>(id);
}
#endif

#define LEXY_KEYWORD(Str, Id) ::lexyd::_keyword<LEXY_NTTP_STRING(Str)>(Id)
} // namespace lexyd

#endif // LEXY_DSL_IDENTIFIER_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_INTEGER_HPP_INCLUDED
#define LEXY_DSL_INTEGER_HPP_INCLUDED

#include <limits>





namespace lexy
{
// Number of digits to express the given value.
template <typename Integer>
constexpr std::size_t _digit_count(int radix, Integer value)
{
    LEXY_PRECONDITION(value >= Integer(0));

    if (value == 0)
        return 1;

    std::size_t result = 0;
    while (value > 0)
    {
        value = Integer(value / Integer(radix));
        ++result;
    }
    return result;
}

template <typename T>
struct integer_traits
{
    using _limits = std::numeric_limits<T>;
    static_assert(_limits::is_integer);

    using type = T;

    static constexpr auto is_bounded = _limits::is_bounded;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, _limits::max());

    template <int Radix>
    static constexpr void add_digit_unchecked(T& result, unsigned digit)
    {
        result = T(result * T(Radix) + T(digit));
    }

    template <int Radix>
    static constexpr bool add_digit_checked(T& result, unsigned digit)
    {
        constexpr auto can_use_unsigned = [] {
            if constexpr (!std::is_integral_v<T>)
                // If it's not a built-in integer, be careful and don't do it.
                return false;
            else if constexpr (sizeof(T) >= sizeof(unsigned))
                // If it's bigger than unsigned, we can't use unsigned.
                return false;
            else
            {
                // We can do it if the worst-case does not overflow unsigned.
                auto worst_case = static_cast<unsigned>(_limits::max());
                return integer_traits<unsigned>::add_digit_checked<Radix>(worst_case, Radix - 1);
            }
        }();

        // Check whether we can do an optimization for small integers,
        // where we do the operation on unsigned and check later.
        if constexpr (can_use_unsigned)
        {
            // This can't overflow, we've checked it above.
            auto value = static_cast<unsigned>(result) * Radix + digit;

            // Check whether the final value can fit.
            if (value > static_cast<unsigned>(_limits::max()))
                return false;
            else
            {
                result = static_cast<T>(value);
                return true;
            }
        }
        else
        {
            // result *= Radix
            constexpr auto max_per_radix = _limits::max() / Radix;
            if (result > max_per_radix)
                return false;
            result = T(result * Radix);

            // result += digit
            if (result > T(_limits::max() - digit))
                return false;
            result = T(result + T(digit));

            return true;
        }
    }
};

template <>
struct integer_traits<code_point>
{
    using type = code_point;

    static constexpr auto is_bounded = true;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, 0x10'FFFF);

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        std::uint_least32_t value = result.value();
        integer_traits<std::uint_least32_t>::add_digit_unchecked<Radix>(value, digit);
        result = code_point(value);
    }
    template <int Radix>
    static constexpr bool add_digit_checked(type& result, unsigned digit)
    {
        std::uint_least32_t value = result.value();
        if (!integer_traits<std::uint_least32_t>::add_digit_checked<Radix>(value, digit))
            return false;
        result = code_point(value);
        return result.is_valid();
    }
};

template <typename T>
struct unbounded
{};
template <typename T>
struct integer_traits<unbounded<T>>
{
    using type                       = typename integer_traits<T>::type;
    static constexpr auto is_bounded = false;

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        integer_traits<T>::template add_digit_unchecked<Radix>(result, digit);
    }
};
} // namespace lexy

namespace lexy
{
struct integer_overflow
{
    static LEXY_CONSTEVAL auto name()
    {
        return "integer overflow";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename T>
constexpr bool _is_bounded = lexy::integer_traits<T>::is_bounded;

// Parses T in the Base while checking for overflow.
template <typename T, typename Base>
struct _unbounded_integer_parser
{
    using traits      = lexy::integer_traits<T>;
    using result_type = typename traits::type;

    static constexpr auto radix = Base::radix;

    template <typename Iterator>
    static constexpr bool parse(result_type& result, Iterator cur, Iterator end)
    {
        // Just parse digits until we've run out of digits.
        while (cur != end)
        {
            auto digit = Base::value(*cur++);
            if (digit >= Base::radix)
                // Skip digit separator.
                continue;

            traits::template add_digit_unchecked<radix>(result, digit);
        }

        return true;
    }
};

// Parses T in the Base without checking for overflow.
template <typename T, typename Base, bool AssumeOnlyDigits>
struct _bounded_integer_parser
{
    using traits      = lexy::integer_traits<T>;
    using result_type = typename traits::type;

    static constexpr auto radix = Base::radix;

    template <typename Iterator>
    static constexpr unsigned find_digit(Iterator& cur, Iterator end)
    {
        auto digit = 0u;
        while (true)
        {
            if (cur == end)
                // No more digits.
                return unsigned(-1);

            digit = Base::value(*cur++);
            if constexpr (AssumeOnlyDigits)
                break;
            else if (digit < Base::radix)
                break;
        }
        return digit;
    }

    template <typename Iterator>
    static constexpr bool parse(result_type& result, Iterator cur, Iterator end)
    {
        constexpr auto max_digit_count = traits::template max_digit_count<radix>;
        static_assert(max_digit_count > 1,
                      "integer must be able to store all possible digit values");

        // Skip leading zeroes.
        while (true)
        {
            if (cur == end)
                return true; // We only had zeroes.

            const auto digit = Base::value(*cur++);
            if (digit == 0 || digit >= radix)
                continue; // Zero or digit separator.

            // First non-zero digit, so we can assign it instead of adding.
            result = result_type(digit);
            break;
        }
        // At this point, we've parsed exactly one non-zero digit.

        // Handle max_digit_count - 1 digits without checking for overflow.
        // We cannot overflow, as the maximal value has one digit more.
        for (std::size_t digit_count = 1; digit_count < max_digit_count - 1; ++digit_count)
        {
            auto digit = find_digit(cur, end);
            if (digit == unsigned(-1))
                return true;

            traits::template add_digit_unchecked<radix>(result, digit);
        }

        // Handle the final digit, if there is any, while checking for overflow.
        {
            auto digit = find_digit(cur, end);
            if (digit == unsigned(-1))
                return true;

            if (!traits::template add_digit_checked<radix>(result, digit))
                return false;
        }

        // If we've reached this point, we've parsed the maximal number of digits allowed.
        // Now we can only fail if there are still digits left.
        return cur == end;
    }
};

// Continuation of integer that assumes the rule is already dealt with.
template <typename T, typename Base, bool AssumeOnlyDigits, typename Tag>
struct _int_p : rule_base
{
    using integer_parser
        = std::conditional_t<_is_bounded<T>, _bounded_integer_parser<T, Base, AssumeOnlyDigits>,
                             _unbounded_integer_parser<T, Base>>;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename Iterator, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Iterator begin, Args&&... args)
        {
            using tag        = std::conditional_t<std::is_void_v<Tag>, lexy::integer_overflow, Tag>;
            using error_type = lexy::error<typename Reader::canonical_reader, tag>;

            auto result = typename integer_parser::result_type(0);
            if (integer_parser::parse(result, begin, reader.cur()))
                return NextParser::parse(context, reader, LEXY_FWD(args)..., result);
            else
            {
                context.error(error_type(begin, reader.cur()));
                return false;
            }
        }
    };
};

// Captures the rule which is then parsed.
// Must be followed by _int_p.
template <typename Rule>
struct _int_c : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, NextParser>::parse(context, reader, reader.cur(),
                                                              LEXY_FWD(args)...);
        }
    };
};

/// Parses the digits matched by the rule into an integer type.
template <typename T, typename Base, typename Rule>
LEXY_CONSTEVAL auto integer(Rule)
{
    return _int_c<Rule>{} + _int_p<T, Base, false, void>{};
}

template <typename T, typename Base>
LEXY_CONSTEVAL auto integer(_digits<Base>)
{
    return _int_c<_digits<Base>>{} + _int_p<T, Base, true, void>{};
}
template <typename T, typename Base, typename Sep>
LEXY_CONSTEVAL auto integer(_digits_s<Base, Sep>)
{
    return _int_c<_digits_s<Base, Sep>>{} + _int_p<T, Base, false, void>{};
}
template <typename T, typename Base>
LEXY_CONSTEVAL auto integer(_digits_t<Base>)
{
    return _int_c<_digits_t<Base>>{} + _int_p<T, Base, true, void>{};
}
template <typename T, typename Base, typename Sep>
LEXY_CONSTEVAL auto integer(_digits_st<Base, Sep>)
{
    return _int_c<_digits_st<Base, Sep>>{} + _int_p<T, Base, false, void>{};
}

template <typename T, typename Base, std::size_t N>
LEXY_CONSTEVAL auto integer(_ndigits<N, Base>)
{
    return _int_c<_ndigits<N, Base>>{} + _int_p<T, Base, true, void>{};
}
template <typename T, typename Base, std::size_t N, typename Sep>
LEXY_CONSTEVAL auto integer(_ndigits_s<N, Base, Sep>)
{
    return _int_c<_ndigits_s<N, Base, Sep>>{} + _int_p<T, Base, true, void>{};
}
} // namespace lexyd

namespace lexy
{
struct invalid_code_point
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid code point";
    }
};
} // namespace lexy

namespace lexyd
{
/// Matches the number of a code point.
template <std::size_t N, typename Base = hex>
constexpr auto code_point_id = _int_c<_ndigits<N, Base>>{}
                               + _int_p<lexy::code_point, Base, true, lexy::invalid_code_point>{};
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED




// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED




namespace lexyd
{
template <typename Needle, typename End>
struct _look : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser // delegate parse
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using engine = lexy::engine_find_before<Needle, End>;
            if (!lexy::engine_peek<engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)...));
        }
    };

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Looks for the Needle before End.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
LEXY_CONSTEVAL auto lookahead(Needle, End)
{
    static_assert(lexy::is_token<Needle> && lexy::is_token<End>);
    return _look<typename Needle::token_engine, typename End::token_engine>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MINUS_HPP_INCLUDED
#define LEXY_DSL_MINUS_HPP_INCLUDED




// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_MINUS_HPP_INCLUDED
#define LEXY_ENGINE_MINUS_HPP_INCLUDED



namespace lexy
{
/// Matches `Matcher` but only if `Except` does not match.
template <typename Matcher, typename Except>
struct engine_minus : lexy::engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Matcher> && lexy::engine_is_matcher<Except>);

    enum class error_code
    {
        minus_failure = 1,
    };

    static constexpr error_code error_from_matcher(typename Matcher::error_code ec)
    {
        LEXY_PRECONDITION(ec != typename Matcher::error_code());
        return error_code(int(ec) + 1);
    }
    static constexpr auto error_to_matcher(error_code ec)
    {
        LEXY_PRECONDITION(int(ec) > 1);
        return typename Matcher::error_code(int(ec) - 1);
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        auto save = reader;

        // First match on the original input.
        if (auto ec = Matcher::match(reader); ec != typename Matcher::error_code())
            return error_from_matcher(ec);

        // Then check whether Except matches on the same input.
        if (auto partial = lexy::partial_reader(save, reader.cur());
            lexy::engine_try_match<Except>(partial) && partial.eof())
            // They did, so we don't match.
            return error_code::minus_failure;

        return error_code();
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        if (ec == error_code::minus_failure)
            // We've already consumed the input.
            return true;
        else
            return Matcher::recover(reader, error_to_matcher(ec));
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_MINUS_HPP_INCLUDED


namespace lexy
{
/// We've matched the Except of a minus.
struct minus_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "minus failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Token, typename Except>
struct _minus : token_base<_minus<Token, Except>>
{
    using token_engine
        = lexy::engine_minus<typename Token::token_engine, typename Except::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::minus_failure)
        {
            auto err = lexy::make_error<Reader, lexy::minus_failure>(pos, reader.cur());
            context.error(err);
        }
        else
        {
            Token::token_error(context, reader, token_engine::error_to_matcher(ec), pos);
        }
    }
};

/// Matches Token unless Except matches on the input Token matched.
template <typename Token, typename Except>
LEXY_CONSTEVAL auto operator-(Token, Except)
{
    static_assert(lexy::is_token<Token>);
    static_assert(lexy::is_token<Except>);
    return _minus<Token, Except>{};
}
template <typename Token, typename E, typename Except>
LEXY_CONSTEVAL auto operator-(_minus<Token, E>, Except except)
{
    static_assert(lexy::is_token<Except>);
    return _minus<Token, decltype(E{} / except)>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED






namespace lexyd
{
struct _nl : token_base<_nl>
{
    static constexpr auto _trie
        = lexy::trie<char, LEXY_NTTP_STRING("\n"), LEXY_NTTP_STRING("\r\n")>;
    using token_engine = lexy::engine_trie<_trie>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "newline");
        context.error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

namespace lexyd
{
struct _eol : token_base<_eol>
{
    struct token_engine : lexy::engine_matcher_base
    {
        using error_code = _nl::token_engine::error_code;

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (reader.eof())
                return error_code();
            else
                return _nl::token_engine::match(reader);
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "EOL");
        context.error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Matches the end of line (EOF or newline).
constexpr auto eol = _eol{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED




namespace lexy
{
// An optional type is something that has the following:
// * a default constructors: this means we can actually construct it from our `nullopt`
// * a dereference operator: this means that it actually contains something else
// * a contextual conversion to bool: this means that it might be "false" (i.e. empty)
//
// This definition should work:
// * it excludes all default constructible types that are convertible to bool (e.g. integers...)
// * it includes pointers, which is ok
// * it includes `std::optional` and all non-std implementations of it
template <typename T>
using _detect_optional_like = decltype(T(), *LEXY_DECLVAL(T&), !LEXY_DECLVAL(const T&));

struct nullopt
{
    template <typename T, typename = _detect_optional_like<T>>
    constexpr operator T() const
    {
        return T();
    }
};
} // namespace lexy

namespace lexyd
{
struct _nullopt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

constexpr auto nullopt = _nullopt{};
} // namespace lexyd

namespace lexyd
{
template <typename Branch>
struct _opt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using branch_parser = lexy::rule_parser<Branch, NextParser>;

            auto result = branch_parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Branch wasn't taken, continue anyway with nullopt.
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
            else
                // Return true/false depending on result.
                return static_cast<bool>(result);
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule)
{
    static_assert(lexy::is_branch<Rule>, "opt() requires a branch condition");
    if constexpr (Rule::is_unconditional_branch)
        // Branch is always taken, so don't wrap in opt().
        return Rule{};
    else
        return _opt<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename R, typename Recover>
struct _optt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Try to parse the terminator.
            using term_parser = lexy::rule_parser<Term, NextParser>;
            if (auto result
                = term_parser::try_parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, and thus created the empty optional.
                return static_cast<bool>(result);
            }

            // Parse the rule followed by the terminator.
            using parser = lexy::rule_parser<R, term_parser>;
            if (!parser::parse(context, reader, LEXY_FWD(args)...))
            {
                using recovery = lexy::rule_parser<Recover, NextParser>;
                return recovery::parse(context, reader, LEXY_FWD(args)...);
            }

            return true;
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PEEK_HPP_INCLUDED
#define LEXY_DSL_PEEK_HPP_INCLUDED




namespace lexyd
{
template <typename Engine, bool Expected>
struct _peek : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser // propagate parse
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (lexy::engine_peek<Engine>(reader) != Expected)
                return lexy::rule_try_parse_result::backtracked;

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)...));
        }
    };

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
LEXY_CONSTEVAL auto peek(Rule rule)
{
    using token = decltype(token(rule));
    return _peek<typename token::token_engine, true>{};
}

/// Check if at this reader position, the rule would not match, but don't actually consume any
/// characters if it does.
template <typename Rule>
LEXY_CONSTEVAL auto peek_not(Rule rule)
{
    using token = decltype(token(rule));
    return _peek<typename token::token_engine, false>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_POSITION_HPP_INCLUDED
#define LEXY_DSL_POSITION_HPP_INCLUDED




namespace lexyd
{
struct _pos : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto pos = reader.cur();
            context.token(lexy::position_token_kind, pos, pos);
            return NextParser::parse(context, reader, LEXY_FWD(args)..., pos);
        }
    };
};

/// Produces an iterator to the current reader position without parsing anything.
constexpr auto position = _pos{};
} // namespace lexyd

#endif // LEXY_DSL_POSITION_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED




namespace lexyd
{
// Not inline: one function per production.
template <typename Rule, typename Context, typename Reader>
constexpr bool _parse(Context& context, Reader& reader)
{
    return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader);
}
template <typename Rule, typename Context, typename Reader>
constexpr auto _try_parse(Context& context, Reader& reader)
{
    return lexy::rule_parser<Rule, lexy::context_value_parser>::try_parse(context, reader);
}

template <typename Production, typename Rule, typename NextParser>
struct _prd_parser
{
    struct _continuation
    {
        template <typename Context, typename Reader, typename ProdContext, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, ProdContext& prod_context,
                                 Args&&... args)
        {
            // If we're about to parse a token production, we need to continue with the parent's
            // (i.e. our current context) whitespace.
            using ws_next
                = std::conditional_t<lexy::is_token_production<Production>,
                                     lexy::whitespace_parser<Context, NextParser>, NextParser>;

            if constexpr (std::is_void_v<typename ProdContext::return_type>)
            {
                LEXY_MOV(prod_context).finish();
                return ws_next::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return ws_next::parse(context, reader, LEXY_FWD(args)...,
                                      LEXY_MOV(prod_context).finish());
            }
        }
    };

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
        -> lexy::rule_try_parse_result
    {
        auto prod_context = context.production_context(Production{}, reader.cur());

        if (auto result = _try_parse<Rule>(prod_context, reader);
            result == lexy::rule_try_parse_result::ok)
        {
            // We succesfully parsed the production.
            // The continuation will call `.finish()` for us.
            return static_cast<lexy::rule_try_parse_result>(
                _continuation::parse(context, reader, prod_context, LEXY_FWD(args)...));
        }
        else // backtracked or canceled
        {
            // Need to backtrack the partial production in either case.
            LEXY_MOV(prod_context).backtrack();
            return result;
        }
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto prod_context = context.production_context(Production{}, reader.cur());

        if (!_parse<Rule>(prod_context, reader))
        {
            // We failed to parse, need to backtrack.
            LEXY_MOV(prod_context).backtrack();
            return false;
        }

        // The continuation will call `.finish()` for us.
        return _continuation::parse(context, reader, prod_context, LEXY_FWD(args)...);
    }
};

template <typename Production>
struct _prd : rule_base
{
    using _rule = lexy::production_rule<Production>;

    static constexpr auto is_branch               = _rule::is_branch;
    static constexpr auto is_unconditional_branch = _rule::is_unconditional_branch;

    template <typename NextParser>
    using parser = _prd_parser<Production, _rule, NextParser>;

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};

template <typename Production>
struct _rec : rule_base
{
    template <typename NextParser>
    struct parser : _prd_parser<Production, lexy::production_rule<Production>, NextParser>
    {};

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PUNCTUATOR_HPP_INCLUDED
#define LEXY_DSL_PUNCTUATOR_HPP_INCLUDED




namespace lexyd
{
#define LEXY_PUNCT(Name, String)                                                                   \
    struct _##Name : ::lexyd::_lit<LEXY_NTTP_STRING(String)>                                       \
    {};                                                                                            \
    inline constexpr auto Name = _##Name {}

LEXY_PUNCT(period, ".");
LEXY_PUNCT(comma, ",");
LEXY_PUNCT(colon, ":");
LEXY_PUNCT(semicolon, ";");

LEXY_PUNCT(hyphen, "-");
LEXY_PUNCT(slash, "/");
LEXY_PUNCT(backslash, "\\");
LEXY_PUNCT(apostrophe, "'");

LEXY_PUNCT(hash_sign, "#");
LEXY_PUNCT(dollar_sign, "$");
LEXY_PUNCT(at_sign, "@");
} // namespace lexyd

#endif // LEXY_DSL_PUNCTUATOR_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_RETURN_HPP_INCLUDED
#define LEXY_DSL_RETURN_HPP_INCLUDED



namespace lexyd
{
struct _ret : rule_base
{
    // We unconditionally jump to the final parser.
    template <typename NextParser>
    using parser = lexy::context_value_parser;
};

/// Finishes parsing a production without considering subsequent rules.
constexpr auto return_ = _ret{};
} // namespace lexyd

#endif // LEXY_DSL_RETURN_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SIGN_HPP_INCLUDED
#define LEXY_DSL_SIGN_HPP_INCLUDED







namespace lexyd
{
/// Matches a plus sign or nothing, producing +1.
constexpr auto plus_sign = LEXY_LIT("+") >> value_c<+1> | else_ >> value_c<+1>;
/// Matches a minus sign or nothing, producing +1 or -1.
constexpr auto minus_sign = LEXY_LIT("-") >> value_c<-1> | else_ >> value_c<+1>;

/// Matches a plus or minus sign or nothing, producing +1 or -1.
constexpr auto sign
    = LEXY_LIT("+") >> value_c<+1> | LEXY_LIT("-") >> value_c<-1> | else_ >> value_c<+1>;
} // namespace lexyd

#endif // LEXY_DSL_SIGN_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is partialject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SWITCH_HPP_INCLUDED
#define LEXY_DSL_SWITCH_HPP_INCLUDED





#ifdef LEXY_IGNORE_DEPRECATED_SWITCH
#    define LEXY_DEPRECATED_SWITCH
#else
#    define LEXY_DEPRECATED_SWITCH                                                                 \
        [[deprecated("`dsl::switch()` has been replaced by `dsl::symbol()`")]]
#endif

namespace lexy
{
struct exhausted_switch
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted switch";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Token, typename Value>
struct _switch_case : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename PartialReader, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, PartialReader& partial, Reader& reader,
                                     Args&&... args) -> lexy::rule_try_parse_result
        {
            if (lexy::engine_try_match<typename Token::token_engine>(partial) && partial.eof())
                return static_cast<lexy::rule_try_parse_result>(
                    lexy::rule_parser<Value, NextParser>::parse(context, reader,
                                                                LEXY_FWD(args)...));
            else
                return lexy::rule_try_parse_result::backtracked;
        }
    };
};

template <typename Value>
struct _switch_case<void, Value> : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    using parser = lexy::rule_parser<Value, NextParser>;
};

// Selects the appropriate case after the switch rule has been matched.
template <typename NextParser, typename... Cases>
struct _switch_select;
template <typename NextParser, typename Tag>
struct _switch_select<NextParser, Tag>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&...)
    {
        // We didn't match any of the switch cases, report an error.
        // save.cur() is the beginning of the switched value, reader.cur() at the end.
        using tag = std::conditional_t<std::is_void_v<Tag>, lexy::exhausted_switch, Tag>;
        auto err  = lexy::make_error<Reader, tag>(save.cur(), reader.cur());
        context.error(err);
        return false;
    }
};
template <typename NextParser, typename H, typename... T>
struct _switch_select<NextParser, H, T...>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&... args)
    {
        if constexpr (H::is_unconditional_branch)
        {
            (void)save;
            return lexy::rule_parser<H, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // We only want to read what the value has matched.
            auto partial = lexy::partial_reader(save, reader.cur());

            auto result = lexy::rule_parser<H, NextParser>::try_parse(context, partial, reader,
                                                                      LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Try the next branch.
                return _switch_select<NextParser, T...>::parse(context, reader, save,
                                                               LEXY_FWD(args)...);
            else
                return static_cast<bool>(result);
        }
    }
};

template <typename Rule, typename Error, typename... Cases>
struct _switch : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // We parse the rule using our special continuation.
            // To recover the old reader position, we create a copy.
            using cont = _switch_select<NextParser, Cases..., Error>;
            return lexy::rule_parser<Rule, cont>::parse(context, reader, Reader(reader),
                                                        LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Adds a case to the switch.
    template <typename Token, typename Value>
    LEXY_CONSTEVAL auto case_(_br<Token, Value>) const
    {
        static_assert(lexy::is_token<Token>, "case condition must be a token");
        return _switch<Rule, Error, Cases..., _switch_case<Token, Value>>{};
    }

    /// Adds a default value to the switch.
    template <typename Default>
    LEXY_CONSTEVAL auto default_(Default) const
    {
        return _switch<Rule, Error, Cases..., _switch_case<void, Default>>{};
    }

    /// Adds an error on the default case.
    template <typename Tag>
    static constexpr _switch<Rule, Tag, Cases...> error = {};

    LEXY_DEPRECATED_ERROR("replace `switch.error<Tag>()` by `switch.error<Tag>`")
    constexpr _switch operator()() const
    {
        return *this;
    }
};

/// Switches on the lexeme matched by the rule.
/// The first case that will match the entire pattern will be taken.
template <typename Rule>
LEXY_DEPRECATED_SWITCH LEXY_CONSTEVAL auto switch_(Rule)
{
    return _switch<Rule, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SWITCH_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TIMES_HPP_INCLUDED
#define LEXY_DSL_TIMES_HPP_INCLUDED





namespace lexy
{
template <std::size_t N, typename T>
using times = T (&)[N];

template <typename T>
using twice = times<2, T>;
} // namespace lexy

namespace lexyd
{
template <std::size_t N, typename Rule>
LEXY_CONSTEVAL auto _gen_times(Rule rule)
{
    if constexpr (N == 1)
        return rule;
    else
        return rule + _gen_times<N - 1>(rule);
}
template <std::size_t N, typename Rule, typename Sep>
LEXY_CONSTEVAL auto _gen_times(Rule rule, Sep)
{
    if constexpr (N == 1)
        return rule + typename Sep::trailing_rule{};
    else
        return rule + typename Sep::rule{} + _gen_times<N - 1>(rule, Sep{});
}

template <std::size_t N, typename Rule, typename Sep>
struct _times : rule_base
{
    static LEXY_CONSTEVAL auto _repeated_rule()
    {
        if constexpr (std::is_same_v<Sep, void>)
            return _gen_times<N>(Rule{});
        else
            return _gen_times<N>(Rule{}, Sep{});
    }

    // We only use this template if our rule does not have a matcher.

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Context, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args,
                                     RuleArgs&&... rule_args)
            {
                // Create an array containing the rule arguments.
                static_assert(N == sizeof...(RuleArgs), "rule must create exactly one value");
                using array_type    = std::common_type_t<std::decay_t<RuleArgs>...>;
                array_type array[N] = {LEXY_FWD(rule_args)...};
                return NextParser::parse(context, reader, LEXY_FWD(args)..., array);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the rule with the special continuation that converts the value into an array
            // afterwards.
            using rule         = decltype(_repeated_rule());
            using continuation = _continuation<Args...>;
            return lexy::rule_parser<rule, continuation>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeats the rule N times and collects the values into an array.
template <std::size_t N, typename Rule>
LEXY_CONSTEVAL auto times(Rule)
{
    static_assert(N > 0);
    return _times<N, Rule, void>{};
}

/// Repeates the rule N times separated by the separator and collects the values into an array.
template <std::size_t N, typename Rule, typename Sep>
LEXY_CONSTEVAL auto times(Rule, Sep)
{
    static_assert(N > 0);
    return _times<N, Rule, Sep>{};
}

template <typename Rule>
LEXY_CONSTEVAL auto twice(Rule rule)
{
    return times<2>(rule);
}
template <typename Rule, typename Sep>
LEXY_CONSTEVAL auto twice(Rule rule, Sep sep)
{
    return times<2>(rule, sep);
}
} // namespace lexyd

#endif // LEXY_DSL_TIMES_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_UNTIL_HPP_INCLUDED
#define LEXY_ENGINE_UNTIL_HPP_INCLUDED



namespace lexy
{
/// Matches everything until and including Condition.
template <typename Condition>
struct engine_until : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition>);

    using error_code = typename Condition::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_try_match<Condition>(reader))
        {
            if (reader.eof())
            {
                // This match fails but gives us an appropriate error code.
                return Condition::match(reader);
            }

            reader.bump();
        }

        return error_code();
    }
};
} // namespace lexy

namespace lexy
{
/// Matches everything until and including Condition or EOF.
template <typename Condition>
struct engine_until_eof : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition>);

    enum class error_code
    {
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_try_match<Condition>(reader))
        {
            if (reader.eof())
                break;

            reader.bump();
        }

        return error_code();
    }
};

template <typename Condition, typename Reader>
inline constexpr bool engine_can_fail<engine_until_eof<Condition>, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_UNTIL_HPP_INCLUDED


namespace lexyd
{
template <typename Condition>
struct _until_eof : token_base<_until_eof<Condition>>
{
    using token_engine = lexy::engine_until_eof<typename Condition::token_engine>;
};

template <typename Condition>
struct _until : token_base<_until<Condition>>
{
    using token_engine = lexy::engine_until<typename Condition::token_engine>;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator)
    {
        // We don't pass the passed position, as this would be the beginning of until.
        // Instead we always use the current reader position (i.e. EOF) as that's where the
        // condition is missing.
        Condition::token_error(context, reader, ec, reader.cur());
    }

    /// Also accepts EOF as the closing condition.
    LEXY_CONSTEVAL auto or_eof() const
    {
        return _until_eof<Condition>{};
    }
};

/// Matches anything until Condition matches.
/// Then matches Condition.
template <typename Condition>
LEXY_CONSTEVAL auto until(Condition)
{
    static_assert(lexy::is_token<Condition>);
    return _until<Condition>{};
}
} // namespace lexyd

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED



namespace lexyd
{
template <typename Branch>
struct _whl : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                using branch_parser
                    = lexy::rule_parser<Branch, lexy::context_discard_parser<Context>>;

                auto result = branch_parser::try_parse(context, reader);
                if (result == lexy::rule_try_parse_result::backtracked)
                    break;
                else if (result == lexy::rule_try_parse_result::canceled)
                    return false;
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches the branch rule as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_(Rule)
{
    static_assert(lexy::is_branch<Rule>, "while() requires a branch condition");
    return _whl<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the rule at least once, then as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_one(Rule rule)
{
    static_assert(lexy::is_branch<Rule>, "while_one() requires a branch condition");
    return rule >> while_(rule);
}
} // namespace lexyd

namespace lexyd
{
/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
LEXY_CONSTEVAL auto do_while(Then then, Condition condition)
{
    if constexpr (lexy::is_branch<Then>)
        return then >> while_(condition >> then);
    else
        return then + while_(condition >> then);
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Rule, typename Recover>
struct _whlt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                using term_parser = lexy::rule_parser<Term, NextParser>;
                if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)...);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had the terminator, and thus are done.
                    return static_cast<bool>(result);
                }

                using parser = lexy::rule_parser<Rule, lexy::context_discard_parser<Context>>;
                if (!parser::parse(context, reader))
                {
                    using recovery = lexy::rule_parser<Recover, NextParser>;
                    return recovery::parse(context, reader, LEXY_FWD(args)...);
                }
            }

            return false; // unreachable
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED



#endif // LEXY_DSL_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_TREE_HPP_INCLUDED
#define LEXY_PARSE_TREE_HPP_INCLUDED





// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED
#define LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED



#include <new>

#if 0
// Subset of the interface of std::pmr::memory_resource.
class MemoryResource
{
public:
    void* allocate(std::size_t bytes, std::size_t alignment);
    void deallocate(void* ptr, std::size_t bytes, std::size_t alignment);

    friend bool operator==(const MemoryResource& lhs, const MemoryResource& rhs);
};
#endif

namespace lexy::_detail
{
class default_memory_resource
{
public:
    void* allocate(std::size_t bytes, std::size_t alignment)
    {
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            return ::operator new (bytes, std::align_val_t{alignment});
        else
            return ::operator new(bytes);
    }

    void deallocate(void* ptr, std::size_t bytes, std::size_t alignment) noexcept
    {
#ifdef __cpp_sized_deallocation
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            ::operator delete (ptr, bytes, std::align_val_t{alignment});
        else
            ::operator delete(ptr, bytes);
#else
        (void)bytes;

        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            ::operator delete (ptr, std::align_val_t{alignment});
        else
            ::operator delete(ptr);
#endif
    }

    friend constexpr bool operator==(default_memory_resource, default_memory_resource) noexcept
    {
        return true;
    }
};
} // namespace lexy::_detail

namespace lexy::_detail
{
template <typename MemoryResource>
class _memory_resource_ptr_empty
{
public:
    constexpr explicit _memory_resource_ptr_empty(MemoryResource*) noexcept {}

    constexpr auto operator*() const noexcept
    {
        return MemoryResource{};
    }

    constexpr auto operator->() const noexcept
    {
        struct proxy
        {
            MemoryResource _resource;

            constexpr MemoryResource* operator->() noexcept
            {
                return &_resource;
            }
        };

        return proxy{};
    }

    constexpr MemoryResource* get() const noexcept
    {
        return nullptr;
    }
};

template <typename MemoryResource>
class _memory_resource_ptr
{
public:
    constexpr explicit _memory_resource_ptr(MemoryResource* resource) noexcept : _resource(resource)
    {
        LEXY_PRECONDITION(resource);
    }

    constexpr MemoryResource& operator*() const noexcept
    {
        return *_resource;
    }

    constexpr MemoryResource* operator->() const noexcept
    {
        return _resource;
    }

    constexpr MemoryResource* get() const noexcept
    {
        return _resource;
    }

private:
    MemoryResource* _resource;
};

template <typename MemoryResource>
using memory_resource_ptr = std::conditional_t<std::is_empty_v<MemoryResource>,
                                               _memory_resource_ptr_empty<MemoryResource>,
                                               _memory_resource_ptr<MemoryResource>>;

template <typename MemoryResource>
constexpr MemoryResource* get_memory_resource()
{
    static_assert(std::is_empty_v<MemoryResource>, "need to pass a MemoryResource ptr");
    return nullptr;
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED






// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED







namespace lexy
{
// Convert the callback into an appropriate sink.
template <typename Callback>
constexpr auto _get_error_sink(const Callback& callback)
{
    if constexpr (std::is_same_v<Callback, lexy::_noop>)
    {
        // We collect noop instead, which counts the errors.
        return lexy::collect(callback).sink();
    }
    else if constexpr (lexy::is_sink<Callback>)
    {
        // It already is a sink.
        return callback.sink();
    }
    else
    {
        static_assert(
            std::is_void_v<typename Callback::return_type>,
            "need to use `lexy::collect()` to create an error callback that can handle multiple errors");

        // We need to collect the errors.
        return lexy::collect(callback).sink();
    }
}
template <typename Callback>
using _error_sink_t = decltype(_get_error_sink(LEXY_DECLVAL(Callback)));

template <typename ErrorCallback>
class validate_result
{
    using _sink_t = _error_sink_t<ErrorCallback>;

public:
    using error_callback = ErrorCallback;
    using error_type     = typename _sink_t::return_type;
    static_assert(!std::is_void_v<error_type>, "ErrorCallback must not be a void returning sink");

    constexpr explicit operator bool() const noexcept
    {
        return is_success();
    }

    constexpr bool is_success() const noexcept
    {
        return _status == _status_success;
    }
    constexpr bool is_error() const noexcept
    {
        return !is_success();
    }
    constexpr bool is_recovered_error() const noexcept
    {
        return _status == _status_recovered;
    }
    constexpr bool is_fatal_error() const noexcept
    {
        return _status == _status_fatal;
    }

    constexpr std::size_t error_count() const noexcept
    {
        if constexpr (std::is_same_v<error_type, std::size_t>)
            // void-returning callback yields the count only.
            return _error;
        else
            // We assume it's some sort of container otherwise.
            return _error.size();
    }

    constexpr const auto& errors() const& noexcept
    {
        return _error;
    }
    constexpr auto&& errors() && noexcept
    {
        return LEXY_MOV(_error);
    }

private:
    constexpr explicit validate_result(bool did_recover, error_type&& error)
    : _error(LEXY_MOV(error)), _status()
    {
        if (error_count() == 0u)
            _status = _status_success;
        else if (did_recover)
            _status = _status_recovered;
        else
            _status = _status_fatal;
    }

    error_type _error;
    enum
    {
        _status_success,
        _status_recovered,
        _status_fatal,
    } _status;

    template <typename Input, typename Callback>
    friend class validate_handler;
};
} // namespace lexy

namespace lexy
{
template <typename Input, typename ErrorCallback>
class validate_handler
{
public:
    constexpr explicit validate_handler(const Input& input, const ErrorCallback& callback)
    : _sink(_get_error_sink(callback)), _input(&input)
    {}

    constexpr auto get_result(bool did_recover) &&
    {
        return validate_result<ErrorCallback>(did_recover, LEXY_MOV(_sink).finish());
    }

    //=== handler functions ===//
    template <typename Production>
    using return_type_for = void;

    template <typename Production>
    constexpr auto get_sink(Production)
    {
        return noop.sink();
    }

    template <typename Production, typename Iterator>
    constexpr auto start_production(Production, Iterator pos)
    {
        return pos;
    }

    template <typename Kind, typename Iterator>
    constexpr void token(Kind, Iterator, Iterator)
    {}

    template <typename Production, typename Iterator, typename... Args>
    constexpr void finish_production(Production, Iterator, Args&&...)
    {}
    template <typename Production, typename Iterator>
    constexpr void backtrack_production(Production, Iterator)
    {}

    template <typename Production, typename Iterator, typename Error>
    constexpr void error(Production p, Iterator pos, Error&& error)
    {
        lexy::error_context err_ctx(p, *_input, pos);
        _sink(err_ctx, LEXY_FWD(error));
    }

private:
    _error_sink_t<ErrorCallback> _sink;
    const Input*                 _input;
};

template <typename Production, typename Input, typename ErrorCallback>
constexpr auto validate(const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = validate_handler(input, callback);
    auto reader  = input.reader();

    auto did_recover = lexy::_detail::parse_impl<Production>(handler, reader);
    return LEXY_MOV(handler).get_result(static_cast<bool>(did_recover));
}
} // namespace lexy

#endif // LEXY_VALIDATE_HPP_INCLUDED


//=== internal: pt_node ===//
namespace lexy::_detail
{
template <typename Reader>
struct pt_node;
template <typename Reader>
struct pt_node_token;
template <typename Reader>
struct pt_node_production;

template <typename Reader>
class pt_node_ptr
{
public:
    static constexpr auto type_token      = 0b0u;
    static constexpr auto type_production = 0b1u;

    static constexpr auto role_sibling = 0b0u;
    static constexpr auto role_parent  = 0b1u;

    // nullptr is a parent pointer to a non-existing parent.
    // This means that it is automatically an empty child range.
    pt_node_ptr() noexcept : pt_node_ptr(nullptr, type_token, role_parent) {}

    void set_sibling(pt_node<Reader>* ptr, unsigned type)
    {
        *this = pt_node_ptr(ptr, type, role_sibling);
    }
    void set_sibling(pt_node_token<Reader>* ptr)
    {
        *this = pt_node_ptr(ptr, type_token, role_sibling);
    }
    void set_sibling(pt_node_production<Reader>* ptr)
    {
        *this = pt_node_ptr(ptr, type_production, role_sibling);
    }

    void set_parent(pt_node_production<Reader>* ptr)
    {
        *this = pt_node_ptr(ptr, type_production, role_parent);
    }

    //=== access ===//
    explicit operator bool() const noexcept
    {
        return base() != nullptr;
    }

    unsigned type() const noexcept
    {
        return _value & 0b1;
    }

    auto* base() const noexcept
    {
        return reinterpret_cast<pt_node<Reader>*>(_value & ~std::uintptr_t(0b11));
    }

    auto token() const noexcept
    {
        return type() == type_token ? static_cast<pt_node_token<Reader>*>(base()) : nullptr;
    }
    auto production() const noexcept
    {
        return type() == type_production ? static_cast<pt_node_production<Reader>*>(base())
                                         : nullptr;
    }

    bool is_sibling_ptr() const noexcept
    {
        return ((_value & 0b10) >> 1) == role_sibling;
    }
    bool is_parent_ptr() const noexcept
    {
        return ((_value & 0b10) >> 1) == role_parent;
    }

private:
    explicit pt_node_ptr(pt_node<Reader>* ptr, unsigned type, unsigned role)
    : _value(reinterpret_cast<std::uintptr_t>(ptr))
    {
        LEXY_PRECONDITION((reinterpret_cast<std::uintptr_t>(ptr) & 0b11) == 0);

        _value |= (role & 0b1) << 1;
        _value |= (type & 0b1);
    }

    std::uintptr_t _value;
};

template <typename Reader>
struct pt_node
{
    // Either points back to the next child of the parent node (the sibling),
    // or back to the parent node if it is its last child.
    // It is only null for the root node.
    pt_node_ptr<Reader> ptr;
};

template <typename Reader>
struct pt_node_token : pt_node<Reader>
{
    // If it's not a pointer, we store size instead of end.
    static constexpr auto _optimize_end = std::is_pointer_v<typename Reader::iterator>;
    using _end_t
        // If we can optimize it, we store the size as a uint32_t, otherwise the iterator.
        = std::conditional_t<_optimize_end, std::uint_least32_t, typename Reader::iterator>;

    typename Reader::iterator begin;
    _end_t                    end_impl;
    ::uint_least16_t          kind;

    explicit pt_node_token(std::uint_least16_t kind, typename Reader::iterator begin,
                           typename Reader::iterator end) noexcept
    : begin(begin), kind(kind)
    {
        update_end(end);
    }

    typename Reader::iterator end() const noexcept
    {
        if constexpr (_optimize_end)
            return begin + end_impl;
        else
            return end_impl;
    }

    void update_end(typename Reader::iterator end) noexcept
    {
        if constexpr (_optimize_end)
        {
            static_assert(sizeof(pt_node_token) == 3 * sizeof(void*));

            auto size = std::size_t(end - begin);
            LEXY_PRECONDITION(size <= UINT_LEAST32_MAX);
            end_impl = std::uint_least32_t(size);
        }
        else
        {
            static_assert(sizeof(pt_node_token) <= 4 * sizeof(void*));

            end_impl = end;
        }
    }
};

template <typename Reader>
struct pt_node_production : pt_node<Reader>
{
    static constexpr std::size_t child_count_bits = sizeof(std::size_t) * CHAR_BIT - 3;

    const char* name;
    std::size_t child_count : child_count_bits;
    std::size_t token_production : 1;
    std::size_t first_child_adjacent : 1;
    std::size_t first_child_type : 1;

    template <typename Production>
    explicit pt_node_production(Production) noexcept
    : child_count(0), token_production(lexy::is_token_production<Production>),
      first_child_adjacent(true), first_child_type(pt_node_ptr<Reader>::type_token)
    {
        static_assert(sizeof(pt_node_production) == 3 * sizeof(void*));

        name = lexy::production_name<Production>();
    }

    pt_node_ptr<Reader> first_child()
    {
        auto memory = static_cast<void*>(this + 1);
        if (child_count == 0)
        {
            // We don't have a child at all.
            pt_node_ptr<Reader> result;
            result.set_parent(this);
            return result;
        }
        else if (first_child_adjacent)
        {
            // The first child is stored immediately afterwards.
            pt_node_ptr<Reader> result;
            result.set_sibling(static_cast<pt_node<Reader>*>(memory), first_child_type);
            return result;
        }
        else
        {
            // We're only storing a pointer to the first child immediately afterwards.
            return *static_cast<pt_node_ptr<Reader>*>(memory);
        }
    }
};
} // namespace lexy::_detail

//=== internal: pt_buffer ===//
namespace lexy::_detail
{
// Basic stack allocator to store all the nodes of a tree.
template <typename MemoryResource>
class pt_buffer
{
    using resource_ptr = _detail::memory_resource_ptr<MemoryResource>;

    static constexpr std::size_t block_size = 4096 - sizeof(void*);

    struct block
    {
        block*        next;
        unsigned char memory[block_size];

        static block* allocate(resource_ptr resource)
        {
            auto memory = resource->allocate(sizeof(block), alignof(block));
            auto ptr    = ::new (memory) block; // Don't initialize array!
            ptr->next   = nullptr;
            return ptr;
        }

        static block* deallocate(resource_ptr resource, block* ptr)
        {
            auto next = ptr->next;
            resource->deallocate(ptr, sizeof(block), alignof(block));
            return next;
        }

        unsigned char* end() noexcept
        {
            return &memory[block_size];
        }
    };

public:
    //=== constructors/destructors/assignment ===//
    explicit constexpr pt_buffer(MemoryResource* resource) noexcept
    : _resource(resource), _head(nullptr), _cur_block(nullptr), _cur_pos(nullptr)
    {}

    pt_buffer(pt_buffer&& other) noexcept
    : _resource(other._resource), _head(other._head), _cur_block(other._cur_block),
      _cur_pos(other._cur_pos)
    {
        other._head = other._cur_block = nullptr;
        other._cur_pos                 = nullptr;
    }

    ~pt_buffer() noexcept
    {
        auto cur = _head;
        while (cur != nullptr)
            cur = block::deallocate(_resource, cur);
    }

    pt_buffer& operator=(pt_buffer&& other) noexcept
    {
        lexy::_detail::swap(_resource, other._resource);
        lexy::_detail::swap(_head, other._head);
        lexy::_detail::swap(_cur_block, other._cur_block);
        lexy::_detail::swap(_cur_pos, other._cur_pos);
        return *this;
    }

    //=== allocation ===//
    // Allocates the first block for the buffer.
    // Must be called before everything else.
    // (If done in the constructor, it would require a move that does allocation which we don't
    // want).
    // If called after being initialized, destroys all nodes without releasing memory.
    void reset()
    {
        if (!_head)
            _head = block::allocate(_resource);

        _cur_block = _head;
        _cur_pos   = &_cur_block->memory[0];
    }

    void reserve(std::size_t size)
    {
        if (remaining_capacity() < size)
        {
            auto next        = block::allocate(_resource);
            _cur_block->next = next;
            _cur_block       = next;
            _cur_pos         = &_cur_block->memory[0];
        }
    }

    template <typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(alignof(T) == alignof(void*));
        LEXY_PRECONDITION(_cur_block);                        // Forgot to call .init().
        LEXY_PRECONDITION(remaining_capacity() >= sizeof(T)); // Forgot to call .reserve().

        auto memory = _cur_pos;
        _cur_pos += sizeof(T);
        return ::new (static_cast<void*>(memory)) T(LEXY_FWD(args)...);
    }

    void unwind(void* marker) noexcept
    {
        auto pos = static_cast<unsigned char*>(marker);

        // Note: this is not guaranteed to work by the standard;
        // We'd have to go through std::less instead.
        // However, on all implementations I care about, std::less just does < anyway.
        if (_cur_block->memory <= pos && pos < _cur_block->end())
            // We're still in the same block, just reset position.
            _cur_pos = pos;
        else
            // Reset to the beginning of the current block only.
            // This can waste memory, but this is not a problem here:
            // unwind() is only used to backtrack a production, which happens after a couple of
            // tokens only; the memory waste is directly proportional to the lookahead length.
            _cur_pos = _cur_block->memory;
    }

private:
    std::size_t remaining_capacity() const noexcept
    {
        return std::size_t(_cur_block->end() - _cur_pos);
    }

    LEXY_EMPTY_MEMBER resource_ptr _resource;
    block*                         _head;

    block*         _cur_block;
    unsigned char* _cur_pos;
};
} // namespace lexy::_detail

//=== parse_tree ===//
namespace lexy
{
template <typename Reader, typename TokenKind = void,
          typename MemoryResource = _detail::default_memory_resource>
class parse_tree
{
public:
    //=== construction ===//
    class builder;

    constexpr parse_tree() : parse_tree(_detail::get_memory_resource<MemoryResource>()) {}
    constexpr explicit parse_tree(MemoryResource* resource) : _buffer(resource), _root(nullptr) {}

    //=== container access ===//
    bool empty() const noexcept
    {
        return _root == nullptr;
    }

    void clear() noexcept
    {
        _buffer.reset();
        _root = nullptr;
    }

    //=== node access ===//
    class node;
    class node_kind;

    node root() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return node(_root);
    }

    //=== traverse ===//
    class traverse_range;

    traverse_range traverse(const node& n) const noexcept
    {
        return traverse_range(n);
    }
    traverse_range traverse() const noexcept
    {
        if (empty())
            return traverse_range();
        else
            return traverse_range(root());
    }

private:
    _detail::pt_buffer<MemoryResource>   _buffer;
    _detail::pt_node_production<Reader>* _root;
};

template <typename Input, typename TokenKind = void,
          typename MemoryResource = _detail::default_memory_resource>
using parse_tree_for = lexy::parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>;

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::builder
{
    struct state;

public:
    template <typename Production>
    explicit builder(parse_tree&& tree, Production production) : _result(LEXY_MOV(tree))
    {
        // Empty the initial parse tree.
        _result._buffer.reset();

        // Allocate a new root node and begin construction there.
        // No need to reserve for the initial node.
        _result._root
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _cur = state(_result._root);
    }
    template <typename Production>
    explicit builder(Production production) : builder(parse_tree(), production)
    {}

    using production_state = state;

    template <typename Production>
    auto start_production(Production production)
    {
        if constexpr (lexy::is_transparent_production<Production>)
            // Don't need to add a new node for a transparent production.
            return state();

        // Allocate a node for the production and append it to the current child list.
        // We reserve enough memory to allow for a trailing pointer.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node_ptr<Reader>));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        // Note: don't append the node yet, we might still backtrack.

        // Subsequent insertions are to the new node, so update state and return old one.
        auto old = LEXY_MOV(_cur);
        _cur     = state(node);
        return old;
    }

    void token(token_kind<TokenKind> _kind, typename Reader::iterator begin,
               typename Reader::iterator end)
    {
        if (!_kind && begin == end)
            // Don't add empty, unknown tokens.
            return;

        auto kind = token_kind<TokenKind>::to_raw(_kind);

        if (auto token = _cur.last_child.token();
            _cur.prod->token_production && token && token->kind == kind)
        {
            // We're having the same token again, merge with the previous one.
            token->update_end(end);
        }
        else
        {
            // Allocate and append.
            _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
            auto node
                = _result._buffer.template allocate<_detail::pt_node_token<Reader>>(kind, begin,
                                                                                    end);
            _cur.append(node);
        }
    }

    void finish_production(state&& s)
    {
        if (!s.prod)
            // We're finishing with a transparent production, do nothing.
            return;

        // We're done with the current production.
        _cur.finish();
        // Append to previous production.
        s.append(_cur.prod);
        // Continue with the previous production.
        _cur = LEXY_MOV(s);
    }

    void backtrack_production(state&& s)
    {
        if (!s.prod)
            // We're backtracking a transparent production, do nothing.
            return;

        // Deallocate everything from the backtracked production.
        _result._buffer.unwind(_cur.prod);
        // Continue with previous production.
        _cur = LEXY_MOV(s);
    }

    parse_tree finish() &&
    {
        LEXY_PRECONDITION(_cur.prod == _result._root);
        _cur.finish();
        return LEXY_MOV(_result);
    }

private:
    parse_tree _result;
    struct state
    {
        // The current production all tokens are appended to.
        _detail::pt_node_production<Reader>* prod = nullptr;
        // The last child of the current production.
        _detail::pt_node_ptr<Reader> last_child;

        state() = default;

        explicit state(_detail::pt_node_production<Reader>* prod) : prod(prod) {}

        template <typename T>
        void append(T* child)
        {
            ++prod->child_count;

            if (last_child)
            {
                // Add a sibling to the last child.
                last_child.base()->ptr.set_sibling(child);
                // child is now the last child.
                last_child.set_sibling(child);
            }
            else
            {
                // We're adding the first child of a node, which is also the last child.
                last_child.set_sibling(child);

                if (last_child.base() == prod + 1)
                {
                    // The first child is stored adjacent.
                    prod->first_child_adjacent = true;
                    prod->first_child_type     = last_child.type() & 0b1;
                }
                else
                {
                    // The child is not stored immediately afterwards.
                    // This only happens when a new block had to be started.
                    // In that case, we've saved enough space after the production to add a pointer.
                    auto memory = static_cast<void*>(prod + 1);
                    ::new (memory) _detail::pt_node_ptr<Reader>(last_child);

                    prod->first_child_adjacent = false;
                }
            }
        }

        void finish()
        {
            if (last_child)
                // The pointer of the last child needs to point back to prod.
                last_child.base()->ptr.set_parent(prod);
        }
    } _cur;
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::node_kind
{
public:
    bool is_token() const noexcept
    {
        return _ptr.token() != nullptr;
    }
    bool is_production() const noexcept
    {
        return _ptr.production() != nullptr;
    }

    bool is_root() const noexcept
    {
        // Root node has no next pointer.
        return !_ptr.base()->ptr;
    }
    bool is_token_production() const noexcept
    {
        return is_production() && _ptr.production()->token_production;
    }

    const char* name() const noexcept
    {
        if (auto prod = _ptr.production())
            return prod->name;
        else if (auto token = _ptr.token())
            return token_kind<TokenKind>::from_raw(token->kind).name();
        else
        {
            LEXY_ASSERT(false, "unreachable");
            return nullptr;
        }
    }

    friend bool operator==(node_kind lhs, node_kind rhs)
    {
        if (lhs.is_token() && rhs.is_token())
            return lhs._ptr.token()->kind == rhs._ptr.token()->kind;
        else
            // See the `operator==` for productions for rationale why this works.
            return lhs._ptr.production()->name == rhs._ptr.production()->name;
    }
    friend bool operator!=(node_kind lhs, node_kind rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator==(node_kind nk, token_kind<TokenKind> tk)
    {
        if (auto token = nk._ptr.token())
            return token_kind<TokenKind>::from_raw(token->kind) == tk;
        else
            return false;
    }
    friend bool operator==(token_kind<TokenKind> tk, node_kind nk)
    {
        return nk == tk;
    }
    friend bool operator!=(node_kind nk, token_kind<TokenKind> tk)
    {
        return !(nk == tk);
    }
    friend bool operator!=(token_kind<TokenKind> tk, node_kind nk)
    {
        return !(nk == tk);
    }

    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator==(node_kind nk, Production)
    {
        auto name = lexy::production_name<Production>();
        // We can safely compare pointers, strings are necessarily interned:
        // if Production::name exists: same address for all types,
        // otherwise we use __PRETTY_FUNCTION__ (or equivalent), which is a function-local static.
        //
        // This only fails if we have different productions with the same name and the compiler does
        // string interning. But as the production name corresponds to the qualified C++ name (by
        // default), this is only possible if the user does something weird.
        return nk.is_production() && nk._ptr.production()->name == name;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator==(Production p, node_kind nk)
    {
        return nk == p;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator!=(node_kind nk, Production p)
    {
        return !(nk == p);
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator!=(Production p, node_kind nk)
    {
        return !(nk == p);
    }

private:
    explicit node_kind(_detail::pt_node_ptr<Reader> ptr) : _ptr(ptr) {}

    _detail::pt_node_ptr<Reader> _ptr;

    friend parse_tree::node;
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::node
{
public:
    void* address() const noexcept
    {
        return _ptr.base();
    }

    auto kind() const noexcept
    {
        return node_kind(_ptr);
    }

    auto parent() const noexcept
    {
        if (kind().is_root())
            // The root has itself as parent.
            return *this;

        // If we follow the sibling pointer, we reach a parent pointer.
        auto cur = _ptr.base()->ptr;
        while (cur.is_sibling_ptr())
            cur = cur.base()->ptr;
        return node(cur);
    }

    class children_range
    {
    public:
        class iterator;
        struct sentinel : _detail::sentinel_base<sentinel, iterator>
        {};

        class iterator : public _detail::forward_iterator_base<iterator, node, node, void>
        {
        public:
            iterator() noexcept : _cur() {}

            node deref() const noexcept
            {
                LEXY_PRECONDITION(*this != sentinel{});
                return node(_cur);
            }

            void increment() noexcept
            {
                LEXY_PRECONDITION(*this != sentinel{});
                _cur = _cur.base()->ptr;
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur.base() == rhs._cur.base();
            }
            bool is_end() const noexcept
            {
                // We're at the end of the children, if the current pointer is a parent pointer.
                // For a default constructed iterator, nullptr is a parent pointer, so this works as
                // well.
                return _cur.is_parent_ptr();
            }

        private:
            explicit iterator(_detail::pt_node_ptr<Reader> ptr) noexcept : _cur(ptr) {}

            _detail::pt_node_ptr<Reader> _cur;

            friend children_range;
        };

        bool empty() const noexcept
        {
            return _count == 0;
        }

        std::size_t size() const noexcept
        {
            return _count;
        }

        iterator begin() const noexcept
        {
            return iterator(_begin);
        }
        sentinel end() const noexcept
        {
            return {};
        }

    private:
        explicit children_range(_detail::pt_node_ptr<Reader> begin, std::size_t count)
        : _begin(begin), _count(count)
        {}

        _detail::pt_node_ptr<Reader> _begin;
        std::size_t                  _count;

        friend node;
    };

    auto children() const noexcept
    {
        if (auto prod = _ptr.production())
            return children_range(prod->first_child(), prod->child_count);
        else
            return children_range(_detail::pt_node_ptr<Reader>{}, 0);
    }

    class sibling_range
    {
    public:
        class iterator : public _detail::forward_iterator_base<iterator, node, node, void>
        {
        public:
            iterator() noexcept : _cur() {}

            node deref() const noexcept
            {
                return node(_cur);
            }

            void increment() noexcept
            {
                if (_cur.base()->ptr.is_parent_ptr())
                    // We're pointing to the parent, go to first child instead.
                    _cur = _cur.base()->ptr.production()->first_child();
                else
                    // We're pointing to a sibling, go there.
                    _cur = _cur.base()->ptr;
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur.base() == rhs._cur.base();
            }

        private:
            explicit iterator(_detail::pt_node_ptr<Reader> ptr) noexcept : _cur(ptr) {}

            _detail::pt_node_ptr<Reader> _cur;

            friend sibling_range;
        };

        bool empty() const noexcept
        {
            return begin() == end();
        }

        iterator begin() const noexcept
        {
            // We begin with the next node after ours.
            // If we don't have siblings, this is our node itself.
            return ++iterator(_node);
        }
        iterator end() const noexcept
        {
            // We end when we're back at the node.
            return iterator(_node);
        }

    private:
        explicit sibling_range(_detail::pt_node_ptr<Reader> node) noexcept : _node(node) {}

        _detail::pt_node_ptr<Reader> _node;

        friend node;
    };

    auto siblings() const noexcept
    {
        return sibling_range(_ptr);
    }

    bool is_last_child() const noexcept
    {
        // We're the last child if our pointer points to the parent.
        return _ptr.base()->ptr.is_parent_ptr();
    }

    auto lexeme() const noexcept
    {
        if (auto token = _ptr.token())
            return lexy::lexeme<Reader>(token->begin, token->end());
        else
            return lexy::lexeme<Reader>();
    }

    auto token() const noexcept
    {
        LEXY_PRECONDITION(kind().is_token());

        auto token = _ptr.token();
        auto kind  = token_kind<TokenKind>::from_raw(token->kind);
        return lexy::token<Reader, TokenKind>(kind, token->begin, token->end());
    }

    friend bool operator==(node lhs, node rhs) noexcept
    {
        return lhs._ptr.base() == rhs._ptr.base();
    }
    friend bool operator!=(node lhs, node rhs) noexcept
    {
        return lhs._ptr.base() != rhs._ptr.base();
    }

private:
    explicit node(_detail::pt_node_ptr<Reader> ptr) noexcept : _ptr(ptr) {}
    explicit node(_detail::pt_node_production<Reader>* ptr) noexcept
    {
        // It doesn't matter whether the pointer is a parent or sibling.
        _ptr.set_parent(ptr);
    }

    _detail::pt_node_ptr<Reader> _ptr;

    friend parse_tree;
};

enum class traverse_event
{
    /// We're visiting a production node before all its children.
    enter,
    /// We're visiting a production node after all its children.
    exit,
    /// We're visiting a token.
    leaf,
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::traverse_range
{
public:
    struct _value_type
    {
        traverse_event   event;
        parse_tree::node node;
    };

    class iterator : public _detail::forward_iterator_base<iterator, _value_type, _value_type, void>
    {
    public:
        iterator() noexcept = default;
        iterator(traverse_event ev, node n) noexcept
        {
            LEXY_PRECONDITION(!n.kind().is_token() || ev == traverse_event::leaf);
            if (ev == traverse_event::exit)
                _cur.set_parent(n._ptr.production());
            else
                _cur.set_sibling(n._ptr.base(), n._ptr.type());
        }

        _value_type deref() const noexcept
        {
            if (_cur.token())
                // We're only visiting tokens once.
                return {traverse_event::leaf, node(_cur)};
            else if (_cur.is_sibling_ptr())
                // If it's a sibling pointer, we're entering the production for the first time.
                return {traverse_event::enter, node(_cur)};
            else if (_cur.is_parent_ptr())
                // If it's a parent pointer, we're revisiting the production after all the children.
                return {traverse_event::exit, node(_cur)};
            else
            {
                LEXY_ASSERT(false, "unreachable");
                return {{}, node(_cur)};
            }
        }

        void increment() noexcept
        {
            if (_cur.token())
                // We're currently pointing to a token.
                // Continue with its sibling.
                _cur = _cur.base()->ptr;
            else if (_cur.is_sibling_ptr())
                // We're currently pointing to a production for the first time.
                // Continue to the first child.
                _cur = _cur.production()->first_child();
            else if (_cur.is_parent_ptr())
                // We're currently pointing back to the parent production.
                // We continue with its sibling.
                _cur = _cur.base()->ptr;
            else
                LEXY_ASSERT(false, "unreachable");
        }

        bool equal(iterator rhs) const noexcept
        {
            // We need to point to the same node and in the same role.
            return _cur.base() == rhs._cur.base()
                   && _cur.is_parent_ptr() == rhs._cur.is_parent_ptr();
        }

    private:
        _detail::pt_node_ptr<Reader> _cur;
    };

    bool empty() const noexcept
    {
        return _begin == _end;
    }

    iterator begin() const noexcept
    {
        return _begin;
    }

    iterator end() const noexcept
    {
        return _end;
    }

private:
    traverse_range() noexcept = default;
    traverse_range(node n) noexcept
    {
        if (n.kind().is_token())
        {
            _begin = iterator(traverse_event::leaf, n);
            _end   = _begin;
            ++_end;
        }
        else
        {
            _begin = iterator(traverse_event::enter, n);
            _end   = iterator(traverse_event::exit, n);
            ++_end;
        }
    }

    iterator _begin, _end;

    friend parse_tree;
};
} // namespace lexy

//=== parse_as_tree ===//
namespace lexy
{
template <typename Tree, typename Input, typename Callback>
class _pt_handler
{
public:
    explicit _pt_handler(Tree& tree, const Input& input, const Callback& cb)
    : _tree(&tree), _depth(0), _validate(input, cb)
    {}

    constexpr auto get_result(bool did_recover) &&
    {
        return LEXY_MOV(_validate).get_result(did_recover);
    }

    //=== handler functions ===//
    template <typename Production>
    using return_type_for = void;

    template <typename Production>
    constexpr auto get_sink(Production)
    {
        return noop.sink();
    }

    struct _state_t
    {
        typename Tree::builder::production_state     builder_state;
        typename lexy::input_reader<Input>::iterator pos;
    };

    template <typename Production, typename Iterator>
    constexpr _state_t start_production(Production prod, Iterator pos)
    {
        if (_depth++ == 0)
        {
            _builder.emplace(LEXY_MOV(*_tree), prod);
            return {{}, pos};
        }
        else
            return {_builder->start_production(prod), pos};
    }

    template <typename Kind, typename Iterator>
    constexpr void token(Kind kind, Iterator begin, Iterator end)
    {
        _builder->token(kind, begin, end);
    }

    template <typename Production, typename... Args>
    constexpr void finish_production(Production, _state_t&& state, Args&&...)
    {
        if (--_depth == 0)
            // Finish tree instead of production.
            *_tree = LEXY_MOV(*_builder).finish();
        else
            _builder->finish_production(LEXY_MOV(state.builder_state));
    }
    template <typename Production>
    constexpr void backtrack_production(Production, _state_t&& state)
    {
        if (--_depth == 0)
            // Clear tree instead of finishing production.
            _tree->clear();
        else
            _builder->backtrack_production(LEXY_MOV(state.builder_state));
    }

    template <typename Production, typename Error>
    constexpr void error(Production p, _state_t&& state, Error&& error)
    {
        _validate.error(p, state.pos, LEXY_FWD(error));
    }

private:
    lexy::_detail::lazy_init<typename Tree::builder> _builder;
    Tree*                                            _tree;
    int                                              _depth;

    lexy::validate_handler<Input, Callback> _validate;
};

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = _pt_handler(tree, input, LEXY_MOV(callback));
    auto reader  = input.reader();

    auto did_recover = lexy::_detail::parse_impl<Production>(handler, reader);
    return LEXY_MOV(handler).get_result(static_cast<bool>(did_recover));
}
} // namespace lexy

#endif // LEXY_PARSE_TREE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_CFILE_HPP_INCLUDED
#define LEXY_EXT_CFILE_HPP_INCLUDED

#include <cstdio>
// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_BUFFER_BUILDER_HPP_INCLUDED
#define LEXY_DETAIL_BUFFER_BUILDER_HPP_INCLUDED

#include <cstring>



#include <new>

namespace lexy::_detail
{
// Builds a buffer: it has a read are and a write area.
// The characters in the read area are already valid and can be read.
// The characters in the write area are not valid, but can be written too.
template <typename T>
class buffer_builder
{
    static_assert(std::is_trivial_v<T>);

    static constexpr std::size_t total_size_bytes = 1024;
    static constexpr std::size_t stack_buffer_size
        = (total_size_bytes - 3 * sizeof(T*)) / sizeof(T);
    static constexpr auto growth_factor = 2;

public:
    buffer_builder() noexcept : _data(_stack_buffer), _read_size(0), _write_size(stack_buffer_size)
    {
        static_assert(sizeof(*this) == total_size_bytes, "invalid buffer size calculation");
    }

    ~buffer_builder() noexcept
    {
        // Free memory if we allocated any.
        if (_data != _stack_buffer)
            ::operator delete(_data);
    }

    buffer_builder(const buffer_builder&) = delete;
    buffer_builder& operator=(const buffer_builder&) = delete;

    // The total capacity: read + write.
    std::size_t capacity() const noexcept
    {
        return _read_size + _write_size;
    }

    // The read area.
    const T* read_data() const noexcept
    {
        return _data;
    }
    std::size_t read_size() const noexcept
    {
        return _read_size;
    }

    // The write area.
    T* write_data() noexcept
    {
        return _data + _read_size;
    }
    std::size_t write_size() const noexcept
    {
        return _write_size;
    }

    // Clears the read area.
    void clear() noexcept
    {
        _write_size += _read_size;
        _read_size = 0;
    }

    // Takes the first n characters of the write area and appends them to the read area.
    void commit(std::size_t n) noexcept
    {
        LEXY_PRECONDITION(n <= _write_size);
        _read_size += n;
        _write_size -= n;
    }

    // Increases the write area, invalidates all pointers.
    void grow()
    {
        const auto cur_cap = capacity();
        const auto new_cap = growth_factor * cur_cap;

        // Allocate new memory.
        auto memory = static_cast<T*>(::operator new(new_cap * sizeof(T)));
        // Copy the read area into the new memory.
        std::memcpy(memory, _data, _read_size);

        // Release the old memory, if there was any.
        if (_data != _stack_buffer)
            ::operator delete(_data);

        // Update for the new area.
        _data = memory;
        // _read_size hasn't been changed
        _write_size = new_cap - _read_size;
    }

    //=== iterator ===//
    // Stable iterator over the memory.
    class stable_iterator : public forward_iterator_base<stable_iterator, const T>
    {
    public:
        constexpr stable_iterator() = default;

        explicit constexpr stable_iterator(const _detail::buffer_builder<T>& buffer,
                                           std::size_t                       idx) noexcept
        : _buffer(&buffer), _idx(idx)
        {}

        constexpr const T& deref() const noexcept
        {
            LEXY_PRECONDITION(_idx != _buffer->read_size());
            return _buffer->read_data()[_idx];
        }

        constexpr void increment() noexcept
        {
            LEXY_PRECONDITION(_idx != _buffer->read_size());
            ++_idx;
        }

        constexpr bool equal(stable_iterator rhs) const noexcept
        {
            LEXY_PRECONDITION(_buffer == rhs._buffer);
            return _idx == rhs._idx;
        }

    private:
        const _detail::buffer_builder<T>* _buffer = nullptr;
        std::size_t                       _idx    = 0;
    };

private:
    T*          _data;
    std::size_t _read_size;
    std::size_t _write_size;
    T           _stack_buffer[stack_buffer_size];
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_BUFFER_BUILDER_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_FILE_HPP_INCLUDED
#define LEXY_INPUT_FILE_HPP_INCLUDED




// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BUFFER_HPP_INCLUDED
#define LEXY_INPUT_BUFFER_HPP_INCLUDED

#include <cstring>





namespace lexy
{
/// Stores the input that will be parsed.
/// For encodings with spare code points, it can append an EOF sentinel.
/// This allows branch-less detection of EOF.
template <typename Encoding       = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
class buffer
{
    static constexpr auto _has_sentinel
        = std::is_same_v<typename Encoding::char_type, typename Encoding::int_type>;

public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    //=== constructors ===//
    /// Allows the creation of an uninitialized buffer that is then filled by the user.
    class builder
    {
    public:
        explicit builder(std::size_t     size,
                         MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
        : _buffer(resource)
        {
            _buffer._data = _buffer.allocate(size);
            _buffer._size = size;
        }

        char_type* data() const noexcept
        {
            return _buffer._data;
        }
        std::size_t size() const noexcept
        {
            return _buffer._size;
        }

        buffer finish() && noexcept
        {
            return LEXY_MOV(_buffer);
        }

    private:
        buffer _buffer;
    };

    constexpr buffer() noexcept : buffer(_detail::get_memory_resource<MemoryResource>()) {}

    constexpr explicit buffer(MemoryResource* resource) noexcept
    : _resource(resource), _data(nullptr), _size(0)
    {}

    explicit buffer(const char_type* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : _resource(resource), _size(size)
    {
        _data = allocate(size);
        std::memcpy(_data, data, size * sizeof(char_type));
    }
    explicit buffer(const char_type* begin, const char_type* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(begin, std::size_t(end - begin), resource)
    {}

    template <typename CharT, typename = _require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(data), size, resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }
    template <typename CharT, typename = _require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* begin, const CharT* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(begin), reinterpret_cast<const char_type*>(end),
             resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }

    template <typename View, typename = decltype(LEXY_DECLVAL(View).data())>
    explicit buffer(const View&     view,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(view.data(), view.size(), resource)
    {}

    buffer(const buffer& other) : buffer(other.data(), other.size(), other._resource.get()) {}
    buffer(const buffer& other, MemoryResource* resource)
    : buffer(other.data(), other.size(), resource)
    {}

    buffer(buffer&& other) noexcept
    : _resource(other._resource), _data(other._data), _size(other._size)
    {
        other._data = nullptr;
        other._size = 0;
    }

    ~buffer() noexcept
    {
        if (!_data)
            return;

        if constexpr (_has_sentinel)
            _resource->deallocate(_data, (_size + 1) * sizeof(char_type), alignof(char_type));
        else
            _resource->deallocate(_data, _size * sizeof(char_type), alignof(char_type));
    }

    buffer& operator=(const buffer& other)
    {
        // Create a temporary buffer that owns the same memory as other but with our resource.
        // We then move assign it to *this.
        return *this = buffer(other, _resource.get());
    }

    buffer& operator=(buffer&& other) noexcept(std::is_empty_v<MemoryResource>)
    {
        if (*_resource == *other._resource)
        {
            // We have the same resource; we can just steal other's memory.
            // We do that by swapping - when other is destroyed it will free our memory.
            _detail::swap(_data, other._data);
            _detail::swap(_size, other._size);
            return *this;
        }
        else
        {
            LEXY_PRECONDITION(!std::is_empty_v<MemoryResource>);

            // We create a copy using the right resource and swap the ownership.
            buffer copy(other, _resource.get());
            _detail::swap(_data, copy._data);
            _detail::swap(_size, copy._size);
            return *this;
        }
    }

    //=== access ===//
    const char_type* begin() const noexcept
    {
        return _data;
    }
    const char_type* end() const noexcept
    {
        return _data + _size;
    }

    const char_type* data() const noexcept
    {
        return _data;
    }

    bool empty() const noexcept
    {
        return _size == 0;
    }

    std::size_t size() const noexcept
    {
        return _size;
    }
    std::size_t length() const noexcept
    {
        return _size;
    }

    //=== input ===//
    auto reader() const& noexcept
    {
        if constexpr (_has_sentinel)
            return _sentinel_reader(_data);
        else
            return _detail::range_reader<encoding, const char_type*>(_data, _data + _size);
    }

private:
    class _sentinel_reader
    {
    public:
        using encoding         = Encoding;
        using char_type        = typename encoding::char_type;
        using iterator         = const char_type*;
        using canonical_reader = _sentinel_reader;

        bool eof() const noexcept
        {
            return *_cur == encoding::eof();
        }

        auto peek() const noexcept
        {
            // The last one will be EOF.
            return *_cur;
        }

        void bump() noexcept
        {
            ++_cur;
        }

        iterator cur() const noexcept
        {
            return _cur;
        }

    private:
        explicit _sentinel_reader(iterator begin) noexcept : _cur(begin) {}

        iterator _cur;
        friend buffer;
    };

    char_type* allocate(std::size_t size) const
    {
        if constexpr (_has_sentinel)
            ++size;

        auto memory = static_cast<char_type*>(
            _resource->allocate(size * sizeof(char_type), alignof(char_type)));
        if constexpr (_has_sentinel)
            memory[size - 1] = encoding::eof();
        return memory;
    }

    LEXY_EMPTY_MEMBER _detail::memory_resource_ptr<MemoryResource> _resource;
    char_type*                                                     _data;
    std::size_t                                                    _size;
};

template <typename CharT>
buffer(const CharT*, const CharT*) -> buffer<deduce_encoding<CharT>>;
template <typename CharT>
buffer(const CharT*, std::size_t) -> buffer<deduce_encoding<CharT>>;
template <typename View>
buffer(const View&) -> buffer<deduce_encoding<std::decay_t<decltype(*LEXY_DECLVAL(View).data())>>>;

template <typename CharT, typename MemoryResource>
buffer(const CharT*, const CharT*, MemoryResource*)
    -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename CharT, typename MemoryResource>
buffer(const CharT*, std::size_t, MemoryResource*)
    -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename View, typename MemoryResource>
buffer(const View&, MemoryResource*)
    -> buffer<deduce_encoding<std::decay_t<decltype(*LEXY_DECLVAL(View).data())>>, MemoryResource>;

//=== make_buffer ===//
template <typename Encoding, encoding_endianness Endian>
struct _make_buffer
{
    template <typename MemoryResource = _detail::default_memory_resource>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        constexpr auto native_endianness
            = LEXY_IS_LITTLE_ENDIAN ? encoding_endianness::little : encoding_endianness::big;

        using char_type = typename Encoding::char_type;
        LEXY_PRECONDITION(size % sizeof(char_type) == 0);
        auto memory = static_cast<const unsigned char*>(_memory);

        if constexpr (sizeof(char_type) == 1 || Endian == native_endianness)
        {
            // No need to deal with endianness at all.
            // The reinterpret_cast is technically UB, as we didn't create objects in memory,
            // but until std::start_lifetime_as is added, there is nothing we can do.
            return buffer<Encoding, MemoryResource>(reinterpret_cast<const char_type*>(memory),
                                                    size / sizeof(char_type), resource);
        }
        else
        {
            typename buffer<Encoding, MemoryResource>::builder builder(size / sizeof(char_type),
                                                                       resource);

            const auto end = memory + size;
            for (auto dest = builder.data(); memory != end; memory += sizeof(char_type))
            {
                constexpr auto is_char16 = std::is_same_v<char_type, char16_t>;
                constexpr auto is_char32 = std::is_same_v<char_type, char32_t>;

                // We convert each group of bytes to the appropriate value.
                if constexpr (is_char16 && Endian == encoding_endianness::little)
                    *dest++ = static_cast<char_type>((memory[0] << 0) | (memory[1] << 8));
                else if constexpr (is_char32 && Endian == encoding_endianness::little)
                    *dest++ = static_cast<char_type>((memory[0] << 0) | (memory[1] << 8)
                                                     | (memory[2] << 16) | (memory[3] << 24));
                else if constexpr (is_char16 && Endian == encoding_endianness::big)
                    *dest++ = static_cast<char_type>((memory[0] << 8) | (memory[1] << 0));
                else if constexpr (is_char32 && Endian == encoding_endianness::big)
                    *dest++ = static_cast<char_type>((memory[0] << 24) | (memory[1] << 16)
                                                     | (memory[2] << 8) | (memory[3] << 0));
                else
                    static_assert(_detail::error<Encoding>, "unhandled encoding/endianness");
            }

            return LEXY_MOV(builder).finish();
        }
    }
};
template <>
struct _make_buffer<utf8_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = _detail::default_memory_resource>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        auto memory = static_cast<const unsigned char*>(_memory);

        // We just skip over the BOM if there is one, it doesn't matter.
        if (size >= 3 && memory[0] == 0xEF && memory[1] == 0xBB && memory[2] == 0xBF)
        {
            memory += 3;
            size -= 3;
        }

        return _make_buffer<utf8_encoding, encoding_endianness::big>{}(memory, size, resource);
    }
};
template <>
struct _make_buffer<utf16_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = _detail::default_memory_resource>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        constexpr auto utf16_big    = _make_buffer<utf16_encoding, encoding_endianness::big>{};
        constexpr auto utf16_little = _make_buffer<utf16_encoding, encoding_endianness::little>{};
        auto           memory       = static_cast<const unsigned char*>(_memory);

        if (size < 2)
            return utf16_big(memory, size, resource);
        if (memory[0] == 0xFF && memory[1] == 0xFE)
            return utf16_little(memory + 2, size - 2, resource);
        else if (memory[0] == 0xFE && memory[1] == 0xFF)
            return utf16_big(memory + 2, size - 2, resource);
        else
            return utf16_big(memory, size, resource);
    }
};
template <>
struct _make_buffer<utf32_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = _detail::default_memory_resource>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        constexpr auto utf32_big    = _make_buffer<utf32_encoding, encoding_endianness::big>{};
        constexpr auto utf32_little = _make_buffer<utf32_encoding, encoding_endianness::little>{};
        auto           memory       = static_cast<const unsigned char*>(_memory);

        if (size < 4)
            return utf32_big(memory, size, resource);
        else if (memory[0] == 0xFF && memory[1] == 0xFE && memory[2] == 0x00 && memory[3] == 0x00)
            return utf32_little(memory + 4, size - 4, resource);
        else if (memory[0] == 0x00 && memory[1] == 0x00 && memory[2] == 0xFE && memory[3])
            return utf32_big(memory + 4, size - 4, resource);
        else
            return utf32_big(memory, size, resource);
    }
};

/// Creates a buffer with the specified encoding/endianness from raw memory.
template <typename Encoding, encoding_endianness Endianness>
constexpr auto make_buffer_from_raw = _make_buffer<Encoding, Endianness>{};

//=== convenience typedefs ===//
template <typename Encoding       = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
using buffer_lexeme = lexeme_for<buffer<Encoding, MemoryResource>>;

template <typename Tag, typename Encoding = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
using buffer_error = error_for<buffer<Encoding, MemoryResource>, Tag>;

template <typename Production, typename Encoding = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
using buffer_error_context = error_context<Production, buffer<Encoding, MemoryResource>>;
} // namespace lexy

#endif // LEXY_INPUT_BUFFER_HPP_INCLUDED


namespace lexy
{
/// Errors that might occur while reading the file.
enum class file_error
{
    _success,
    /// An internal OS error, such as failure to read from the file.
    os_error,
    /// The file was not found.
    file_not_found,
    /// The file cannot be opened.
    permission_denied,
};
} // namespace lexy

namespace lexy::_detail
{
using file_callback = void (*)(void* user_data, const char* memory, std::size_t size);

// Reads the entire contents of the specified file into memory.
// On success, invokes the callback before freeing the memory.
// On error, returns the error without invoking the callback.
//
// Do not change ABI, especially with different build configurations!
file_error read_file(const char* path, file_callback cb, void* user_data);
} // namespace lexy::_detail

namespace lexy
{
template <typename Encoding       = default_encoding,
          typename MemoryResource = _detail::default_memory_resource>
class read_file_result
{
public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    explicit operator bool() const noexcept
    {
        return _ec == file_error::_success;
    }

    file_error error() const noexcept
    {
        LEXY_PRECONDITION(!*this);
        return _ec;
    }

    const char_type* data() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _buffer.data();
    }
    std::size_t size() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _buffer.size();
    }

    auto reader() const& noexcept
    {
        LEXY_PRECONDITION(*this);
        return _buffer.reader();
    }

public:
    // Pretend these two don't exist.
    explicit read_file_result(file_error                               ec,
                              lexy::buffer<Encoding, MemoryResource>&& buffer) noexcept
    : _buffer(LEXY_MOV(buffer)), _ec(ec)
    {}
    explicit read_file_result(file_error ec, MemoryResource* resource) noexcept
    : _buffer(resource), _ec(ec)
    {
        LEXY_PRECONDITION(!*this);
    }

private:
    lexy::buffer<Encoding, MemoryResource> _buffer;
    file_error                             _ec;
};

/// Reads the file at the specified path into a buffer.
template <typename Encoding          = default_encoding,
          encoding_endianness Endian = encoding_endianness::bom,
          typename MemoryResource    = _detail::default_memory_resource>
auto read_file(const char*     path,
               MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    -> read_file_result<Encoding, MemoryResource>
{
    struct user_data_t
    {
        lexy::buffer<Encoding, MemoryResource> buffer;
        MemoryResource*                        resource;

        user_data_t(MemoryResource* resource) : buffer(resource), resource(resource) {}
    } user_data(resource);

    auto error = _detail::read_file(
        path,
        [](void* _user_data, const char* memory, std::size_t size) {
            auto user_data = static_cast<user_data_t*>(_user_data);

            user_data->buffer
                = lexy::make_buffer_from_raw<Encoding, Endian>(memory, size, user_data->resource);
        },
        &user_data);

    return read_file_result(error, LEXY_MOV(user_data.buffer));
}
} // namespace lexy

#endif // LEXY_INPUT_FILE_HPP_INCLUDED


namespace lexy_ext
{
/// Reads from a FILE as opposed to a path.
template <typename Encoding                = lexy::default_encoding,
          lexy::encoding_endianness Endian = lexy::encoding_endianness::bom,
          typename MemoryResource          = lexy::_detail::default_memory_resource>
auto read_file(std::FILE*      file,
               MemoryResource* resource = lexy::_detail::get_memory_resource<MemoryResource>())
    -> lexy::read_file_result<Encoding, MemoryResource>
{
    using result_type = lexy::read_file_result<Encoding, MemoryResource>;

    if (!file)
        return result_type(lexy::file_error::file_not_found, resource);
    else if (std::ferror(file))
        return result_type(lexy::file_error::os_error, resource);

    // We can't use ftell() to get file size, as the file might not be open in binary mode or is
    // stdin. So instead use a conservative loop.
    lexy::_detail::buffer_builder<char> builder;
    while (true)
    {
        const auto buffer_size = builder.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from the file,
        // commiting what we've just read.
        const auto read = std::fread(builder.write_data(), sizeof(char), buffer_size, file);
        builder.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            if (std::ferror(file))
                // We have a read error.
                return result_type(lexy::file_error::os_error, resource);

            // We should have reached the end of the file.
            LEXY_ASSERT(std::feof(file), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        builder.grow();
    }

    auto buffer = lexy::make_buffer_from_raw<Encoding, Endian>(builder.read_data(),
                                                               builder.read_size(), resource);
    return result_type(lexy::file_error::_success, LEXY_MOV(buffer));
}
} // namespace lexy_ext

#endif // LEXY_EXT_CFILE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>




// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED
#define LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED





namespace lexy_ext
{
/// Converts positions (iterators) into locations (line/column nr).
///
/// The unit for line and column numbers can be customized.
/// Every time the corresponding token matches, is the corresponding number increased.
///
/// See https://foonathan.net/2021/02/column/ for a discussion of potential units.
/// Use e.g. `lexy::dsl::code_point` and `lexy::dsl::newline` to count code points.
template <typename Input, typename TokenColumn, typename TokenLine>
class input_location_finder
{
    using engine_column = typename TokenColumn::token_engine;
    using engine_line   = typename TokenLine::token_engine;

public:
    using iterator = typename lexy::input_reader<Input>::iterator;

    /// The location in the input.
    class location
    {
    public:
        constexpr std::size_t line_nr() const noexcept
        {
            return _line;
        }
        constexpr std::size_t column_nr() const noexcept
        {
            return _column;
        }

        /// The entire line that contains the position.
        constexpr lexy::lexeme_for<Input> context() const
        {
            auto reader = _reader;
            auto begin  = reader.cur();

            // Find the end of the line.
            while (true)
            {
                if (lexy::engine_peek<engine_line>(reader))
                    break;
                else if (reader.eof())
                    break;
                else
                    reader.bump();
            }

            auto end = reader.cur();
            return lexy::lexeme_for<Input>(begin, end);
        }

    private:
        constexpr location(lexy::input_reader<Input> reader, std::size_t line, std::size_t column)
        : _reader(LEXY_MOV(reader)), _line(line), _column(column)
        {}

        // The reader starts at the beginning of the given line.
        lexy::input_reader<Input> _reader;
        std::size_t               _line, _column;

        friend input_location_finder;
    };

    constexpr explicit input_location_finder(const Input& input, TokenColumn, TokenLine)
    : _reader(input.reader())
    {}

    /// Finds the given position, starting at the anchor location.
    /// This is an optimization if you know the position is after the anchor.
    constexpr location find(iterator pos, const location& anchor) const
    {
        auto reader = anchor._reader;

        // We start at the given line in the initial column.
        std::size_t cur_line   = anchor._line;
        std::size_t cur_column = 1;
        auto        line_start = reader;

        // Find the given position.
        while (true)
        {
            if (reader.cur() == pos)
            {
                // We found the position of the error.
                break;
            }
            else if (lexy::engine_try_match<engine_line>(reader))
            {
                // We're at a new line.
                ++cur_line;
                cur_column = 1;
                line_start = reader;
            }
            else if (lexy::engine_try_match<engine_column>(reader))
            {
                // Next column.
                ++cur_column;
            }
            else if (reader.eof())
            {
                // We have an OOB error position.
                LEXY_PRECONDITION(false);
                break;
            }
            else
            {
                // Invalid column, just ignore it in the column count.
                reader.bump();
            }
        }

        // Return where we ended up.
        return location(line_start, cur_line, cur_column);
    }

    /// Finds the location of the position.
    constexpr location find(iterator pos) const
    {
        // We start at the beginning of the file with the search.
        location anchor(_reader, 1, 1);
        return find(pos, anchor);
    }

private:
    lexy::input_reader<Input> _reader;
};

/// Convenience function to find a single location.
template <typename Input, typename TokenColumn, typename TokenLine>
constexpr auto find_input_location(const Input&                                 input,
                                   typename lexy::input_reader<Input>::iterator pos,
                                   TokenColumn column, TokenLine line)
{
    input_location_finder finder(input, column, line);
    return finder.find(pos);
}
} // namespace lexy_ext

#endif // LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED


namespace lexy_ext
{
template <typename Location>
void _print_location(Location location)
{
    std::fputs("     | \n", stderr);

    std::fprintf(stderr, "%2zd:%2zd| ", location.line_nr(), location.column_nr());
    for (auto c : location.context())
        std::fputc(c, stderr);
    std::fputc('\n', stderr);
}

template <typename Location>
void _print_message_indent(Location location)
{
    std::fputs("     | ", stderr);
    for (auto i = 0u; i != location.column_nr() - 1; ++i)
        std::fputc(' ', stderr);

    // The next character printed will be under the error location of the column above.
}

// Print a generic error.
template <typename Location, typename Reader, typename Tag>
void _print_message(Location location, const lexy::error<Reader, Tag>& e)
{
    if (e.begin() == e.end())
        std::fputc('^', stderr);
    else
    {
        for (auto cur = e.begin(); cur != e.end(); ++cur)
        {
            if (cur == location.context().end())
                break; // More than one line.
            std::fputc('^', stderr);
        }
    }

    std::fprintf(stderr, " %s", e.message());
}

// Print an expected_literal error.
template <typename Location, typename Reader>
void _print_message(Location, const lexy::error<Reader, lexy::expected_literal>& e)
{
    for (auto i = 0u; i != e.index(); ++i)
        std::fputc('^', stderr);
    std::fprintf(stderr, "^ expected '%s'", reinterpret_cast<const char*>(e.string()));
}

// Print an expected_keyword error.
template <typename Location, typename Reader>
void _print_message(Location, const lexy::error<Reader, lexy::expected_keyword>& e)
{
    if (e.begin() == e.end())
        std::fputc('^', stderr);
    else
    {
        for (auto cur = e.begin(); cur != e.end(); ++cur)
            std::fputc('^', stderr);
    }

    std::fprintf(stderr, " expected keyword '%s'", reinterpret_cast<const char*>(e.string()));
}

// Print an expected_char_class error.
template <typename Location, typename Reader>
void _print_message(Location, const lexy::error<Reader, lexy::expected_char_class>& e)
{
    std::fprintf(stderr, "^ expected '%s' character", e.character_class());
}

// The error callback that prints to stderr.
constexpr auto report_error = lexy::callback([](const auto& context, const auto& error) {
    // Convert the context location and error location into line/column information.
    lexy_ext::input_location_finder finder(context.input(), lexy::dsl::ascii::character,
                                           lexy::dsl::newline);
    auto                            context_location = finder.find(context.position());
    auto                            location         = finder.find(error.position(),
                                context_location); // error position is after context position

    // Print the main error headline.
    auto prod_name = context.production();
    std::fflush(stdout);
    std::fprintf(stderr, "error: while parsing %s\n", prod_name);

    if (location.line_nr() != context_location.line_nr())
    {
        _print_location(context_location);
        _print_message_indent(context_location);
        std::fputs("^ beginning here\n", stderr);
        _print_location(location);
        _print_message_indent(location);
    }
    else
    {
        _print_location(location);
        _print_message_indent(context_location);
        for (auto i = context_location.column_nr(); i != location.column_nr(); ++i)
            std::fputc('~', stderr);
    }

    _print_message(location, error);
    std::fputs("\n------\n", stderr);
});
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

