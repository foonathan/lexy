// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED
#define LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_BASE_HPP_INCLUDED
#define LEXY_ACTION_BASE_HPP_INCLUDED

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

//=== utility traits===//
#define LEXY_MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define LEXY_FWD(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define LEXY_DECLVAL(...)                                                                          \
    reinterpret_cast<::lexy::_detail::add_rvalue_ref<__VA_ARGS__>>(*reinterpret_cast<char*>(1024))

namespace lexy::_detail
{
template <typename T>
using add_rvalue_ref = T&&;

template <typename... T>
constexpr bool error = false;

template <typename T>
constexpr void swap(T& lhs, T& rhs)
{
    T tmp = LEXY_MOV(lhs);
    lhs   = LEXY_MOV(rhs);
    rhs   = LEXY_MOV(tmp);
}

template <typename T, typename U>
constexpr bool is_decayed_same = std::is_same_v<std::decay_t<T>, std::decay_t<U>>;

template <typename T, typename Fallback>
using type_or = std::conditional_t<std::is_void_v<T>, Fallback, T>;
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
#    if defined(_MSC_VER) && !defined(__clang__)
// Currently can't handle returning strings from consteval, check back later.
#        define LEXY_HAS_CONSTEVAL 0
#    elif __cpp_consteval
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

#    define LEXY_CHAR_OF_u8 char8_t
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

#    define LEXY_CHAR_OF_u8 char
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
#        if defined(__GNUC__) && !defined(__clang__) && __GNUC__ <= 11
//           GCC <= 11 has buggy support, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=101040
#            define LEXY_HAS_EMPTY_MEMBER 0
#        elif __has_cpp_attribute(no_unique_address)
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED
#define LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED

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
#        define LEXY_ASSERT(Expr, Msg) ((Expr) ? void(0) : assert((Expr) && (Msg)))

#    endif

#else

// We don't want assertions.

#    define LEXY_PRECONDITION(Expr) static_cast<void>(sizeof(Expr))
#    define LEXY_ASSERT(Expr, Msg) static_cast<void>(sizeof(Expr))

#endif

#endif // LEXY_DETAIL_ASSERT_HPP_INCLUDED



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
    constexpr T& emplace(Args&&... args)
    {
        LEXY_PRECONDITION(!*this);

        *this = lazy_init(0, LEXY_FWD(args)...);
        return this->_value;
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

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED


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



//=== encoding classes ===//
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
    static constexpr bool is_secondary_char_type()
    {
        return false;
    }

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
};

// An encoding where the input is assumed to be valid ASCII.
struct ascii_encoding
{
    using char_type = char;
    using int_type  = char;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return false;
    }

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
};

/// An encoding where the input is assumed to be valid UTF-8.
struct utf8_encoding
{
    using char_type = LEXY_CHAR8_T;
    using int_type  = LEXY_CHAR8_T;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return std::is_same_v<OtherCharType, char>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // 0xFF is not part of valid UTF-8.
        return int_type(0xFF);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};

/// An encoding where the input is assumed to be valid UTF-16.
struct utf16_encoding
{
    using char_type = char16_t;
    using int_type  = std::int_least32_t;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return sizeof(wchar_t) == sizeof(char16_t) && std::is_same_v<OtherCharType, wchar_t>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // Every value of char16_t is valid UTF16.
        return int_type(-1);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};

/// An encoding where the input is assumed to be valid UTF-32.
struct utf32_encoding
{
    using char_type = char32_t;
    using int_type  = char32_t;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return sizeof(wchar_t) == sizeof(char32_t) && std::is_same_v<OtherCharType, wchar_t>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // The highest unicode code point is U+10'FFFF, so this is never a valid code point.
        return int_type(0xFFFF'FFFF);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return c;
    }
};

/// An encoding where the input is just raw bytes, not characters.
struct byte_encoding
{
    using char_type = unsigned char;
    using int_type  = int;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return std::is_same_v<OtherCharType, char> || std::is_same_v<OtherCharType, std::byte>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        return -1;
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};
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
constexpr bool _is_compatible_char_type
    = std::is_same_v<typename Encoding::char_type,
                     CharT> || Encoding::template is_secondary_char_type<CharT>();

template <typename Encoding, typename CharT>
using _require_secondary_char_type
    = std::enable_if_t<Encoding::template is_secondary_char_type<CharT>()>;

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
      || Reader::encoding::template is_secondary_char_type<CharT>();

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

#ifndef LEXY_GRAMMAR_HPP_INCLUDED
#define LEXY_GRAMMAR_HPP_INCLUDED


// Copyright (C) 2020-2021 Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_DETECT_HPP_INCLUDED
#define LEXY_DETAIL_DETECT_HPP_INCLUDED



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

// We use a shorthand namespace to decrease symbol size.
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
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;
template <typename T>
constexpr bool is_token_rule = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_branch_rule = [] {
    if constexpr (is_rule<T>)
        return T::is_branch;
    else
        return false;
}();
} // namespace lexy

//=== production ===//
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

template <typename Production>
LEXY_CONSTEVAL const char* production_name()
{
    return _detail::type_name<Production>();
}
} // namespace lexy

namespace lexy
{
template <typename Production>
using _detect_whitespace = decltype(Production::whitespace);

template <typename Production, typename Root>
auto _production_whitespace()
{
    if constexpr (is_token_production<Production>)
    {
        // Token productions don't have whitespace.
        return;
    }
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Production>)
    {
        // We have whitespace defined in the production.
        return Production::whitespace;
    }
    else if constexpr (lexy::_detail::is_detected<_detect_whitespace, Root>)
    {
        // We have whitespace defined in the root.
        return Root::whitespace;
    }

    // If we didn't have any cases, function returns void.
}
template <typename Production, typename Root>
using production_whitespace = decltype(_production_whitespace<Production, Root>());
} // namespace lexy

namespace lexy
{
template <typename Production>
struct production_value
{
    static constexpr auto get = Production::value;
    using type                = std::decay_t<decltype(get)>;
};
} // namespace lexy

#endif // LEXY_GRAMMAR_HPP_INCLUDED



#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

#ifdef LEXY_IGNORE_DEPRECATED_ERROR
#    define LEXY_DEPRECATED_ERROR(msg)
#else
#    define LEXY_DEPRECATED_ERROR(msg) [[deprecated(msg)]]
#endif

//=== parse_events ===//
namespace lexy::parse_events
{
struct _production_event
{};

/// Start of the given production.
/// Arguments: position
/// Returns: new marker
template <typename Production>
struct production_start : _production_event
{};
/// End of the given production.
/// Arguments: position, values
/// Returns: value produced  by production.
template <typename Production>
struct production_finish : _production_event
{};
/// Production is canceled.
/// Arguments: position
template <typename Production>
struct production_cancel : _production_event
{};

/// A parse error occurrs.
/// Arguments: error object
struct error
{};

/// A token was consumed.
/// Arguments: kind, begin, end
struct token
{};
/// Beginning of a list.
/// Arguments: position
/// Returns sink.
struct list
{};

/// The input backtracked from end to begin.
/// Only meaningful for begin != end.
/// Arguments: begin, end
struct backtracked
{};

/// Non-trivial error recovery started,
/// i.e. it is currently discarding input.
/// Arguments: position
struct recovery_start
{};

/// Non-trivial error recovery succeeded.
/// It will now continue with normal parsing.
/// Arguments: position
struct recovery_finish
{};

/// Non-trivial error recovery failed because it reaches the limit.
/// It will now cancel until the next recovery point.
/// Arguments: position
struct recovery_cancel
{};
} // namespace lexy::parse_events

namespace lexyd
{
namespace _ev = lexy::parse_events;
}

//=== parse_context_var ===//
namespace lexy::_detail
{
template <typename Parent, typename Id, typename T>
class parse_context_var
{
public:
    constexpr explicit parse_context_var(Parent& parent, Id, T&& value)
    : _parent(&parent), _value(LEXY_MOV(value))
    {}

    //=== parse context ===//
    using handler         = typename Parent::handler;
    using production      = typename Parent::production;
    using root_production = typename Parent::root_production;

    constexpr auto& production_context()
    {
        return _parent->production_context();
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
    {
        return production_context().on(ev, LEXY_FWD(args)...);
    }

    //=== context variables ===//
    template <typename Id2>
    static LEXY_CONSTEVAL bool contains([[maybe_unused]] Id2 id)
    {
        if constexpr (std::is_same_v<Id, Id2>)
            return true;
        else
            return Parent::contains(id);
    }

    template <typename Id2>
    constexpr auto& get([[maybe_unused]] Id2 id)
    {
        if constexpr (std::is_same_v<Id2, Id>)
            return _value;
        else
            return _parent->get(id);
    }

private:
    Parent* _parent;
    T       _value;
};

} // namespace lexy::_detail

//=== parser ===//
namespace lexy
{
template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;

enum class rule_try_parse_result
{
    ok          = int(true),
    canceled    = int(false),
    backtracked = 2,
};

/// A final parser that drops all arguments; creating an empty result.
template <typename Context>
struct discard_parser
{
    template <typename NewContext, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(NewContext&, Reader&, Args&&...)
    {
        static_assert(sizeof...(Args) == 0, "discarded rule must not produce any values");
        static_assert(std::is_same_v<Context, NewContext>,
                      "discarded rule cannot add state to the context");
        return true;
    }
};

// Same as the other overload, but raises the event.
template <typename Matcher, typename Context, typename Reader>
constexpr bool engine_peek(Context& context, Reader reader)
{
    auto begin = reader.cur();
    auto ec    = Matcher::match(reader);
    auto end   = reader.cur();

    context.on(parse_events::backtracked{}, begin, end);
    return ec == typename Matcher::error_code{};
}
} // namespace lexy

//=== whitespace ===//
namespace lexy::_detail
{
template <typename NextParser>
struct automatic_ws_parser;
}

namespace lexy
{
template <typename Context, typename NextParser,
          typename = lexy::production_whitespace<typename Context::production,
                                                 typename Context::root_production>>
struct whitespace_parser : _detail::automatic_ws_parser<NextParser>
{};
// If we know the whitespace rule is void, go to NextParser immediately.
// This is both an optimization and also doesn't require inclusion of whitespace.hpp.
template <typename Context, typename NextParser>
struct whitespace_parser<Context, NextParser, void> : NextParser
{};
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED



//=== parse_context ===//
namespace lexy::_detail
{
template <typename Handler, typename Production>
using handler_production_result = typename Handler::template production_result<Production>;

template <typename Handler, typename Production>
using handler_marker = typename Handler::template marker<Production>;

template <typename Handler, typename Production, typename RootProduction>
class parse_context
{
public:
    //=== parse context ===//
    using handler         = Handler;
    using production      = Production;
    using root_production = RootProduction;

    constexpr auto& production_context()
    {
        return *this;
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args) -> std::enable_if_t<
        !std::is_base_of_v<parse_events::_production_event, Event>,
        decltype(LEXY_DECLVAL(Handler&).on(LEXY_DECLVAL(const handler_marker<Handler, Production>&),
                                           ev, LEXY_FWD(args)...))>
    {
        LEXY_ASSERT(_handler, "using already finished context");
        return _handler->on(_marker, ev, LEXY_FWD(args)...);
    }

    //=== context variables ===//
    template <typename Id>
    static LEXY_CONSTEVAL bool contains(Id)
    {
        return false;
    }

    template <typename Id>
    constexpr auto get(Id)
    {
        static_assert(lexy::_detail::error<Id>, "context does not contain a variable with that id");
        return nullptr;
    }

#ifndef _MSC_VER
private:
    // MSVC is a bad compiler and should feel bad.
    // Or at least fix their friend declarations.
#endif

    template <typename Iterator>
    constexpr explicit parse_context(Handler& handler, Iterator begin)
    : _handler(&handler), _marker(handler.on(parse_events::production_start<Production>{}, begin))
    {}

    template <typename ChildProduction, typename Iterator>
    constexpr auto on(parse_events::production_start<ChildProduction>, Iterator begin)
    {
        // If the new production is a token production, need to re-root it.
        using new_root = std::conditional_t<lexy::is_token_production<ChildProduction>,
                                            ChildProduction, root_production>;
        return parse_context<Handler, ChildProduction, new_root>(*_handler, begin);
    }

    template <typename Iterator, typename... Args>
    constexpr void on(parse_events::production_finish<Production> ev, Iterator end,
                      Args&&... args) &&
    {
        using result_t = handler_production_result<Handler, Production>;
        if constexpr (std::is_void_v<result_t>)
        {
            _handler->on(LEXY_MOV(_marker), ev, end, LEXY_FWD(args)...);
            _result.emplace();
        }
        else
        {
            _result.emplace(_handler->on(LEXY_MOV(_marker), ev, end, LEXY_FWD(args)...));
        }

        _handler = nullptr; // invalidate
    }

    template <typename Iterator>
    constexpr void on(parse_events::production_cancel<Production> ev, Iterator pos) &&
    {
        _handler->on(LEXY_MOV(_marker), ev, pos);
        _handler = nullptr; // invalidate
    }

    Handler*                                                  _handler;
    handler_marker<Handler, Production>                       _marker;
    lazy_init<handler_production_result<Handler, Production>> _result;

    template <typename, typename, typename>
    friend class parse_context;

    friend struct final_parser;
    template <typename, typename>
    friend struct production_parser;
    template <typename P, typename H, typename Reader>
    friend constexpr auto action_impl(H& handler, Reader& reader)
        -> lazy_init<handler_production_result<H, P>>;
};
} // namespace lexy::_detail

//=== do_action ===//
namespace lexy::_detail
{
struct final_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        using event = parse_events::production_finish<typename Context::production>;
        LEXY_MOV(context.production_context()).on(event{}, reader.cur(), LEXY_FWD(args)...);
        return true;
    }
};

template <typename Production, typename Context, typename Reader>
constexpr bool parse_production(Context& context, Reader& reader)
{
    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, final_parser>::parse(context, reader);
}

template <typename Production, typename Context, typename Reader>
constexpr auto try_parse_production(Context& context, Reader& reader)
{
    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, final_parser>::try_parse(context, reader);
}

template <typename Production, typename NextParser>
struct production_parser
{
    struct _continuation
    {
        template <typename Context, typename Reader, typename Handler, typename Root,
                  typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                 parse_context<Handler, Production, Root>& new_context,
                                 Args&&... args)
        {
            // Might need to skip whitespace, according to the original context.
            using continuation
                = std::conditional_t<lexy::is_token_production<Production>,
                                     lexy::whitespace_parser<Context, NextParser>, NextParser>;

            // Pass the produced value to the next parser.
            using result_t = handler_production_result<Handler, Production>;
            if constexpr (std::is_void_v<result_t>)
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            else
                return continuation::parse(context, reader, LEXY_FWD(args)...,
                                           LEXY_MOV(*new_context._result));
        }
    };
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto new_context
            = context.production_context().on(parse_events::production_start<Production>{},
                                              reader.cur());
        if (parse_production<Production>(new_context, reader))
        {
            // Extract the value and continue.
            return _continuation::parse(context, reader, new_context, LEXY_FWD(args)...);
        }
        else
        {
            // We had an error, cancel the production.
            LEXY_MOV(new_context).on(parse_events::production_cancel<Production>{}, reader.cur());
            return false;
        }
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
        -> lexy::rule_try_parse_result
    {
        auto new_context
            = context.production_context().on(parse_events::production_start<Production>{},
                                              reader.cur());
        if (auto result = try_parse_production<Production>(new_context, reader);
            result == lexy::rule_try_parse_result::ok)
        {
            // Extract the value and continue.
            return _continuation::parse(context, reader, new_context, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }
        else
        {
            // We had an error, cancel the production.
            LEXY_MOV(new_context).on(parse_events::production_cancel<Production>{}, reader.cur());
            return result;
        }
    }
};

template <typename Production, typename Handler, typename Reader>
constexpr auto action_impl(Handler& handler, Reader& reader)
    -> lazy_init<handler_production_result<Handler, Production>>
{
    parse_context<Handler, Production, Production> context(handler, reader.cur());

    if (!parse_production<Production>(context, reader))
    {
        // We had an error, cancel the production.
        LEXY_ASSERT(!context._result, "result must be empty on cancel");
        LEXY_MOV(context).on(parse_events::production_cancel<Production>{}, reader.cur());
    }

    return LEXY_MOV(context._result);
}
} // namespace lexy::_detail

namespace lexy
{
template <typename Production, typename Handler, typename Reader>
constexpr auto do_action(Handler&& handler, Reader& reader)
{
    static_assert(!std::is_reference_v<Handler>, "need to move handler in");
    auto result = _detail::action_impl<Production>(handler, reader);
    if (result)
    {
        using result_t = _detail::handler_production_result<Handler, Production>;
        if constexpr (std::is_void_v<result_t>)
            return LEXY_MOV(handler).template get_result_value<Production>();
        else
            return LEXY_MOV(handler).template get_result_value<Production>(LEXY_MOV(*result));
    }
    else
    {
        return LEXY_MOV(handler).template get_result_empty<Production>();
    }
}
} // namespace lexy

#endif // LEXY_ACTION_BASE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_VALIDATE_HPP_INCLUDED
#define LEXY_ACTION_VALIDATE_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_BASE_HPP_INCLUDED
#define LEXY_CALLBACK_BASE_HPP_INCLUDED



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


namespace lexy
{
template <typename T>
using _detect_callback = typename T::return_type;
template <typename T>
constexpr bool is_callback = _detail::is_detected<_detect_callback, T>;

template <typename T, typename... Args>
using _detect_callback_for = decltype(LEXY_DECLVAL(const T)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_callback_for
    = _detail::is_detected<_detect_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename Context>
using _detect_callback_context = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(const Context&)]);
template <typename T, typename Context>
constexpr bool is_callback_context
    = _detail::is_detected<_detect_callback_context, T, std::decay_t<Context>>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(const T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
} // namespace lexy

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_CONTAINER_HPP_INCLUDED
#define LEXY_CALLBACK_CONTAINER_HPP_INCLUDED



namespace lexy
{
struct nullopt;

template <typename Container>
using _detect_reserve = decltype(LEXY_DECLVAL(Container&).reserve(std::size_t()));
template <typename Container>
constexpr auto _has_reserve = _detail::is_detected<_detect_reserve, Container>;

template <typename Container>
struct _list
{
    constexpr Container operator()(Container&& container) const
    {
        return LEXY_MOV(container);
    }
    constexpr Container operator()(nullopt&&) const
    {
        return Container();
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::decay_t<decltype((LEXY_DECLVAL(Container&).push_back(LEXY_FWD(args)), ...),
                                 LEXY_DECLVAL(Container))>
    {
        Container result;
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));

        // Note that we call emplace_back() for efficiency (or something),
        // but SFINAE check for push_back(), as we don't want arbitrary conversions
        // (and also emplace_back() isn't constrained).
        (result.emplace_back(LEXY_FWD(args)), ...);

        return result;
    }
    template <typename C = Container, typename... Args>
    constexpr auto operator()(const typename C::allocator_type& allocator, Args&&... args) const
        -> decltype((LEXY_DECLVAL(C&).push_back(LEXY_FWD(args)), ...), C(allocator))
    {
        Container result(allocator);
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));

        // See above.
        (result.emplace_back(LEXY_FWD(args)), ...);

        return result;
    }

    struct _sink
    {
        Container _result;

        using return_type = Container;

        template <typename C = Container, typename U>
        auto operator()(U&& obj) -> decltype(LEXY_DECLVAL(C&).push_back(LEXY_FWD(obj)))
        {
            return _result.push_back(LEXY_FWD(obj));
        }

        template <typename C = Container, typename... Args>
        auto operator()(Args&&... args)
            -> decltype(LEXY_DECLVAL(C&).emplace_back(LEXY_FWD(args)...))
        {
            return _result.emplace_back(LEXY_FWD(args)...);
        }

        Container&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{Container()};
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _sink{Container(allocator)};
    }
};

/// A callback with sink that creates a list of things (e.g. a `std::vector`, `std::list`, etc.).
/// It repeatedly calls `push_back()` and `emplace_back()`.
template <typename Container>
constexpr auto as_list = _list<Container>{};

template <typename Container>
struct _collection
{
    constexpr Container operator()(Container&& container) const
    {
        return LEXY_MOV(container);
    }
    constexpr Container operator()(nullopt&&) const
    {
        return Container();
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::decay_t<decltype((LEXY_DECLVAL(Container&).insert(LEXY_FWD(args)), ...),
                                 LEXY_DECLVAL(Container))>
    {
        Container result;
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));

        // Note that we call emplace() for efficiency (or something),
        // but SFINAE check for insert(), as we don't want arbitrary conversions
        // (and also emplace() isn't constrained).
        (result.emplace(LEXY_FWD(args)), ...);

        return result;
    }

    template <typename C = Container, typename... Args>
    constexpr auto operator()(const typename C::allocator_type& allocator, Args&&... args) const
        -> decltype((LEXY_DECLVAL(C&).insert(LEXY_FWD(args)), ...), C(allocator))
    {
        Container result(allocator);
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));

        // Note that we call emplace() for efficiency (or something),
        // but SFINAE check for insert(), as we don't want arbitrary conversions
        // (and also emplace() isn't constrained).
        (result.emplace(LEXY_FWD(args)), ...);

        return result;
    }

    struct _sink
    {
        Container _result;

        using return_type = Container;

        template <typename C = Container, typename U>
        auto operator()(U&& obj) -> decltype(LEXY_DECLVAL(C&).insert(LEXY_FWD(obj)))
        {
            return _result.insert(LEXY_FWD(obj));
        }

        template <typename C = Container, typename... Args>
        auto operator()(Args&&... args) -> decltype(LEXY_DECLVAL(C&).emplace(LEXY_FWD(args)...))
        {
            return _result.emplace(LEXY_FWD(args)...);
        }

        Container&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{Container()};
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _sink{Container(allocator)};
    }
};

/// A callback with sink that creates an unordered collection of things (e.g. a `std::set`,
/// `std::unordered_map`, etc.). It repeatedly calls `insert()` and `emplace()`.
template <typename T>
constexpr auto as_collection = _collection<T>{};
} // namespace lexy

namespace lexy
{
template <typename Container, typename Callback>
class _collect_sink
{
public:
    constexpr explicit _collect_sink(Callback callback) : _callback(LEXY_MOV(callback)) {}
    template <typename C = Container>
    constexpr explicit _collect_sink(Callback callback, const typename C::allocator_type& allocator)
    : _result(allocator), _callback(LEXY_MOV(callback))
    {}

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
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _collect_sink<Container, Callback>(_callback, allocator);
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

#endif // LEXY_CALLBACK_CONTAINER_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_NOOP_HPP_INCLUDED
#define LEXY_CALLBACK_NOOP_HPP_INCLUDED



namespace lexy
{
struct _noop
{
    using return_type = void;

    template <typename... Args>
    constexpr auto sink(const Args&...) const
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

#endif // LEXY_CALLBACK_NOOP_HPP_INCLUDED

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
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    constexpr explicit validate_handler(const Input& input, const ErrorCallback& callback)
    : _sink(_get_error_sink(callback)), _input(&input)
    {}

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr auto get_result_value() && noexcept
    {
        return validate_result<ErrorCallback>(true, LEXY_MOV(_sink).finish());
    }
    template <typename Production>
    constexpr auto get_result_empty() && noexcept
    {
        return validate_result<ErrorCallback>(false, LEXY_MOV(_sink).finish());
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {
        iterator position; // beginning of the production
    };

    template <typename Production, typename Iterator>
    constexpr marker<Production> on(parse_events::production_start<Production>, Iterator pos)
    {
        return {pos};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production> m, parse_events::error, Error&& error)
    {
        lexy::error_context err_ctx(Production{}, *_input, m.position);
        _sink(err_ctx, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

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
    return lexy::do_action<Production>(LEXY_MOV(handler), reader);
}
} // namespace lexy

#endif // LEXY_ACTION_VALIDATE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_TREE_HPP_INCLUDED
#define LEXY_PARSE_TREE_HPP_INCLUDED



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


//=== iterator algorithms ===//
namespace lexy::_detail
{
// Can't use std::is_base_of_v<std::random_access_iterator_tag, ...> without including <iterator>.
template <typename Iterator>
using _detect_random_access = decltype(LEXY_DECLVAL(Iterator) - LEXY_DECLVAL(Iterator));
template <typename Iterator>
constexpr auto is_random_access_iterator = is_detected<_detect_random_access, Iterator>;

template <typename Iterator, typename Sentinel>
constexpr std::size_t range_size(Iterator begin, Sentinel end)
{
    if constexpr (std::is_same_v<Iterator, Sentinel> && is_random_access_iterator<Iterator>)
    {
        return static_cast<std::size_t>(end - begin);
    }
    else
    {
        std::size_t result = 0;
        for (auto cur = begin; cur != end; ++cur)
            ++result;
        return result;
    }
}

template <typename Iterator>
constexpr Iterator next(Iterator iter)
{
    return ++iter;
}
template <typename Iterator>
constexpr Iterator next(Iterator iter, std::size_t n)
{
    if constexpr (is_random_access_iterator<Iterator>)
    {
        return iter + n;
    }
    else
    {
        for (auto i = 0u; i != n; ++i)
            ++iter;
        return iter;
    }
}

// Used for assertions.
template <typename Iterator>
constexpr bool precedes(Iterator first, Iterator after)
{
    if constexpr (is_random_access_iterator<Iterator>)
        return first <= after;
    else
        return true; // Don't know.
}

// Requires: begin <= end_a && begin <= end_b.
// Returns min(end_a, end_b).
template <typename Iterator>
constexpr Iterator earlier_range_end(Iterator begin, Iterator end_a, Iterator end_b)
{
    if constexpr (is_random_access_iterator<Iterator>)
    {
        LEXY_PRECONDITION(begin <= end_a && begin <= end_b);
        if (end_a <= end_b)
            return end_a;
        else
            return end_b;
    }
    else
    {
        auto cur = begin;
        while (cur != end_a && cur != end_b)
            ++cur;
        return cur;
    }
}
} // namespace lexy::_detail

//=== facade classes ===//
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
    using value_type        = std::remove_cv_t<T>;
    using reference         = Reference;
    using pointer           = lexy::_detail::type_or<Pointer, _proxy_pointer<value_type>>;
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
    static void* allocate(std::size_t bytes, std::size_t alignment)
    {
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            return ::operator new (bytes, std::align_val_t{alignment});
        else
            return ::operator new(bytes);
    }

    static void deallocate(void* ptr, std::size_t bytes, std::size_t alignment) noexcept
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

#ifndef LEXY_TOKEN_HPP_INCLUDED
#define LEXY_TOKEN_HPP_INCLUDED

#include <climits>
#include <cstdint>





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
    constexpr lexeme(iterator pos, std::size_t size) noexcept
    : _begin(pos), _end(_detail::next(pos, size))
    {}

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
    if constexpr (lexy::_detail::is_random_access_iterator<typename Reader::iterator>)
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


namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind              = UINT_LEAST16_MAX,
    whitespace_token_kind           = UINT_LEAST16_MAX - 1,
    position_token_kind             = UINT_LEAST16_MAX - 2,
    eof_token_kind                  = UINT_LEAST16_MAX - 3,
    eol_token_kind                  = UINT_LEAST16_MAX - 4,
    identifier_token_kind           = UINT_LEAST16_MAX - 5,
    _smallest_predefined_token_kind = identifier_token_kind,
};

constexpr const char* _kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";
    case whitespace_token_kind:
        return "whitespace";
    case position_token_kind:
        return "position";
    case eof_token_kind:
        return "EOF";
    case eol_token_kind:
        return "eol";
    case identifier_token_kind:
        return "identifier";
    }

    return ""; // unreachable
}
} // namespace lexy

namespace lexy
{
template <typename TokenKind, typename... Tokens>
struct _tk_map
{
    TokenKind _data[sizeof...(Tokens)];

    template <std::size_t... Idx>
    LEXY_CONSTEVAL explicit _tk_map(lexy::_detail::index_sequence<Idx...>, const TokenKind* data,
                                    TokenKind new_kind)
    // Add new kind at the end.
    : _data{data[Idx]..., new_kind}
    {}

    template <TokenKind Kind, typename Token>
    LEXY_CONSTEVAL auto map(Token) const
    {
        static_assert(lexy::is_token_rule<Token>, "cannot map non-token to token kind");
        return _tk_map<TokenKind, Tokens..., Token>(lexy::_detail::index_sequence_for<Tokens...>{},
                                                    _data, Kind);
    }

    template <typename Token>
    LEXY_CONSTEVAL auto lookup(Token) const
    {
        constexpr auto idx = [] {
            // There is an easier way to do it via fold expressions but clang 6 generates a bogus
            // warning about sequence points.
            // As such, we do the less fancy version of looking for the index in an array.
            bool is_same[]
                = {std::is_same_v<typename Token::token_type, typename Tokens::token_type>...};

            for (std::size_t idx = 0; idx != sizeof...(Tokens); ++idx)
                if (is_same[idx])
                    return idx;

            return sizeof...(Tokens);
        }();
        if constexpr (idx == sizeof...(Tokens))
            return unknown_token_kind;
        else
            return _data[idx];
    }
};

struct _tk_map_empty
{
    template <typename Token>
    static LEXY_CONSTEVAL auto lookup(Token)
    {
        return unknown_token_kind;
    }

    template <auto TokenKind, typename Token>
    LEXY_CONSTEVAL auto map(Token) const
    {
        static_assert(lexy::is_token_rule<Token>, "cannot map non-token to token kind");
        return _tk_map<std::decay_t<decltype(TokenKind)>,
                       Token>(lexy::_detail::index_sequence_for<>{}, nullptr, TokenKind);
    }
};

inline constexpr auto token_kind_map = _tk_map_empty{};

/// A mapping of token rule to token kind; specialize for your own kinds.
template <typename TokenKind>
constexpr auto token_kind_map_for = token_kind_map;

// Prevent user-defined specialization for void.
template <>
inline constexpr auto token_kind_map_for<void> = token_kind_map;
} // namespace lexy

namespace lexy
{
template <typename TokenKind>
using _detect_token_kind_name = decltype(token_kind_name(TokenKind{}));

/// What sort of token it is.
template <typename TokenKind = void>
class token_kind
{
    static_assert(std::is_void_v<TokenKind> || std::is_enum_v<TokenKind>,
                  "invalid type for TokenKind");
    using _underlying_type = lexy::_detail::type_or<TokenKind, int>;

public:
    //=== constructors ===//
    /// Creates an unknown token kind.
    constexpr token_kind() noexcept : token_kind(unknown_token_kind) {}

    /// Creates a predefined token kind.
    constexpr token_kind(predefined_token_kind value) noexcept : _value(value) {}

    /// Creates the token kind with the specified value.
    constexpr token_kind(_underlying_type value) noexcept
    : _value(static_cast<std::uint_least16_t>(value))
    {
        LEXY_PRECONDITION(_underlying_type(0) <= value
                          && value < _underlying_type(_smallest_predefined_token_kind));
    }

    /// Creates the token kind of a token rule.
    template <typename TokenRule, typename = std::enable_if_t<lexy::is_token_rule<TokenRule>>>
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        // Look for internal mapping first.
        auto token_rule_kind = TokenRule::token_kind();
        if constexpr ((std::is_enum_v<TokenKind> //
                       && std::is_same_v<decltype(token_rule_kind), TokenKind>)
                      || (std::is_void_v<TokenKind> //
                          && std::is_integral_v<decltype(token_rule_kind)>))
        {
            // The token has an associated kind of the same type.
            *this = token_kind(token_rule_kind);
        }
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
            return token_kind_name(get()); // ADL
        else
            // We only have a generic name.
            return "token";
    }

    constexpr _underlying_type get() const noexcept
    {
        return static_cast<_underlying_type>(_value);
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

template <typename TokenKind, typename = std::enable_if_t<std::is_integral_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<void>;
template <typename TokenKind, typename = std::enable_if_t<std::is_enum_v<TokenKind>>>
token_kind(TokenKind) -> token_kind<TokenKind>;
} // namespace lexy

namespace lexy
{
/// A parsed token, i.e. its kind and its lexeme.
template <typename Reader, typename TokenKind = void>
class token
{
public:
    using encoding  = typename Reader::encoding;
    using char_type = typename encoding::char_type;
    using iterator  = typename Reader::iterator;

    explicit constexpr token(token_kind<TokenKind> kind, lexy::lexeme<Reader> lex) noexcept
    : _lexeme(lex), _kind(kind)
    {
        LEXY_PRECONDITION(lex.begin() != iterator());
    }
    explicit constexpr token(token_kind<TokenKind> kind, iterator begin, iterator end) noexcept
    : token(kind, lexy::lexeme<Reader>(begin, end))
    {}

    constexpr token_kind<TokenKind> kind() const noexcept
    {
        return _kind;
    }

    constexpr const char* name() const noexcept
    {
        return _kind.name();
    }

    constexpr iterator position() const noexcept
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

template <typename Input, typename TokenKind = void>
using token_for = token<lexy::input_reader<Input>, TokenKind>;
} // namespace lexy

#endif // LEXY_TOKEN_HPP_INCLUDED


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
        // NOLINTNEXTLINE: We need pointer conversion.
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
    // If it's random access, we store size instead of end.
    static constexpr auto _optimize_end
        = _detail::is_random_access_iterator<typename Reader::iterator>;
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
    constexpr explicit parse_tree(MemoryResource* resource)
    : _buffer(resource), _root(nullptr), _size(0), _depth(0)
    {}

    //=== container access ===//
    bool empty() const noexcept
    {
        return _root == nullptr;
    }

    std::size_t size() const noexcept
    {
        return _size;
    }

    std::size_t depth() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return _depth;
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
    std::size_t                          _size;
    std::size_t                          _depth;
};

template <typename Input, typename TokenKind = void,
          typename MemoryResource = _detail::default_memory_resource>
using parse_tree_for = lexy::parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>;

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::builder
{
public:
    template <typename Production>
    explicit builder(parse_tree&& tree, Production production) : _result(LEXY_MOV(tree))
    {
        // Empty the initial parse tree.
        _result._buffer.reset();

        // Allocate a new root node.
        // No need to reserve for the initial node.
        _result._root
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _result._size = 1;

        // Begin construction at the root.
        _cur = marker(_result._root, 0);
    }
    template <typename Production>
    explicit builder(Production production) : builder(parse_tree(), production)
    {}

    struct marker
    {
        // The current production all tokens are appended to.
        _detail::pt_node_production<Reader>* prod = nullptr;
        // The depth of the current production.
        std::size_t depth = 0;
        // The last child of the current production.
        _detail::pt_node_ptr<Reader> last_child;

        marker() = default;

        explicit marker(_detail::pt_node_production<Reader>* prod, std::size_t depth)
        : prod(prod), depth(depth)
        {}

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

        void finish(std::size_t& size, std::size_t& max_depth)
        {
            if (last_child)
                // The pointer of the last child needs to point back to prod.
                last_child.base()->ptr.set_parent(prod);

            // Update the size.
            size += prod->child_count;

            // And update the depth.
            auto local_max_depth = prod->child_count > 0 ? depth + 1 : depth;
            if (max_depth < local_max_depth)
                max_depth = local_max_depth;
        }
    };

    template <typename Production>
    auto start_production(Production production)
    {
        if constexpr (lexy::is_transparent_production<Production>)
            // Don't need to add a new node for a transparent production.
            return marker();

        // Allocate a node for the production and append it to the current child list.
        // We reserve enough memory to allow for a trailing pointer.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node_ptr<Reader>));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        // Note: don't append the node yet, we might still backtrack.

        // Subsequent inertions are to the new node, so update marker and return old one.
        auto old = LEXY_MOV(_cur);
        _cur     = marker(node, old.depth + 1);
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

    void finish_production(marker&& m)
    {
        if (!m.prod)
            // We're finishing with a transparent production, do nothing.
            return;

        // We're done with the current production.
        _cur.finish(_result._size, _result._depth);
        // Append to previous production.
        m.append(_cur.prod);
        // Continue with the previous production.
        _cur = LEXY_MOV(m);
    }

    void cancel_production(marker&& m)
    {
        if (!m.prod)
            // We're backtracking a transparent production, do nothing.
            return;

        // Deallocate everything from the backtracked production.
        _result._buffer.unwind(_cur.prod);
        // Continue with previous production.
        _cur = LEXY_MOV(m);
    }

    parse_tree&& finish() &&
    {
        LEXY_PRECONDITION(_cur.prod == _result._root);
        _cur.finish(_result._size, _result._depth);
        return LEXY_MOV(_result);
    }

private:
    parse_tree _result;
    marker     _cur;
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
    using event = traverse_event;

    struct _value_type
    {
        traverse_event   event;
        parse_tree::node node;
    };

    class iterator : public _detail::forward_iterator_base<iterator, _value_type, _value_type, void>
    {
    public:
        iterator() noexcept = default;

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
            if (_cur.token() || _cur.is_parent_ptr())
                // We're currently pointing to a token or back to the parent production.
                // Continue with its sibling.
                _cur = _cur.base()->ptr;
            else if (_cur.is_sibling_ptr())
                // We're currently pointing to a production for the first time.
                // Continue to the first child.
                _cur = _cur.production()->first_child();
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

        friend traverse_range;
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
            _begin._cur.set_sibling(n._ptr.token());
            _end = _begin;
        }
        else
        {
            _begin._cur.set_sibling(n._ptr.production());
            _end._cur.set_parent(n._ptr.production());
        }

        // Turn it into a half-open range.
        ++_end;
    }

    iterator _begin, _end;

    friend parse_tree;
};
} // namespace lexy

#endif // LEXY_PARSE_TREE_HPP_INCLUDED


namespace lexy
{
template <typename Tree, typename Input, typename Callback>
class parse_tree_handler
{
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    explicit parse_tree_handler(Tree& tree, const Input& input, const Callback& cb)
    : _tree(&tree), _depth(0), _validate(input, cb)
    {}

    constexpr auto get_result(bool did_recover) &&
    {
        return LEXY_MOV(_validate).get_result(did_recover);
    }

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr auto get_result_value() && noexcept
    {
        return LEXY_MOV(_validate).template get_result_value<Production>();
    }
    template <typename Production>
    constexpr auto get_result_empty() && noexcept
    {
        return LEXY_MOV(_validate).template get_result_empty<Production>();
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {
        typename Tree::builder::marker                                                builder;
        typename lexy::validate_handler<Input, Callback>::template marker<Production> validate;
    };

    template <typename Production>
    constexpr auto on(parse_events::production_start<Production>, iterator pos)
    {
        if (_depth++ == 0)
        {
            _builder.emplace(LEXY_MOV(*_tree), Production{});
            return marker<Production>{{}, {pos}};
        }
        else
        {
            return marker<Production>{_builder->start_production(Production{}), {pos}};
        }
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename TokenKind>
    constexpr void on(const marker<Production>&, parse_events::token, TokenKind kind,
                      iterator begin, iterator end)
    {
        _builder->token(kind, begin, end);
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production> m, parse_events::error, Error&& error)
    {
        _validate.on(m.validate, parse_events::error{}, LEXY_FWD(error));
    }

    template <typename Production, typename... Args>
    constexpr void on(marker<Production>&& m, parse_events::production_finish<Production>, iterator,
                      Args&&...)
    {
        if (--_depth == 0)
            // Finish tree instead of production.
            *_tree = LEXY_MOV(*_builder).finish();
        else
            _builder->finish_production(LEXY_MOV(m.builder));
    }
    template <typename Production>
    constexpr void on(marker<Production>&& m, parse_events::production_cancel<Production>, iterator)
    {
        if (--_depth == 0)
            // Clear tree instead of production.
            _tree->clear();
        else
            _builder->cancel_production(LEXY_MOV(m.builder));
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

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
    auto handler = parse_tree_handler(tree, input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), reader);
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_TRACE_HPP_INCLUDED
#define LEXY_ACTION_TRACE_HPP_INCLUDED

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




// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VISUALIZE_HPP_INCLUDED
#define LEXY_VISUALIZE_HPP_INCLUDED

#include <cstdio>

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CODE_POINT_HPP_INCLUDED
#define LEXY_CODE_POINT_HPP_INCLUDED

#include <cstdint>



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
    constexpr bool is_ascii() const noexcept
    {
        return _value <= 0x7F;
    }
    constexpr bool is_bmp() const noexcept
    {
        return _value <= 0xFFFF;
    }
    constexpr bool is_valid() const noexcept
    {
        return _value <= 0x10'FFFF;
    }

    constexpr bool is_control() const noexcept
    {
        return _value <= 0x1F || (0x7F <= _value && _value <= 0x9F);
    }
    constexpr bool is_surrogate() const noexcept
    {
        return 0xD800 <= _value && _value <= 0xDFFF;
    }
    constexpr bool is_private_use() const noexcept
    {
        return (0xE000 <= _value && _value <= 0xF8FF)
               || (0x0F'0000 <= _value && _value <= 0x0F'FFFD)
               || (0x10'0000 <= _value && _value <= 0x10'FFFD);
    }
    constexpr bool is_noncharacter() const noexcept
    {
        // Contiguous range of 32 non-characters.
        if (0xFDD0 <= _value && _value <= 0xFDEF)
            return true;

        // Last two code points of every plane.
        auto in_plane = _value & 0xFFFF;
        return in_plane == 0xFFFE || in_plane == 0xFFFF;
    }

    constexpr bool is_scalar() const noexcept
    {
        return is_valid() && !is_surrogate();
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

#endif // LEXY_CODE_POINT_HPP_INCLUDED

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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_RANGE_INPUT_HPP_INCLUDED
#define LEXY_INPUT_RANGE_INPUT_HPP_INCLUDED





namespace lexy
{
template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class range_input
{
public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    using iterator = Iterator;
    using sentinel = Sentinel;

    //=== constructors ===//
    constexpr range_input() noexcept : _begin(), _end() {}

    constexpr range_input(iterator begin, sentinel end) noexcept : _begin(begin), _end(end) {}

    //=== access ===//
    constexpr iterator begin() const noexcept
    {
        return _begin;
    }

    constexpr sentinel end() const noexcept
    {
        return _end;
    }

    //=== reader ===//
    constexpr auto reader() const& noexcept
    {
        return _detail::range_reader<Encoding, Iterator, Sentinel>(_begin, _end);
    }

private:
    Iterator                   _begin;
    LEXY_EMPTY_MEMBER Sentinel _end;
};

template <typename Iterator, typename Sentinel>
range_input(Iterator begin, Sentinel end)
    -> range_input<deduce_encoding<std::decay_t<decltype(*begin)>>, Iterator, Sentinel>;
} // namespace lexy

#endif // LEXY_INPUT_RANGE_INPUT_HPP_INCLUDED



//=== visualization_options ===//
namespace lexy
{
enum visualization_flags
{
    visualize_default = 0,

    /// Visualization can use unicode characters.
    visualize_use_unicode = 1 << 0,
    /// Visualization can use ANSI color escape sequences.
    visualize_use_color = 1 << 1,
    /// Visualization can use Unicode symbols e.g. for newline/space instead of the code point.
    visualize_use_symbols = 1 << 2,

    /// Visualization can use unicode, color and symbols.
    visualize_fancy = visualize_use_unicode | visualize_use_color | visualize_use_symbols,

    /// Visualize space ' ' as visible character/symbol.
    visualize_space = 1 << 3,
};

constexpr visualization_flags operator|(visualization_flags lhs, visualization_flags rhs) noexcept
{
    return visualization_flags(int(lhs) | int(rhs));
}

/// Options that control visualization.
struct visualization_options
{
    static constexpr unsigned char max_tree_depth_limit = 32;

    /// Boolean flags.
    visualization_flags flags = visualize_default;

    /// The maximal depth when visualizing a tree.
    /// Must be <= max_tree_depth_limit.
    unsigned char max_tree_depth = max_tree_depth_limit;
    /// The maximal width when visualizing a lexeme.
    /// 0 means unlimited.
    unsigned char max_lexeme_width = 0;
    /// How many spaces are printed for a tab character.
    /// 0 means it is printed as escaped control character.
    unsigned char tab_width = 0;

    constexpr bool is_set(visualization_flags f) const noexcept
    {
        return (flags & f) != 0;
    }

    constexpr visualization_options reset(visualization_flags f) const noexcept
    {
        auto copy  = *this;
        copy.flags = visualization_flags(copy.flags & ~f);
        return copy;
    }

    friend constexpr visualization_options operator|(visualization_options lhs,
                                                     visualization_flags   rhs) noexcept
    {
        lhs.flags = lhs.flags | rhs;
        return lhs;
    }
};
} // namespace lexy

//=== visualize_to ===//
namespace lexy::_detail
{
template <typename Encoding>
constexpr auto make_literal_lexeme(const typename Encoding::char_type* str)
{
    struct reader
    {
        using encoding         = Encoding;
        using char_type        = typename Encoding::char_type;
        using iterator         = const char_type*;
        using canonical_reader = reader;
    };

    auto end = str;
    while (*end)
        ++end;

    return lexy::lexeme<reader>(str, end);
}

template <typename OutIt>
constexpr OutIt write_str(OutIt out, const char* str)
{
    while (*str)
        *out++ = *str++;
    return out;
}
template <typename OutIt>
constexpr OutIt write_str(OutIt out, const LEXY_CHAR8_T* str)
{
    while (*str)
        *out++ = static_cast<char>(*str++);
    return out;
}

template <int N = 16, typename OutIt, typename... Args>
constexpr OutIt write_format(OutIt out, const char* fmt, const Args&... args)
{
    char buffer[std::size_t(N + 1)];
    auto count = std::snprintf(buffer, N, fmt, args...);
    LEXY_ASSERT(count <= N, "buffer not big enough");

    for (auto i = 0; i != count; ++i)
        *out++ = buffer[i];
    return out;
}

enum class color
{
    reset  = 0,
    bold   = 1,
    faint  = 2,
    italic = 3,

    black   = 30,
    red     = 31,
    green   = 32,
    yellow  = 33,
    blue    = 34,
    magenta = 35,
    cyan    = 36,
    white   = 37,
};

template <color CodeHead, color... CodeTail, typename OutIt>
constexpr OutIt write_color(OutIt out, visualization_options opts)
{
    if (!opts.is_set(visualize_use_color))
        return out;

    out = write_str(out, "\033[");

    auto write_code = [](OutIt out, int code) {
        if (code > 10)
        {
            *out++ = static_cast<char>((code / 10) + '0');
            *out++ = static_cast<char>((code % 10) + '0');
        }
        else
        {
            *out++ = static_cast<char>(code + '0');
        }
        return out;
    };
    out = write_code(out, int(CodeHead));
    ((*out++ = ';', write_code(out, int(CodeTail))), ...);

    *out++ = 'm';
    return out;
}

template <typename OutIt>
constexpr OutIt write_ellipsis(OutIt out, visualization_options opts)
{
    if (opts.is_set(visualize_use_unicode))
        return _detail::write_str(out, u8"…");
    else
        return _detail::write_str(out, "...");
}
} // namespace lexy::_detail

namespace lexy
{
template <typename OutputIt>
OutputIt visualize_to(OutputIt out, lexy::code_point cp, visualization_options opts = {})
{
    auto write_special_char = [opts](OutputIt out, auto inner) {
        out = _detail::write_color<_detail::color::cyan, _detail::color::faint>(out, opts);
        if (opts.is_set(visualize_use_unicode))
            out = _detail::write_str(out, u8"⟨");
        else
            out = _detail::write_str(out, "\\");

        out = inner(out);

        if (opts.is_set(visualize_use_unicode))
            out = _detail::write_str(out, u8"⟩");
        out = _detail::write_color<_detail::color::reset>(out, opts);
        return out;
    };

    if (!cp.is_valid())
    {
        out = write_special_char(out, [opts](OutputIt out) {
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_str(out, "U+????");
            else
                return _detail::write_str(out, "u????");
        });
        return out;
    }
    else if (cp.is_control())
    {
        auto c = static_cast<char>(cp.value());
        switch (c)
        {
        case '\0':
            out = write_special_char(out, [opts](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_str(out, "NUL");
                else
                    return _detail::write_str(out, "0");
            });
            break;
        case '\r':
            out = write_special_char(out, [opts](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_str(out, "CR");
                else
                    return _detail::write_str(out, "r");
            });
            break;

        case '\n':
            if (opts.is_set(visualize_use_symbols))
            {
                out = _detail::write_color<_detail::color::faint>(out, opts);
                out = _detail::write_str(out, u8"⏎");
                out = _detail::write_color<_detail::color::reset>(out, opts);
            }
            else if (opts.is_set(visualize_use_unicode))
            {
                out = write_special_char(out, [](OutputIt out) {
                    return _detail::write_str(out, "LF");
                });
            }
            else
            {
                out = write_special_char(out,
                                         [](OutputIt out) { return _detail::write_str(out, "n"); });
            }
            break;
        case '\t':
            if (opts.tab_width > 0)
            {
                // We print that many space characters.
                // This is recursion, but the recursive call does not recurse further.
                for (auto i = 0u; i < opts.tab_width; ++i)
                    out = visualize_to(out, lexy::code_point(' '), opts);
            }
            else if (opts.is_set(visualize_use_symbols))
            {
                out = _detail::write_color<_detail::color::faint>(out, opts);
                out = _detail::write_str(out, u8"⇨");
                out = _detail::write_color<_detail::color::reset>(out, opts);
            }
            else if (opts.is_set(visualize_use_unicode))
            {
                out = write_special_char(out, [](OutputIt out) {
                    return _detail::write_str(out, "HT");
                });
            }
            else
            {
                out = write_special_char(out,
                                         [](OutputIt out) { return _detail::write_str(out, "t"); });
            }
            break;

        default:
            out = write_special_char(out, [opts, c](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_format(out, "U+%04X", c);
                else
                    return _detail::write_format(out, "u%04X", c);
            });
            break;
        }
        return out;
    }
    else if (cp.value() == ' ')
    {
        if (opts.is_set(visualize_space))
        {
            if (opts.is_set(visualize_use_symbols))
            {
                out = _detail::write_color<_detail::color::faint>(out, opts);
                out = _detail::write_str(out, u8"␣");
                out = _detail::write_color<_detail::color::reset>(out, opts);
            }
            else if (opts.is_set(visualize_use_unicode))
            {
                out = write_special_char(out, [](OutputIt out) {
                    return _detail::write_str(out, "SP");
                });
            }
            else
            {
                out = write_special_char(out, [](OutputIt out) {
                    return _detail::write_str(out, "u0020");
                });
            }
        }
        else
        {
            *out++ = ' ';
        }

        return out;
    }
    else if (cp.value() == '\\')
    {
        if (!opts.is_set(visualize_use_unicode))
            out = write_special_char(out,
                                     [](OutputIt out) { return _detail::write_str(out, "\\"); });
        else
            *out++ = '\\'; // Doesn't need escaping if we can use unicode.
        return out;
    }
    else if (cp.is_ascii())
    {
        // ASCII, non-control character, print as-is.
        *out++ = static_cast<char>(cp.value());
        return out;
    }
    else
    {
        out = write_special_char(out, [opts, cp](OutputIt out) {
            auto c = static_cast<int>(cp.value());
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_format(out, "U+%04X", c);
            else if (cp.is_bmp())
                return _detail::write_format(out, "u%04X", c);
            else
                return _detail::write_format(out, "U%08X", c);
        });
        return out;
    }
}

template <typename OutputIt, typename Reader>
OutputIt visualize_to(OutputIt out, lexy::lexeme<Reader> lexeme,
                      [[maybe_unused]] visualization_options opts = {})
{
    // We need to ensure that we're not printing more "code points" than `opts.max_lexeme_width`,
    // or unlimited if `opts.max_lexeme_width == 0`.
    // The trick is to count and check for `count == opts.max_lexeme_width` after increment.

    using encoding = typename Reader::encoding;
    if constexpr (std::is_same_v<encoding, lexy::ascii_encoding> //
                  || std::is_same_v<encoding, lexy::default_encoding>)
    {
        auto count = 0u;
        for (char c : lexeme)
        {
            // If the character is in fact ASCII, visualize the code point.
            // Otherwise, visualize an unknown code point.
            if (lexy::_is_ascii(c))
                out = visualize_to(out, lexy::code_point(static_cast<char32_t>(c)), opts);
            else
                out = visualize_to(out, lexy::code_point(), opts);

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>     //
                       || std::is_same_v<encoding, lexy::utf16_encoding> //
                       || std::is_same_v<encoding, lexy::utf32_encoding>)
    {
        // Parse the individual code points, and write them out.
        lexy::range_input<encoding, typename Reader::iterator> input(lexeme.begin(), lexeme.end());
        auto                                                   reader = input.reader();

        auto count = 0u;
        while (true)
        {
            lexy::engine_cp_auto::error_code ec{};
            auto                             cp = lexy::engine_cp_auto::parse(ec, reader);
            if (ec == lexy::engine_cp_auto::error_code::eof)
                break;

            out = visualize_to(out, cp, opts);

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (std::is_same_v<encoding, lexy::byte_encoding>)
    {
        // Write each byte.
        auto count = 0u;
        for (auto iter = lexeme.begin(); iter != lexeme.end(); ++iter)
        {
            if (iter != lexeme.begin())
                *out++ = ' ';
            out = _detail::write_format(out, "%02X", *iter);

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else
    {
        static_assert(lexy::_detail::error<encoding>, "unknown encoding");
        return out;
    }
}

template <typename OutputIt, typename Tree, typename = decltype(LEXY_DECLVAL(Tree&).traverse())>
OutputIt visualize_to(OutputIt out, const Tree& tree, visualization_options opts = {})
{
    struct label_t
    {
        const LEXY_CHAR_OF_u8* space;
        const LEXY_CHAR_OF_u8* line;
        const LEXY_CHAR_OF_u8* end;
        const LEXY_CHAR_OF_u8* branch;
    };
    auto label = opts.is_set(visualize_use_unicode) ? label_t{u8"   ", u8"│  ", u8"└──", u8"├──"}
                                                    : label_t{u8"  ", u8"  ", u8"- ", u8"- "};

    // True if the node currently opened at the depth is the last child of its parent,
    // false otherwise.
    bool is_last_child[visualization_options::max_tree_depth_limit] = {};
    LEXY_PRECONDITION(opts.max_tree_depth <= visualization_options::max_tree_depth_limit);

    // Writes the prefix using the last child information.
    auto write_prefix
        = [opts, label, &is_last_child](OutputIt out, std::size_t cur_depth, bool cur_is_last) {
              if (cur_depth == 0)
                  // Root node doesn't have a prefix.
                  return out;

              out = _detail::write_color<_detail::color::faint>(out, opts);

              // We begin at depth 1, as depth 0 doesn't require a prefix.
              for (auto i = 1u; i < cur_depth; ++i)
                  if (is_last_child[i])
                      // If the current node at that depth is the last child, we just indent.
                      out = _detail::write_str(out, label.space);
                  else
                      // Otherwise, we need to carry the line.
                      out = _detail::write_str(out, label.line);

              // Print the final branching symbol for the current node.
              if (cur_is_last)
                  out = _detail::write_str(out, label.end);
              else
                  out = _detail::write_str(out, label.branch);

              out = _detail::write_color<_detail::color::reset>(out, opts);
              return out;
          };

    // Traverse and print the tree.
    std::size_t cur_depth = 0;
    for (auto [event, node] : tree.traverse())
    {
        auto last_child = node.is_last_child();

        using event_t = typename decltype(tree.traverse())::event;
        switch (event)
        {
        case event_t::enter:
            if (cur_depth <= opts.max_tree_depth)
            {
                out = write_prefix(out, cur_depth, last_child);

                out = _detail::write_color<_detail::color::bold>(out, opts);
                out = _detail::write_str(out, node.kind().name());
                out = _detail::write_color<_detail::color::reset>(out, opts);

                if (cur_depth == opts.max_tree_depth)
                {
                    // Print an ellipsis instead of children.
                    out = _detail::write_str(out, ": ");
                    out = _detail::write_ellipsis(out, opts);
                    out = _detail::write_str(out, "\n");
                }
                else
                {
                    // Print a newline and prepare for children.
                    out                      = _detail::write_str(out, ":\n");
                    is_last_child[cur_depth] = last_child;
                }
            }
            ++cur_depth;
            break;

        case event_t::exit:
            --cur_depth;
            break;

        case event_t::leaf:
            if (cur_depth <= opts.max_tree_depth)
            {
                out = write_prefix(out, cur_depth, last_child);

                out = _detail::write_color<_detail::color::bold>(out, opts);
                out = _detail::write_str(out, node.kind().name());
                out = _detail::write_color<_detail::color::reset>(out, opts);

                if (!node.lexeme().empty())
                {
                    out = _detail::write_str(out, ": ");
                    out = visualize_to(out, node.lexeme(), opts | lexy::visualize_space);
                }

                out = _detail::write_str(out, "\n");
            }
            break;
        }
    }

    return out;
}
} // namespace lexy

//=== visualize ===//
namespace lexy
{
struct cfile_output_iterator
{
    std::FILE* _file;

    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    cfile_output_iterator& operator=(char c)
    {
        std::fputc(c, _file);
        return *this;
    }
};

/// Writes the visualization to the FILE.
template <typename T>
void visualize(std::FILE* file, const T& obj, visualization_options opts = {})
{
    visualize_to(cfile_output_iterator{file}, obj, opts);
}
} // namespace lexy

//=== visualization_display_width ===//
namespace lexy
{
template <typename T>
std::size_t visualization_display_width(const T& obj, visualization_options opts = {})
{
    struct iterator
    {
        std::size_t width;

        iterator& operator*() noexcept
        {
            return *this;
        }
        iterator& operator++(int) noexcept
        {
            return *this;
        }

        iterator& operator=(char c)
        {
            // We're having ASCII or UTF-8 characters.
            // All unicode characters used occupy a single cell,
            // so we just need to count all code units that are not continuation bytes.
            auto value = static_cast<unsigned char>(c);
            if ((value & 0b1100'0000) != 0b1000'0000)
                ++width;
            return *this;
        }
    };

    // Disable usage of color, as they introduce additional characters that must nobe counted.
    return visualize_to(iterator{0}, obj, opts.reset(visualize_use_color)).width;
}
} // namespace lexy

#endif // LEXY_VISUALIZE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED
#define LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_MATCH_HPP_INCLUDED
#define LEXY_ACTION_MATCH_HPP_INCLUDED




namespace lexy
{
class match_handler
{
public:
    constexpr match_handler() : _failed(false) {}

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr bool get_result_value() && noexcept
    {
        // Parsing succeeded or parsing recovered from an error.
        // Return true, if we had an error, false otherwise.
        return !_failed;
    }
    template <typename Production>
    constexpr bool get_result_empty() && noexcept
    {
        // Parsing could not recover from an error, return false.
        LEXY_ASSERT(_failed, "parsing failed without logging an error?!");
        return false;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {};

    template <typename Production, typename Iterator>
    constexpr marker<Production> on(parse_events::production_start<Production>, Iterator)
    {
        return {};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production>, parse_events::error, Error&&)
    {
        _failed = true;
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}

private:
    bool _failed;
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto reader = input.reader();
    return lexy::do_action<Production>(match_handler(), reader);
}
} // namespace lexy

#endif // LEXY_ACTION_MATCH_HPP_INCLUDED




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
            {
                context.on(_ev::backtracked{}, begin, reader.cur());
                return lexy::rule_try_parse_result::backtracked;
            }
            auto end = reader.cur();
            context.on(_ev::token{}, Derived::token_kind(), begin, end);

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
            context.on(_ev::token{}, Derived::token_kind(), position, reader.cur());

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
        context.on(_ev::error{}, err);
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
template <typename Rule>
struct _token : token_base<_token<Rule>>
{
    struct _production
    {
        static constexpr auto rule = Rule{};
    };

    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            return lexy::do_action<_production>(lexy::match_handler(), reader) ? error_code()
                                                                               : error_code::error;
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::missing_token>(pos);
        context.on(_ev::error{}, err);
    }
};

/// Turns the arbitrary rule into a token by matching it without producing any values.
template <typename Rule>
constexpr auto token(Rule)
{
    if constexpr (lexy::is_token_rule<Rule>)
        return Rule{};
    else
        return _token<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TOKEN_HPP_INCLUDED

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

    template <std::size_t Node, typename Transitions = void>
    struct _node                              // Base case if we pass void as transitions.
    : _node<Node, _transition_sequence<Node>> // Compute transition and forward.
    {};
    template <std::size_t Node, std::size_t... Transitions>
    struct _node<Node, lexy::_detail::index_sequence<Transitions...>>
    {
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
                        ? (reader.bump(),
                           result = _node<Trie.transition_next(Node, Transitions)>::parse(reader),
                           true)
                        : false)
                   || ...);

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
    template <std::size_t Node>
    struct _node<Node, lexy::_detail::index_sequence<>>
    {
        template <typename Reader>
        static constexpr auto parse(Reader&)
        {
            // A node without transitions returns its value immediately.
            return Trie.node_value(Node);
        }
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We begin in the root node of the trie.
        return _node<0>::parse(reader) == Trie.invalid_value ? error_code::error : error_code();
    }

    template <typename Reader>
    static constexpr std::size_t parse(error_code& ec, Reader& reader)
    {
        auto result = _node<0>::parse(reader);
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
        context.on(_ev::error{}, err);
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

    static LEXY_CONSTEVAL auto token_kind()
    {
        return lexy::eol_token_kind;
    }

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "EOL");
        context.on(_ev::error{}, err);
    }
};

/// Matches the end of line (EOF or newline).
constexpr auto eol = _eol{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED




namespace lexy_ext
{
// Fake token that counts code units without verification.
struct _unchecked_code_unit
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            invalid = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (reader.eof())
                return error_code::invalid;

            reader.bump();
            return error_code();
        }
    };
};

/// Converts positions (iterators) into locations (line/column nr).
///
/// The unit for line and column numbers can be customized.
/// Every time the corresponding token matches, is the corresponding number increased.
///
/// See https://foonathan.net/2021/02/column/ for a discussion of potential units.
/// Use e.g. `lexy::dsl::code_point` and `lexy::dsl::newline` to count code points.
///
/// By default, it counts code units and newlines.
template <typename Input, typename TokenColumn = _unchecked_code_unit,
          typename TokenLine = std::decay_t<decltype(lexy::dsl::newline)>>
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
            return {_reader.cur(), _eol};
        }

        /// The newline after the line, if there is any.
        constexpr lexy::lexeme_for<Input> newline() const
        {
            auto reader = _reader;
            // Advance to EOl.
            while (reader.cur() != _eol)
                reader.bump();
            // Bump newline.
            lexy::engine_try_match<engine_line>(reader);
            return {_eol, reader.cur()};
        }

    private:
        constexpr location(lexy::input_reader<Input> reader, std::size_t line, std::size_t column)
        : _reader(LEXY_MOV(reader)), _eol(), _line(line), _column(column)
        {
            // Find EOL.
            for (auto reader = _reader; true; reader.bump())
            {
                if (reader.eof() || lexy::engine_peek<engine_line>(reader))
                {
                    _eol = reader.cur();
                    break;
                }
            }
        }

        // The reader starts at the beginning of the given line.
        lexy::input_reader<Input> _reader;
        iterator                  _eol;
        std::size_t               _line, _column;

        friend input_location_finder;
    };

    constexpr explicit input_location_finder(const Input& input) : _reader(input.reader()) {}
    constexpr explicit input_location_finder(const Input& input, TokenColumn, TokenLine)
    : _reader(input.reader())
    {}

    /// The starting location.
    constexpr location beginning() const
    {
        return location(_reader, 1, 1);
    }

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
        auto anchor = beginning();
        return find(pos, anchor);
    }

private:
    lexy::input_reader<Input> _reader;
};

/// Convenience function to find a single location.
template <typename Input, typename TokenColumn, typename TokenLine>
constexpr auto find_input_location(const Input&                                 input,
                                   typename lexy::input_reader<Input>::iterator pos, TokenColumn,
                                   TokenLine)
{
    return input_location_finder<Input, TokenColumn, TokenLine>(input).find(pos);
}
template <typename Input>
constexpr auto find_input_location(const Input&                                 input,
                                   typename lexy::input_reader<Input>::iterator pos)
{
    return input_location_finder<Input>(input).find(pos);
}
} // namespace lexy_ext

#endif // LEXY_EXT_INPUT_LOCATION_HPP_INCLUDED
 // implementation detail only

//=== debug_event ===//
namespace lexy::parse_events
{
/// Debug event was triggered.
/// Arguments: pos, str
struct debug_event
{};
} // namespace lexy::parse_events

namespace lexyd
{
template <typename String>
struct _debug
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::debug_event{}, reader.cur(), String::get().c_str());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

#if LEXY_HAS_NTTP
template <lexy::_detail::string_literal Str>
constexpr auto debug = _debug<lexy::_detail::type_string<Str>>{};
#endif

#define LEXY_DEBUG(Str)                                                                            \
    ::lexyd::_debug<LEXY_NTTP_STRING(Str)> {}
} // namespace lexyd

//=== trace ====//
namespace lexy
{
namespace _ev = lexy::parse_events;

template <typename OutputIt, typename Input, typename TokenKind = void>
class trace_handler
{
    using location_finder = lexy_ext::input_location_finder<Input>;
    using location        = typename location_finder::location;

    struct label_t
    {
        const LEXY_CHAR_OF_u8* line;
        const LEXY_CHAR_OF_u8* event;
        const LEXY_CHAR_OF_u8* finish_event;
        const LEXY_CHAR_OF_u8* cancel_event;
    };

    label_t label() const
    {
        return _opts.is_set(visualize_use_unicode) ? label_t{u8"│  ", u8"├──", u8"┴", u8"└"}
                                                   : label_t{u8"  ", u8"- ", u8"- finish", u8"-"};
    }

    enum class prefix
    {
        event,
        cancel,
        finish,
    };

    OutputIt write_prefix(OutputIt out, std::size_t cur_depth, const location& loc, prefix p) const
    {
        const auto l = label();

        if (cur_depth > 0)
            *out++ = '\n';

        out = _detail::write_color<_detail::color::faint>(out, _opts);
        out = _detail::write_format(out, "%2zu:%3zu", loc.line_nr(), loc.column_nr());
        out = _detail::write_str(out, ": ");
        out = _detail::write_color<_detail::color::reset>(out, _opts);

        if (cur_depth > 0)
        {
            for (auto i = 0u; i != cur_depth - 1; ++i)
                out = _detail::write_str(out, l.line);

            switch (p)
            {
            case prefix::event:
                out = _detail::write_str(out, l.event);
                break;
            case prefix::cancel:
                out = _detail::write_str(out, l.cancel_event);
                out = _detail::write_color<_detail::color::yellow>(out, _opts);
                if (_opts.is_set(visualize_use_unicode))
                    out = _detail::write_str(out, u8"╳");
                else
                    out = _detail::write_str(out, "x");
                out = _detail::write_color<_detail::color::reset>(out, _opts);
                break;
            case prefix::finish:
                out = _detail::write_str(out, l.finish_event);
                break;
            }
        }

        return out;
    }

public:
    explicit trace_handler(OutputIt out, const Input& input,
                           visualization_options opts = {}) noexcept
    : _out(out), _cur_depth(0), _locations(input), _anchor(_locations.beginning()), _opts(opts)
    {
        LEXY_PRECONDITION(_opts.max_tree_depth <= visualization_options::max_tree_depth_limit);
    }

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    OutputIt get_result_value() && noexcept
    {
        *_out++ = '\n';
        return _out;
    }
    template <typename Production>
    OutputIt get_result_empty() && noexcept
    {
        *_out++ = '\n';
        return _out;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {
        // The beginning of the previous production.
        // If the current production gets canceled, it needs to be restored.
        location previous_anchor;
    };

    template <typename Production, typename Iterator>
    marker<Production> on(_ev::production_start<Production>, Iterator pos)
    {
        const auto loc = _locations.find(pos, _anchor);

        // All other events are after this point.
        auto previous_anchor = _anchor;
        _anchor              = loc;

        if (_cur_depth <= _opts.max_tree_depth)
        {
            _out = write_prefix(_out, _cur_depth, loc, prefix::event);
            _out = _detail::write_color<_detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, lexy::production_name<Production>());
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            if (_cur_depth == _opts.max_tree_depth)
            {
                // Print an ellipsis instead of children.
                _out = _detail::write_str(_out, ": ");
                _out = _detail::write_ellipsis(_out, _opts);
            }
            else
            {
                // Prepare for children.
                _out = _detail::write_str(_out, ":");
            }
        }

        ++_cur_depth;
        _last_token.reset();

        return {previous_anchor};
    }

    template <typename Production, typename Iterator>
    auto on(marker<Production>, _ev::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename TK, typename Iterator>
    void on(const marker<Production>&, _ev::token, TK _kind, Iterator begin, Iterator end)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto kind = lexy::token_kind<TokenKind>(_kind);
        const auto loc  = _locations.find(begin, _anchor);

        if (_last_token.merge(Production{}, kind))
        {
            _out = visualize_to(_out, lexy::lexeme_for<Input>(begin, end), _opts | visualize_space);
        }
        else
        {
            _out = write_prefix(_out, _cur_depth, loc, prefix::event);
            _out = _detail::write_color<_detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, kind.name());
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            if (begin != end)
            {
                _out = _detail::write_str(_out, ": ");
                _out = visualize_to(_out, lexy::lexeme_for<Input>(begin, end),
                                    _opts | visualize_space);
            }
        }

        _last_token.update(kind);
    }

    template <typename Production, typename Reader, typename Tag>
    void on(marker<Production>, _ev::error, const lexy::error<Reader, Tag>& error)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto loc = _locations.find(error.position(), _anchor);

        _out = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::red, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "error");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::red>(_out, _opts);
        _out = _detail::write_str(_out, ": ");

        if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string());

            _out = _detail::write_str(_out, "expected '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string());

            _out = _detail::write_str(_out, "expected keyword '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
        {
            _out = _detail::write_str(_out, "expected '");
            _out = _detail::write_str(_out, error.character_class());
            _out = _detail::write_str(_out, "' character");
        }
        else
        {
            _out = _detail::write_str(_out, error.message());
        }

        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        // We can no longer merge tokens.
        _last_token.reset();
    }

    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::backtracked, Iterator begin, Iterator end)
    {
        if (_cur_depth > _opts.max_tree_depth
            // If we haven't actually consumed any characters, we didn't really backtrack;
            // peeking at the next character is allowed.
            || begin == end)
            return;

        const auto loc = _locations.find(begin, _anchor);

        _out = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "backtracked");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (begin != end)
        {
            _out = _detail::write_str(_out, ": ");
            _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
            _out = visualize_to(_out, lexy::lexeme_for<Input>(begin, end),
                                _opts.reset(visualize_use_color) | visualize_space);
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);
        }

        // We can no longer merge tokens.
        _last_token.reset();
    }

    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::recovery_start, Iterator pos)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto loc = _locations.find(pos, _anchor);
        _out           = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "error recovery");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
        _out = _detail::write_str(_out, ":");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (_cur_depth == _opts.max_tree_depth)
        {
            // Print an ellipsis instead of children.
            _out = _detail::write_str(_out, " ");
            _out = _detail::write_ellipsis(_out, _opts);
        }

        // We can no longer merge tokens.
        _last_token.reset();
        // Treat it as an extra level.
        ++_cur_depth;
    }

    template <typename Iterator>
    void on_recovery_end(Iterator pos, bool success)
    {
        if (_cur_depth < _opts.max_tree_depth)
        {
            const auto loc = _locations.find(pos, _anchor);
            _out = write_prefix(_out, _cur_depth, loc, success ? prefix::finish : prefix::cancel);

            if (!success)
            {}
        }

        _last_token.reset();
        --_cur_depth;
    }
    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::recovery_finish, Iterator pos)
    {
        on_recovery_end(pos, true);
    }
    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::recovery_cancel, Iterator pos)
    {
        on_recovery_end(pos, false);
    }

    template <typename Production, typename Iterator>
    void on(const marker<Production>&, _ev::debug_event, Iterator pos, const char* str)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        const auto loc = _locations.find(pos, _anchor);

        _out = write_prefix(_out, _cur_depth, loc, prefix::event);
        _out = _detail::write_color<_detail::color::blue, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "debug");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::blue>(_out, _opts);
        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_str(_out, str);
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        // We can no longer merge tokens.
        _last_token.reset();
    }

    template <typename Production, typename Iterator, typename... Args>
    void on(marker<Production>&&, _ev::production_finish<Production>, Iterator pos, Args&&...)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            const auto loc = _locations.find(pos, _anchor);
            _out           = write_prefix(_out, _cur_depth, loc, prefix::finish);
        }

        --_cur_depth;
    }
    template <typename Production, typename Iterator>
    void on(marker<Production>&& m, _ev::production_cancel<Production>, Iterator pos)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            const auto loc = _locations.find(pos, _anchor);
            _out           = write_prefix(_out, _cur_depth, loc, prefix::cancel);
        }

        --_cur_depth;

        // Restore the anchor as we've backtracked.
        _anchor = m.previous_anchor;
    }

private:
    struct last_token_info
    {
        bool                        first_token;
        lexy::token_kind<TokenKind> kind;

        last_token_info() : first_token(true) {}

        template <typename Production>
        bool merge(Production, lexy::token_kind<TokenKind> new_kind) const
        {
            return lexy::is_token_production<Production> && !first_token && kind == new_kind;
        }

        void update(lexy::token_kind<TokenKind> new_kind)
        {
            first_token = false;
            kind        = new_kind;
        }

        void reset()
        {
            first_token = true;
        }
    };

    OutputIt _out;

    std::size_t     _cur_depth;
    last_token_info _last_token;

    location_finder _locations;
    location        _anchor;

    visualization_options _opts;
};

template <typename Production, typename TokenKind = void, typename OutputIt, typename Input>
OutputIt trace_to(OutputIt out, const Input& input, visualization_options opts = {})
{
    auto reader = input.reader();
    return lexy::do_action<Production>(trace_handler<OutputIt, Input, TokenKind>(out, input, opts),
                                       reader);
}

template <typename Production, typename TokenKind = void, typename Input>
void trace(std::FILE* file, const Input& input, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, opts);
}
} // namespace lexy

#endif // LEXY_ACTION_TRACE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_HPP_INCLUDED
#define LEXY_DSL_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED





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
        context.on(_ev::error{}, err);
    }
};

template <typename R, typename S>
constexpr auto operator/(R, S)
{
    static_assert(lexy::is_token_rule<R> && lexy::is_token_rule<S>);
    return _alt<R, S>{};
}

template <typename... R, typename S>
constexpr auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_token_rule<S>);
    return _alt<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_token_rule<R>);
    return _alt<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator/(_alt<R...>, _alt<S...>)
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
    constexpr ascii_table() : _table() {}

    /// Adds the character to the given category.
    template <typename CharT>
    constexpr ascii_table& insert(CharT c, std::size_t category)
    {
        auto as_unsigned = static_cast<unsigned char>(c);
        LEXY_PRECONDITION(as_unsigned <= 0x7F);
        LEXY_PRECONDITION(category < CategoryCount);

        // Set the given bit.
        _table[as_unsigned] = int_n(_table[as_unsigned] | 1 << category);

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
            // NOLINTNEXTLINE: We've checked that we're positive in the condition above.
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
        context.on(_ev::error{}, err);
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

    using token_engine = lexy::engine_char_range<0x21, 0x7E>;
};
inline constexpr auto graph = _graph{};

struct _print : _ascii<_print>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "ASCII.print";
    }

    using token_engine = lexy::engine_char_range<0x20, 0x7E>;
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
        context.on(_ev::error{}, err);
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
        context.on(_ev::error{}, err);
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
            using token_parser = lexy::rule_parser<H, lexy::discard_parser<Context>>;
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
                context.on(_ev::backtracked{}, save.cur(), reader.cur());
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

    static constexpr auto is_branch               = (lexy::is_token_rule<R> && ...);
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    using parser = lexy::rule_parser<_seq_impl<R...>, NextParser>;
};

template <typename R, typename S>
constexpr auto operator+(R, S)
{
    return _seq<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator+(_seq<R...>, S)
{
    return _seq<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator+(R, _seq<S...>)
{
    return _seq<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator+(_seq<R...>, _seq<S...>)
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
constexpr auto operator>>(Condition, Then)
{
    static_assert(lexy::is_branch_rule<Condition>, "condition must be a branch");
    return _br<Condition, Then>{};
}
template <typename Condition, typename... R>
constexpr auto operator>>(Condition, _seq<R...>)
{
    static_assert(lexy::is_branch_rule<Condition>, "condition must be a branch");
    return _br<Condition, R...>{};
}
template <typename Condition, typename C, typename... R>
constexpr auto operator>>(Condition, _br<C, R...>)
{
    static_assert(lexy::is_branch_rule<Condition>, "condition must be a branch");
    return _br<Condition, C, R...>{};
}

// Prevent nested branches in `_br`'s condition.
template <typename C, typename... R, typename Then>
constexpr auto operator>>(_br<C, R...>, Then)
{
    return C{} >> _seq<R..., Then>{};
}
template <typename C, typename... R, typename... S>
constexpr auto operator>>(_br<C, R...>, _seq<S...>)
{
    return C{} >> _seq<R..., S...>{};
}

// Disambiguation.
template <typename C1, typename... R, typename C2, typename... S>
constexpr auto operator>>(_br<C1, R...>, _br<C2, S...>)
{
    return _br<C1, R..., C2, S...>{};
}

//=== operator+ ===//
// If we add something on the left to a branch, we loose the branchy-ness.
template <typename Rule, typename Condition, typename... R>
constexpr auto operator+(Rule rule, _br<Condition, R...>)
{
    return rule + _seq<Condition, R...>{};
}
// Disambiguation.
template <typename... R, typename Condition, typename... S>
constexpr auto operator+(_seq<R...>, _br<Condition, S...>)
{
    return _seq<R...>{} + _seq<Condition, S...>{};
}

// If we add something on the right to a branch, we extend the then.
template <typename Condition, typename... R, typename Rule>
constexpr auto operator+(_br<Condition, R...>, Rule)
{
    return _br<Condition, R..., Rule>{};
}
// Disambiguation.
template <typename Condition, typename... R, typename... S>
constexpr auto operator+(_br<Condition, R...>, _seq<S...>)
{
    return _br<Condition, R..., S...>{};
}

// If we add two branches, we use the condition of the first one and treat the second as sequence.
template <typename C1, typename... R, typename C2, typename... S>
constexpr auto operator+(_br<C1, R...>, _br<C2, S...>)
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
struct _else_dsl
{
    template <typename R>
    friend constexpr auto operator>>(_else_dsl, R rule)
    {
        return _else{} >> rule;
    }
    template <typename... R>
    friend constexpr auto operator>>(_else_dsl, _seq<R...> rule)
    {
        return _else{} >> rule;
    }
    template <typename C, typename... R>
    friend constexpr auto operator>>(_else_dsl, _br<C, R...> rule)
    {
        return _else{} >> rule;
    }
};

/// Takes the branch unconditionally.
inline constexpr auto else_ = _else_dsl{};
} // namespace lexyd

#endif // LEXY_DSL_BRANCH_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED





#ifdef LEXY_IGNORE_DEPRECATED_REQUIRE
#    define LEXY_DEPRECATED_REQUIRE
#else
#    define LEXY_DEPRECATED_REQUIRE                                                                \
        [[deprecated("`dsl::require()/dsl::prevent()` have been replaced by `dsl::peek[_not]()`")]]
#endif

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
            context.on(_ev::error{}, err);
            return false;
        }
    };

    /// Adds a rule whose match will be part of the error location.
    template <typename Rule>
    constexpr auto operator()(Rule rule) const
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
template <typename Branch, typename Error>
struct _must : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser : lexy::rule_parser<Branch, NextParser>
    {
        // inherit try_parse() from Branch

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using parser = lexy::rule_parser<Branch, NextParser>;
            auto result  = parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result != lexy::rule_try_parse_result::backtracked)
                return static_cast<bool>(result);
            else
                return lexy::rule_parser<Error, NextParser>::parse(context, reader,
                                                                   LEXY_FWD(args)...);
        }
    };
};

template <typename Branch>
struct _must_dsl
{
    template <typename Tag>
    struct _err : _must<Branch, lexyd::_err<Tag, void>>
    {
        template <typename Rule>
        constexpr auto operator()(Rule rule) const
        {
            auto err = lexyd::error<Tag>(rule);
            return _must<Branch, decltype(err)>{};
        }
    };

    template <typename Tag>
    static constexpr _err<Tag> error = _err<Tag>{};
};

/// Tries to parse `Branch` and raises a specific error on failure.
/// It can still be used as a branch rule; then behaves exactly like `Branch.`
template <typename Branch>
constexpr auto must(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    static_assert(!Branch::is_unconditional_branch);
    return _must_dsl<Branch>{};
}
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
                context.on(_ev::error{}, err);
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
            auto copy  = reader;
            if (lexy::engine_try_match<typename Token::token_engine>(copy))
            {
                // Token did match what we don't want.
                // Report an error, but continue parsing.
                auto err = lexy::make_error<Reader, Tag>(begin, copy.cur());
                context.on(_ev::error{}, err);
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
LEXY_DEPRECATED_REQUIRE constexpr auto require(Rule rule)
{
    auto t = token(rule);
    return _require_dsl<decltype(t)>{};
}

/// Requires that lookahead does not match a rule at a location.
template <typename Rule>
LEXY_DEPRECATED_REQUIRE constexpr auto prevent(Rule rule)
{
    auto t = token(rule);
    return _prevent_dsl<decltype(t)>{};
}

template <typename Tag, typename Rule>
LEXY_DEPRECATED_REQUIRE constexpr auto require(Rule rule)
{
    return require(rule).template error<Tag>;
}
template <typename Tag, typename Rule>
LEXY_DEPRECATED_REQUIRE constexpr auto prevent(Rule rule)
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
        context.on(_ev::error{}, err);
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
    static constexpr auto _would_be_unconditional_branch = (R::is_unconditional_branch || ...);

    // We only make it a choice if it's not an unconditional branch;
    // this is almost surely a bug.
    static constexpr auto is_branch              = !_would_be_unconditional_branch;
    static constexpr auto is_unconditonal_branch = false;

    template <typename NextParser>
    using parser = _chc_parser<NextParser, R...>;
};

template <typename R, typename S>
constexpr auto operator|(R, S)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator|(_chc<R...>, S)
{
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator|(R, _chc<S...>)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator|(_chc<R...>, _chc<S...>)
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
        context.on(_ev::error{}, err);
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
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));

        auto l = (Limit{} / ... / tokens);
        return _find<Token, decltype(l)>{};
    }

    constexpr auto get_limit() const
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
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));

        auto l = (tokens / ...);
        return _find<Token, decltype(l)>{};
    }

    constexpr auto get_limit() const
    {
        return eof;
    }
};

/// Recovers once it finds one of the given tokens (without consuming them).
template <typename... Tokens>
constexpr auto find(Tokens... tokens)
{
    static_assert(sizeof...(Tokens) > 0);
    static_assert((lexy::is_token_rule<Tokens> && ...));

    auto needle = (tokens / ...);
    return _find<decltype(needle), void>{};
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
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));

        auto l = (Limit{} / ... / tokens);
        return _reco<decltype(l), R...>{};
    }

    constexpr auto get_limit() const
    {
        return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
constexpr auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    static_assert((lexy::is_branch_rule<Branches> && ...));
    return _reco<lexyd::_eof, Branches...>{};
}
} // namespace lexyd

namespace lexyd
{
// Performs the recovery part of a try rule.
template <typename Recover, typename NextParser>
struct _try_recovery
{
    struct _continuation
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, bool& recovery_finished,
                                 Args&&... args)
        {
            recovery_finished = true;
            context.on(_ev::recovery_finish{}, reader.cur());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        context.on(_ev::recovery_start{}, reader.cur());

        auto recovery_finished = false;
        auto result
            = lexy::rule_parser<Recover, _continuation>::parse(context, reader, recovery_finished,
                                                               LEXY_FWD(args)...);
        if (!recovery_finished)
            context.on(_ev::recovery_cancel{}, reader.cur());
        return result;
    }
};
template <typename NextParser>
struct _try_recovery<void, NextParser> : NextParser
{};

template <typename Rule, typename Recover>
struct _tryr : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, bool& rule_succeeded,
                                     Args&&... args)
            {
                rule_succeeded = true;
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto rule_succeeded = false;
            // Try parsing with special continuation that sets rule_succeeded to false if reached.
            auto result
                = lexy::rule_parser<Rule, _continuation>::try_parse(context, reader, rule_succeeded,
                                                                    LEXY_FWD(args)...);
            if (rule_succeeded || result == lexy::rule_try_parse_result::backtracked)
            {
                // Our rule has succeded or backtracked.
                // In either case, it did not fail.
                // It could be the case that some later rule has failed, but that's not our problem.
                return result;
            }
            else
            {
                // Rule has failed, recover.
                // Note that we already took the branch by definition, so we no longer backtrack.
                // Rule has failed, recover.
                return _try_recovery<Recover, NextParser>::parse(context, reader, LEXY_FWD(args)...)
                           ? lexy::rule_try_parse_result::ok
                           : lexy::rule_try_parse_result::canceled;
            }
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto rule_succeeded = false;
            // Parse with special continuation that sets rule_succeeded to false if reached.
            auto result
                = lexy::rule_parser<Rule, _continuation>::parse(context, reader, rule_succeeded,
                                                                LEXY_FWD(args)...);
            if (rule_succeeded)
            {
                // Rule didn't fail.
                // It could be the case that some later rule has failed, but that's not our problem.
                return result;
            }
            else
            {
                // Rule has failed, recover.
                return _try_recovery<Recover, NextParser>::parse(context, reader,
                                                                 LEXY_FWD(args)...);
            }
        }
    };
};

/// Pares Rule, if that fails, continues immediately.
template <typename Rule>
constexpr auto try_(Rule)
{
    return _tryr<Rule, void>{};
}

/// Parses Rule, if that fails, parses recovery rule.
template <typename Rule, typename Recover>
constexpr auto try_(Rule, Recover)
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

            lexy::_detail::parse_context_var loop_context(context, _break{}, flag{});
            while (!loop_context.get(_break{}).loop_break)
            {
                using parser
                    = lexy::rule_parser<Rule, lexy::discard_parser<decltype(loop_context)>>;
                if (!parser::parse(loop_context, reader))
                    return false;
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeatedly matches the rule until a break rule matches.
template <typename Rule>
constexpr auto loop(Rule)
{
    return _loop<Rule>{};
}
} // namespace lexyd

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
                using branch_parser = lexy::rule_parser<Branch, lexy::discard_parser<Context>>;

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
constexpr auto while_(Rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "while() requires a branch condition");
    return _whl<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the rule at least once, then as often as possible.
template <typename Rule>
constexpr auto while_one(Rule rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "while_one() requires a branch condition");
    return rule >> while_(rule);
}
} // namespace lexyd

namespace lexyd
{
/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
constexpr auto do_while(Then then, Condition condition)
{
    if constexpr (lexy::is_branch_rule<Then>)
        return then >> while_(condition >> then);
    else
        return then + while_(condition >> then);
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



//=== implementation ===//
namespace lexy::_detail
{
struct tag_no_whitespace
{};

template <typename Rule>
struct ws_production
{
    static constexpr auto rule = lexy::dsl::loop(Rule{} | lexy::dsl::break_);
};

template <typename Context>
struct whitespace_handler
{
    Context* parent;

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr bool get_result_value() && noexcept
    {
        return true;
    }
    template <typename Production>
    constexpr bool get_result_empty() && noexcept
    {
        return false;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {};

    template <typename Rule, typename Iterator>
    constexpr auto on(parse_events::production_start<ws_production<Rule>>, Iterator)
    {
        return marker<ws_production<Rule>>{};
    }
    template <typename Production, typename Iterator>
    constexpr auto on(parse_events::production_start<Production>, Iterator)
    {
        static_assert(_detail::error<Production>,
                      "whitespace rule must not contain `dsl::p` or `dsl::recurse`;"
                      "use `dsl::inline_` instead");
        return marker<Production>{};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production>, parse_events::error ev, Error&& error)
    {
        parent->on(ev, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}
};

template <typename Rule, typename NextParser>
struct manual_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto begin = reader.cur();
        if constexpr (lexy::is_token_rule<Rule>)
        {
            // Parsing a token repeatedly cannot fail, so we can optimize it using an engine.
            using engine = lexy::engine_while<typename Rule::token_engine>;
            engine::match(reader);
        }
        else
        {
            // Parse the rule using a special handler that only forwards errors.
            using production = ws_production<Rule>;
            whitespace_handler<Context> ws_handler{&context};
            if (!lexy::do_action<production>(LEXY_MOV(ws_handler), reader))
                return false;
        }
        auto end = reader.cur();

        // Add a whitespace token node.
        if (begin != end)
            context.on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin, end);

        return NextParser::parse(context, reader, LEXY_FWD(args)...);
    }
};
template <typename NextParser>
struct manual_ws_parser<void, NextParser> : NextParser
{};

template <typename Context>
using context_whitespace
    = lexy::production_whitespace<typename Context::production, typename Context::root_production>;

template <typename NextParser>
struct automatic_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if constexpr (Context::contains(lexy::_detail::tag_no_whitespace{}))
        {
            // Automatic whitespace skipping is disabled.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Skip the appropriate whitespace.
            using rule = context_whitespace<Context>;
            return manual_ws_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    }
};

} // namespace lexy::_detail

//=== whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsr : rule_base
{
    template <typename NextParser>
    struct parser : lexy::_detail::manual_ws_parser<Rule, NextParser>
    {};

    template <typename R>
    friend constexpr auto operator|(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} | r)>{};
    }
    template <typename R>
    friend constexpr auto operator|(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r | Rule{})>{};
    }

    template <typename R>
    friend constexpr auto operator/(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} / r)>{};
    }
    template <typename R>
    friend constexpr auto operator/(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r / Rule{})>{};
    }
};

struct _ws : rule_base
{
    template <typename NextParser>
    struct parser : lexy::_detail::automatic_ws_parser<NextParser>
    {};

    /// Overrides implicit whitespace detection.
    template <typename Rule>
    constexpr auto operator()(Rule) const
    {
        return _wsr<Rule>{};
    }
};

/// Matches whitespace.
constexpr auto whitespace = _ws{};
} // namespace lexyd

//=== no_whitespace ===//
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
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
                // Optimization: no whitespace rule in the current context; do nothing special.
                return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader,
                                                                      LEXY_FWD(args)...);

            // Parse the rule using the context that doesn't allow inner whitespace.
            lexy::_detail::parse_context_var ws_context(context, lexy::_detail::tag_no_whitespace{},
                                                        lexy::_detail::tag_no_whitespace{});
            return lexy::rule_parser<Rule, _cont>::try_parse(ws_context, reader, context,
                                                             LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
                // Optimization: no whitespace rule in the current context; do nothing special.
                return lexy::rule_parser<Rule, NextParser>::parse(context, reader,
                                                                  LEXY_FWD(args)...);

            // Parse the rule using the context that doesn't allow inner whitespace.
            lexy::_detail::parse_context_var ws_context(context, lexy::_detail::tag_no_whitespace{},
                                                        lexy::_detail::tag_no_whitespace{});
            return lexy::rule_parser<Rule, _cont>::parse(ws_context, reader, context,
                                                         LEXY_FWD(args)...);
        }
    };
};

/// Disables automatic skipping of whitespace for all tokens of the given rule.
template <typename Rule>
constexpr auto no_whitespace(Rule)
{
    if constexpr (lexy::is_token_rule<Rule>)
        return Rule{}; // Token already behaves that way.
    else
        return _wsn<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED


namespace lexyd
{
template <typename Terminator, typename R, typename Recover>
struct _optt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _lstt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _olstt;

template <typename Terminator, typename... RecoveryLimit>
struct _term
{
    /// Adds the tokens to the recovery limit.
    template <typename... Tokens>
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));
        return _term<Terminator, RecoveryLimit..., Tokens...>{};
    }

    //=== rules ===//
    /// Matches rule followed by the terminator.
    template <typename Rule>
    constexpr auto operator()(Rule rule) const
    {
        if constexpr (lexy::is_branch_rule<Rule>)
            return rule >> terminator();
        else
            return rule + terminator();
    }

    /// Matches rule followed by the terminator, recovering on error.
    template <typename Rule>
    constexpr auto try_(Rule rule) const
    {
        return lexyd::try_(rule + terminator(), recovery_rule());
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt(R) const
    {
        return _optt<Terminator, R, decltype(recovery_rule())>{};
    }

    /// Matches `list(r, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto list(R) const
    {
        return _lstt<Terminator, R, void, decltype(recovery_rule())>{};
    }
    template <typename R, typename Sep>
    constexpr auto list(R, Sep) const
    {
        return _lstt<Terminator, R, Sep, decltype(recovery_rule())>{};
    }

    /// Matches `opt_list(r, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt_list(R) const
    {
        return _olstt<Terminator, R, void, decltype(recovery_rule())>{};
    }
    template <typename R, typename S>
    constexpr auto opt_list(R, S) const
    {
        return _olstt<Terminator, R, S, decltype(recovery_rule())>{};
    }

    //=== access ===//
    /// Matches the terminator alone.
    constexpr auto terminator() const
    {
        return Terminator{};
    }

    /// Matches the recovery rule alone.
    constexpr auto recovery_rule() const
    {
        if constexpr (sizeof...(RecoveryLimit) == 0)
            return recover(terminator());
        else
            return recover(terminator()).limit(RecoveryLimit{}...);
    }
};

/// Creates a terminator using the given branch.
template <typename Branch>
constexpr auto terminator(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
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
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));
        return _brackets<Open, Close, RecoveryLimit..., Tokens...>{};
    }

    //=== rules ===//
    /// Matches the rule surrounded by brackets.
    template <typename R>
    constexpr auto operator()(R r) const
    {
        return open() >> as_terminator()(r);
    }

    /// Matches the rule surrounded by brackets, recovering on error.
    template <typename R>
    constexpr auto try_(R r) const
    {
        return open() >> as_terminator().try_(r);
    }

    /// Matches `opt(r)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt(R r) const
    {
        return open() >> as_terminator().opt(r);
    }

    /// Matches `list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto list(R r) const
    {
        return open() >> as_terminator().list(r);
    }
    template <typename R, typename S>
    constexpr auto list(R r, S sep) const
    {
        return open() >> as_terminator().list(r, sep);
    }

    /// Matches `opt_list(r, sep)` surrounded by brackets.
    /// The rule does not require a condition.
    template <typename R>
    constexpr auto opt_list(R r) const
    {
        return open() >> as_terminator().opt_list(r);
    }
    template <typename R, typename S>
    constexpr auto opt_list(R r, S sep) const
    {
        return open() >> as_terminator().opt_list(r, sep);
    }

    //=== access ===//
    /// Matches the open bracket.
    constexpr auto open() const
    {
        return Open{};
    }
    /// Matches the closing bracket.
    constexpr auto close() const
    {
        return Close{};
    }

    /// Returns an equivalent terminator.
    constexpr auto as_terminator() const
    {
        return _term<Close, RecoveryLimit...>{};
    }

    constexpr auto recovery_rule() const
    {
        return as_terminator().recovery_rule();
    }
};

/// Defines open and close brackets.
template <typename Open, typename Close>
constexpr auto brackets(Open, Close)
{
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
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
};

/// Captures whatever the rule matches as a lexeme.
template <typename Rule>
constexpr auto capture(Rule)
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

template <typename Predicate>
struct _cp : token_base<_cp<Predicate>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            invalid = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            // Parse one code point.
            lexy::engine_cp_auto::error_code ec{};
            [[maybe_unused]] auto            cp = lexy::engine_cp_auto::parse(ec, reader);
            if (ec != lexy::engine_cp_auto::error_code{})
                return error_code(int(ec) + 1);

            // Check whether it matches the predicate.
            if constexpr (!std::is_void_v<Predicate>)
            {
                if (!Predicate()(cp))
                    return error_code::invalid;
            }

            return error_code();
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context&                          context, const Reader&,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        if (ec == token_engine::error_code::invalid)
        {
            auto name = lexy::_detail::type_name<Predicate>();
            auto err  = lexy::make_error<Reader, lexy::expected_char_class>(pos, name);
            context.on(_ev::error{}, err);
        }
        else
        {
            auto name = _cp_name<typename Reader::encoding>();
            auto err  = lexy::make_error<Reader, lexy::expected_char_class>(pos, name);
            context.on(_ev::error{}, err);
        }
    }

    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _cp<P>{};
    }
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp<void>{};
} // namespace lexyd

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED








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
    // Whether or not the state has already been handled.
    const bool* handled;
    // Write the index of the item in here.
    std::size_t idx = 0;
    // Whether or not we should break.
    bool loop_break = false;
};

// Final parser for one item in the combination.
struct _comb_final
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader&, std::size_t idx, Args&&... args)
    {
        auto& state = context.get(_break{});
        state.idx   = idx;
        if constexpr (sizeof...(Args) > 0)
        {
            if (!state.handled[idx])
                // Only call the sink if it is not a duplicate.
                state.sink(LEXY_FWD(args)...);
        }
        return true;
    }
};

// Parser for one item in the combination.
template <std::size_t Idx, typename Rule>
struct _comb_it : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader, Idx);
        }

        template <typename Context, typename Reader>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader)
        {
            return lexy::rule_parser<Rule, NextParser>::parse(context, reader, Idx);
        }
    };
};

template <typename DuplicateError, typename ElseRule, typename... R>
struct _comb : rule_base
{
    template <std::size_t... Idx>
    static auto _comb_choice_(lexy::_detail::index_sequence<Idx...>)
    {
        if constexpr (std::is_void_v<ElseRule>)
            return (_comb_it<Idx, R>{} | ...);
        else
            return (_comb_it<Idx, R>{} | ... | ElseRule{});
    }
    using _comb_choice = decltype(_comb_choice_(lexy::_detail::index_sequence_for<R...>{}));

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto N = sizeof...(R);

            auto sink       = context.on(_ev::list{}, reader.cur());
            bool handled[N] = {};
            using state_t   = _comb_state<decltype(sink)>;

            lexy::_detail::parse_context_var comb_context(context, _break{},
                                                          state_t{sink, handled});
            auto&                            state = comb_context.get(_break{});

            // Parse all iterations of the choice.
            for (auto count = 0; count < int(N); ++count)
            {
                auto begin = reader.cur();

                using parser = lexy::rule_parser<_comb_choice, _comb_final>;
                if (!parser::parse(comb_context, reader))
                    return false;
                else if (state.loop_break)
                    break; // Partial combination and we're done.

                if (handled[state.idx])
                {
                    using tag = lexy::_detail::type_or<DuplicateError, lexy::combination_duplicate>;
                    auto err  = lexy::make_error<Reader, tag>(begin, reader.cur());
                    context.on(_ev::error{}, err);
                    // We can trivially recover, but need to do another iteration.
                    --count;
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
constexpr auto combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "combination() requires a branch rule");
    static_assert((!R::is_unconditional_branch && ...),
                  "combination() does not support unconditional branches");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
constexpr auto partial_combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "partial_combination() requires a branch rule");
    static_assert((!R::is_unconditional_branch && ...),
                  "partial_combination() does not support unconditional branches");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
}

template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `combination<Tag>(r...)` by `combination(r...).duplicate_error<Tag>`")
constexpr auto combination(R... r)
{
    return combination(r...).template duplicate_error<Tag>;
}
template <typename Tag, typename... R>
LEXY_DEPRECATED_ERROR(
    "replace `partial_combination<Tag>(r...)` by `partial_combination(r...).duplicate_error<Tag>`")
constexpr auto partial_combination(R... r)
{
    return partial_combination(r...).template duplicate_error<Tag>;
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED





namespace lexy
{
struct unequal_counts
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unequal counts";
    }
};
} // namespace lexy

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
            static_assert(!Context::contains(Id{}));
            lexy::_detail::parse_context_var counter_ctx(context, Id{}, InitialValue);
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
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

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
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Rule, _cont>::try_parse(context, reader, reader.cur(),
                                                             LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value>
struct _ctx_cis : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (context.get(Id{}) != Value)
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        // inherit parse
    };
};

template <typename Id>
struct _ctx_cvalue : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., context.get(Id{}));
        }
    };
};

template <typename... Ids>
struct _ctx_ceq;
template <typename H, typename... T>
struct _ctx_ceq<H, T...> : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto value = context.get(H{});
            if (((value != context.get(T{})) || ...))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto value = context.get(H{});
            if (((value != context.get(T{})) || ...))
            {
                auto err = lexy::make_error<Reader, lexy::unequal_counts>(reader.cur());
                context.on(_ev::error{}, err);
                // Trivially recover.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

} // namespace lexyd

namespace lexyd
{
template <typename Id>
struct _ctx_counter
{
    struct id
    {};

    template <int InitialValue = 0>
    constexpr auto create() const
    {
        return _ctx_ccreate<id, InitialValue>{};
    }

    constexpr auto inc() const
    {
        return _ctx_cadd<id, +1>{};
    }
    constexpr auto dec() const
    {
        return _ctx_cadd<id, -1>{};
    }

    template <typename Rule>
    constexpr auto push(Rule) const
    {
        return _ctx_cpush<id, Rule, +1>{};
    }
    template <typename Rule>
    constexpr auto pop(Rule) const
    {
        return _ctx_cpush<id, Rule, -1>{};
    }

    template <int Value>
    constexpr auto is() const
    {
        return _ctx_cis<id, Value>{};
    }
    constexpr auto is_zero() const
    {
        return is<0>();
    }

    constexpr auto value() const
    {
        return _ctx_cvalue<id>{};
    }
};

/// Declares an integer counter that is added to the parsing context.
template <typename Id>
constexpr auto context_counter = _ctx_counter<Id>{};

/// Takes a branch only if all counters are equal.
template <typename... Ids>
constexpr auto equal_counts(_ctx_counter<Ids>...)
{
    static_assert(sizeof...(Ids) > 1);
    return _ctx_ceq<typename _ctx_counter<Ids>::id...>{};
}
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
            static_assert(!Context::contains(Id{}));
            lexy::_detail::parse_context_var flag_ctx(context, Id{}, InitialValue);
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

template <typename Id, bool Value>
struct _ctx_fis : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser : NextParser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (context.get(Id{}) != Value)
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        // inherit parse
    };
};

template <typename Id>
struct _ctx_fvalue : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., context.get(Id{}));
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id>
struct _ctx_flag
{
    struct id
    {};

    template <bool InitialValue = false>
    constexpr auto create() const
    {
        return _ctx_fcreate<id, InitialValue>{};
    }

    constexpr auto set() const
    {
        return _ctx_fset<id, true>{};
    }
    constexpr auto reset() const
    {
        return _ctx_fset<id, false>{};
    }

    constexpr auto toggle() const
    {
        return _ctx_ftoggle<id>{};
    }

    constexpr auto is_set() const
    {
        return _ctx_fis<id, true>{};
    }
    constexpr auto is_reset() const
    {
        return _ctx_fis<id, false>{};
    }

    constexpr auto value() const
    {
        return _ctx_fvalue<id>{};
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

#ifndef LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED


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
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename Iter, typename... Args>
        LEXY_DSL_FUNC bool _parse_impl(Context& context, Reader& reader,
                                       [[maybe_unused]] Reader saved_reader, Iter begin, Iter end,
                                       Args&&... args)
        {
            // Create a node in the parse tree.
            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end);

            // Check that we're not creating a reserved identifier.
            if constexpr (sizeof...(Reserved) > 0)
            {
                using reserved = decltype((Reserved{} / ...));

                auto id_reader = lexy::partial_reader(saved_reader, end);
                if (lexy::engine_try_match<typename reserved::token_engine>(id_reader)
                    && id_reader.cur() == end)
                {
                    // We found a reserved identifier.
                    auto err = lexy::make_error<Reader, lexy::reserved_identifier>(begin, end);
                    context.on(_ev::error{}, err);
                    // But we can trivially recover, as we've still matched a well-formed
                    // identifier.
                }
            }

            // Skip whitespace and continue.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using engine = typename _idp<Leading, Trailing>::token_engine;

            // Trie to parse the pattern.
            [[maybe_unused]] auto saved_reader = reader;
            auto                  begin        = reader.cur();
            if (auto ec = engine::match(reader); ec != typename engine::error_code())
            {
                context.on(_ev::backtracked{}, begin, reader.cur());
                return lexy::rule_try_parse_result::backtracked;
            }
            auto end = reader.cur();

            // Check for reserved patterns, etc.
            return _parse_impl(context, reader, saved_reader, begin, end, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using pattern = _idp<Leading, Trailing>;
            using engine  = typename pattern::token_engine;

            // Parse the pattern.
            [[maybe_unused]] auto saved_reader = reader;
            auto                  begin        = reader.cur();
            if (auto ec = engine::match(reader); ec != typename engine::error_code())
            {
                pattern::token_error(context, reader, ec, begin);
                return false;
            }
            auto end = reader.cur();

            // Check for reserved patterns, etc.
            return _parse_impl(context, reader, saved_reader, begin, end, LEXY_FWD(args)...);
        }
    };

    template <typename R>
    constexpr auto _make_reserve(R r) const
    {
        return lexyd::token(r);
    }
    template <typename String, typename Id>
    constexpr auto _make_reserve(_kw<String, Id>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // We turn the keyword into a literal to be able to use a trie for matching.
        return _lit<String>{};
    }

    //=== dsl ===//
    /// Adds a set of reserved identifiers.
    template <typename... R>
    constexpr auto reserve(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        return _id<Leading, Trailing, Reserved..., decltype(_make_reserve(r))...>{};
    }

    /// Reserves everything starting with the given rule.
    template <typename... R>
    constexpr auto reserve_prefix(R... prefix) const
    {
        return reserve((prefix + lexyd::any)...);
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    constexpr auto reserve_containing(R...) const
    {
        return reserve(_contains<R>{}...);
    }

    /// Matches every identifier, ignoring reserved ones.
    constexpr auto pattern() const
    {
        return _idp<Leading, Trailing>{};
    }

    /// Matches the initial char set of an identifier.
    constexpr auto leading_pattern() const
    {
        return Leading{};
    }

    /// Matches the trailing char set of an identifier.
    constexpr auto trailing_pattern() const
    {
        return Trailing{};
    }
};

/// Creates an identifier that consists of one or more of the given tokens.
template <typename Token>
constexpr auto identifier(Token)
{
    return _id<Token, Token>{};
}

/// Creates an identifier that consists of one leading token followed by zero or more trailing
/// tokens.
template <typename LeadingToken, typename TrailingToken>
constexpr auto identifier(LeadingToken, TrailingToken)
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
        context.on(_ev::error{}, err);
    }
};

template <typename String, typename L, typename T, typename... R>
constexpr auto _keyword(_id<L, T, R...>)
{
    // We don't need the reserved words, remove them to keep type name short.
    static_assert(String::size > 0, "keyword must not be empty");
    return _kw<String, _id<L, T>>{};
}

#if LEXY_HAS_NTTP
/// Matches the keyword.
template <lexy::_detail::string_literal Str, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...> id)
{
    return _keyword<lexy::_detail::type_string<Str>>(id);
}
#endif

#define LEXY_KEYWORD(Str, Id) ::lexyd::_keyword<LEXY_NTTP_STRING(Str)>(Id)
} // namespace lexyd

#endif // LEXY_DSL_IDENTIFIER_HPP_INCLUDED


namespace lexy
{
struct different_identifier
{
    static LEXY_CONSTEVAL auto name()
    {
        return "different identifier";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Id>
struct _ctx_icreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(!Context::contains(Id{}));
            lexy::_detail::parse_context_var identifier_ctx(context, Id{}, lexy::lexeme<Reader>());
            return NextParser::parse(identifier_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Identifier>
struct _ctx_icap : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _cont
        {
            template <typename Context, typename Reader>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args,
                                     lexy::lexeme<Reader> lexeme)
            {
                context.get(Id{}) = lexeme;
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexeme);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Identifier, _cont<Args...>>::try_parse(context, reader,
                                                                            LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Identifier, _cont<Args...>>::parse(context, reader,
                                                                        LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Identifier, typename Tag>
struct _ctx_irem : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            // Trie to parse the pattern of the identifier only: we don't need a value nor need to
            // check for reserved identifier, because it must match the one we've succesfully parsed
            // earlier.
            using engine = typename decltype(Identifier{}.pattern())::token_engine;
            auto begin   = reader.cur();
            if (auto ec = engine::match(reader); ec != typename engine::error_code())
                return lexy::rule_try_parse_result::backtracked;
            auto lexeme = lexy::lexeme(reader, begin);

            if (!lexy::_detail::equal_lexemes(context.get(Id{}), lexeme))
                return lexy::rule_try_parse_result::backtracked;

            context.on(_ev::token{}, lexy::identifier_token_kind, lexeme.begin(), lexeme.end());
            // Don't produce a value.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Again, parse pattern only.
            using pattern = decltype(Identifier{}.pattern());
            using engine  = typename decltype(Identifier{}.pattern())::token_engine;
            auto begin    = reader.cur();
            if (auto ec = engine::match(reader); ec != typename engine::error_code())
            {
                pattern::token_error(context, reader, ec, begin);
                return false;
            }
            auto lexeme = lexy::lexeme(reader, begin);

            // Check that the identifier is the same.
            if (!lexy::_detail::equal_lexemes(context.get(Id{}), lexeme))
            {
                using tag = lexy::_detail::type_or<Tag, lexy::different_identifier>;
                auto err  = lexy::make_error<Reader, tag>(lexeme.begin(), lexeme.end());
                context.on(_ev::error{}, err);
                // We can trivially recover, as we still had a valid identifier.
            }

            context.on(_ev::token{}, lexy::identifier_token_kind, lexeme.begin(), lexeme.end());
            // Don't produce a value.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _ctx_irem<Id, Identifier, Error> error = {};
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, typename Identifier>
struct _ctx_identifier
{
    struct id
    {};

    constexpr auto create() const
    {
        return _ctx_icreate<id>{};
    }

    constexpr auto capture() const
    {
        return _ctx_icap<id, Identifier>{};
    }

    constexpr auto rematch() const
    {
        return _ctx_irem<id, Identifier, void>{};
    }
};

/// Declares a context variable that stores one instance of the given identifier.
template <typename Id, typename Leading, typename Trailing, typename... Reserved>
constexpr auto context_identifier(_id<Leading, Trailing, Reserved...>)
{
    return _ctx_identifier<Id, _id<Leading, Trailing, Reserved...>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED

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
constexpr auto opt(Rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "opt() requires a branch condition");
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
                using recovery = _try_recovery<Recover, NextParser>;
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
constexpr auto if_(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>, "if_() requires a branch condition");
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
    using trailing_rule = decltype(lexyd::if_(Branch{} >> lexyd::try_(lexyd::error<Tag>)));

    template <typename Context, typename Reader>
    LEXY_DSL_FUNC void report_trailing_error(Context&                  context, Reader&,
                                             typename Reader::iterator sep_pos)
    {
        auto err = lexy::make_error<Reader, Tag>(sep_pos);
        context.on(_ev::error{}, err);
    }

    //=== dsl ===//
    template <typename NewTag>
    static constexpr _sep<Branch, NewTag> trailing_error = {};
};

/// Defines a separator for a list.
template <typename Branch>
constexpr auto sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
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
constexpr auto trailing_sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _tsep<Branch>{};
}

template <typename Branch>
LEXY_DEPRECATED_SEP constexpr auto no_trailing_sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _sep<Branch, lexy::unexpected_trailing_separator>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED


#ifdef LEXY_IGNORE_DEPRECATED_OPT_LIST
#    define LEXY_DEPRECATED_OPT_LIST
#else
#    define LEXY_DEPRECATED_OPT_LIST                                                               \
        [[deprecated("`dsl::opt_list(...)` has been replaced by `dsl::opt(dsl::list(...))`")]]
#endif

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

    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, lexy::_detail::lazy_init<Sink>& sink,
                             Args&&... args)
    {
        auto& cb = sink.emplace(context.on(_ev::list{}, reader.cur()));
        if constexpr (sizeof...(Args) > 0)
            cb(LEXY_FWD(args)...);
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

template <typename Sep, typename NextParser>
struct _report_trailing_sep
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, typename Reader::iterator sep_pos,
                             Args&&... args)
    {
        // If trailing seperators are allowed, this does nothing.
        // Otherwise, we report the error but can trivially recover.
        Sep::report_trailing_error(context, reader, sep_pos);
        return NextParser::parse(context, reader, LEXY_FWD(args)...);
    }
};

template <typename NextParser>
struct _report_recovery_finish
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        context.on(_ev::recovery_finish{}, reader.cur());
        return NextParser::parse(context, reader, LEXY_FWD(args)...);
    }
};

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

        using item_parser = lexy::rule_parser<Item, _list_sink>;

        using sep_parser = _sep_parser<Sep>;
        using trailing_sep_parser
            = lexy::rule_parser<Term,
                                _report_trailing_sep<Sep, _list_finish<NextParser, PrevArgs...>>>;

        using term_parser = lexy::rule_parser<Term, _list_finish<NextParser, PrevArgs...>>;
        using recovery_term_parser
            = lexy::rule_parser<Term,
                                _report_recovery_finish<_list_finish<NextParser, PrevArgs...>>>;

        auto sep_pos = reader.cur();
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
                    // Parse term, and report error about trailing separator (if necessary).
                    if (auto result = trailing_sep_parser::try_parse(context, reader, sep_pos,
                                                                     LEXY_FWD(args)..., sink);
                        result != lexy::rule_try_parse_result::backtracked)
                    {
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
                context.on(_ev::recovery_start{}, reader.cur());
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
                            context.on(_ev::recovery_finish{}, reader.cur());
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
                            context.on(_ev::recovery_finish{}, reader.cur());
                            state = state::terminator;
                            break;
                        }

                        // Here we either try something else or start recovering from yet another
                        // failed item.
                    }

                    // Recovery succeeds when we reach the terminator.
                    if (auto result
                        = recovery_term_parser::try_parse(context, reader, LEXY_FWD(args)..., sink);
                        result != lexy::rule_try_parse_result::backtracked)
                    {
                        // We're now done with the entire list.
                        return static_cast<bool>(result);
                    }

                    // Recovery fails when we reach the limit.
                    using limit = typename decltype(RecoveryLimit{}.get_limit())::token_engine;
                    if (lexy::engine_peek<limit>(reader))
                    {
                        context.on(_ev::recovery_cancel{}, reader.cur());
                        return false;
                    }

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
            // We construct the sink lazily only if the branch is taken.
            using sink_t = std::decay_t<decltype(context.on(_ev::list{}, reader.cur()))>;
            lexy::_detail::lazy_init<sink_t> sink;

            // Try parsing the initial item.
            using item_parser = lexy::rule_parser<Item, _list_sink>;
            if (auto result = item_parser::try_parse(context, reader, sink);
                result != lexy::rule_try_parse_result::ok)
                // Either we backtracked the item, in which case we also backtrack.
                // Or we failed parsing the item, in which case we also fail.
                return result;

            // Continue with the rest of the items.
            // At this point, the branch has been taken.
            using continuation = _list_loop<Item, Sep, NextParser, Args...>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., *sink));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.on(_ev::list{}, reader.cur());

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
constexpr auto list(Item)
{
    static_assert(lexy::is_branch_rule<Item>,
                  "list() without a separator requires a branch condition");
    return _lst<Item, void>{};
}

/// Parses a list of items with the specified separator.
template <typename Item, typename Sep, typename Tag>
constexpr auto list(Item, _sep<Sep, Tag>)
{
    return _lst<Item, _sep<Sep, Tag>>{};
}

/// Parses a list of items with the specified separator that can be trailing.
template <typename Item, typename Sep>
constexpr auto list(Item, _tsep<Sep>)
{
    static_assert(lexy::is_branch_rule<Item>,
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
                auto sink = context.on(_ev::list{}, reader.cur());
                return _list_finish<NextParser, Args...>::parse(context, reader, LEXY_FWD(args)...,
                                                                sink);
            }
        }
    };
};

/// Parses a list that might be empty.
template <typename Item>
LEXY_DEPRECATED_OPT_LIST constexpr auto opt_list(Item)
{
    static_assert(lexy::is_branch_rule<Item>, "opt_list() requires a branch condition");
    return _olst<Item, void>{};
}
template <typename Item, typename Sep>
LEXY_DEPRECATED_OPT_LIST constexpr auto opt_list(Item, Sep)
{
    static_assert(lexy::is_branch_rule<Item>, "opt_list() requires a branch condition");
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
            auto sink = context.on(_ev::list{}, reader.cur());

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
            auto sink = context.on(_ev::list{}, reader.cur());

            // Try parsing the terminator.
            using term_parser = lexy::rule_parser<Term, NextParser>;
            if (auto result
                = term_parser::try_parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, and thus produced a nullopt.
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
                    context.on(_ev::backtracked{}, save.cur(), reader.cur());
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
                    using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                    auto err  = lexy::make_error<Reader, tag>(begin, end);
                    context.on(_ev::error{}, err);
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
            using token_parser = lexy::rule_parser<Token, lexy::discard_parser<Context>>;
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
                context.on(_ev::backtracked{}, save.cur(), reader.cur());
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, _idp<L, T>::token_kind(), save.cur(), reader.cur());
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
                // Otherwise, we don't call `context.on(_ev::token{}, )` or have the same end as the
                // non-optimized symbol parser.

                if (begin == reader.cur())
                {
                    // We need to parse the entire identifier from scratch.
                    // The identifier pattern does not produce a value, so we can safely discard.
                    using id_parser = lexy::rule_parser<_idp<L, T>, lexy::discard_parser<Context>>;
                    if (!id_parser::parse(context, reader))
                        // Didn't have an identifier, so different error.
                        return false;
                }
                else
                {
                    // We're having a prefix of a valid identifier.
                    // As an additional optimization, just need to parse the remaining characters.
                    lexy::engine_while<trailing_engine>::match(reader);
                    context.on(_ev::token{}, _idp<L, T>::token_kind(), begin, reader.cur());
                }
                auto end = reader.cur();

                // Now we can report the erorr.
                using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                auto err  = lexy::make_error<Reader, tag>(begin, end);
                context.on(_ev::error{}, err);
                return false;
            }
            auto end = reader.cur();

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, _idp<L, T>::token_kind(), begin, end);
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, _idp<L, T>, ErrorTag> error = {};
};

template <const auto& Table, typename Tag>
struct _sym<Table, void, Tag> : rule_base
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
            // Try to parse the symbol.
            auto save = reader;
            auto idx  = Table.try_parse(reader);
            if (!idx)
            {
                // We didn't have a symbol, so backtrack.
                context.on(_ev::backtracked{}, save.cur(), reader.cur());
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, lexy::identifier_token_kind, save.cur(), reader.cur());
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.cur();
            auto idx   = Table.try_parse(reader);
            if (!idx)
            {
                // We didn't have a symbol.
                using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                auto err  = lexy::make_error<Reader, tag>(begin);
                context.on(_ev::error{}, err);
                return false;
            }
            auto end = reader.cur();

            // We've succesfully matched a symbol.
            // Report its corresponding identifier token and produce the value.
            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end);
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[idx]);
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, void, ErrorTag> error = {};
};

template <const auto& Table>
struct _sym_dsl : _sym<Table, void, void>
{
    template <typename Token>
    constexpr auto operator()(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _sym<Table, Token, void>{};
    }
    template <typename L, typename T, typename... R>
    constexpr auto operator()(_id<L, T, R...> id) const
    {
        static_assert(sizeof...(R) == 0,
                      "symbol() must not be used in the presence of reserved identifiers");
        return _sym<Table, decltype(id.pattern()), void>{};
    }
};

/// Parses optional rule, then matches the resulting lexeme against the symbol table.
template <const auto& Table>
constexpr auto symbol = _sym_dsl<Table>{};
} // namespace lexyd

#endif // LEXY_DSL_SYMBOL_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_VALUE_HPP_INCLUDED
#define LEXY_DSL_VALUE_HPP_INCLUDED




#ifdef LEXY_IGNORE_DEPRECATED_VALUE
#    define LEXY_DEPRECATED_VALUE
#else
#    define LEXY_DEPRECATED_VALUE                                                                  \
        [[deprecated("`dsl::value_*()` has been replaced by `lexy::bind()`")]]
#endif

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
LEXY_DEPRECATED_VALUE constexpr auto value_c = _valc<Value>{};
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
LEXY_DEPRECATED_VALUE constexpr auto value_f = _valf<F>{};
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
LEXY_DEPRECATED_VALUE constexpr auto value_t = _valt<T>{};
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
LEXY_DEPRECATED_VALUE constexpr auto value_str = _vals<lexy::_detail::type_string<Str>>{};
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
            // Repeat loop if we've recovered.
            return engine::recover(reader, ec);
        }
        auto content_end = reader.cur();

        context.on(_ev::token{}, Char::token_kind(), content_begin, content_end);
        sink(lexy::lexeme<Reader>(content_begin, content_end));
    }
    else
    {
        auto content_begin = reader.cur();
        engine::match(reader);
        auto content_end = reader.cur();

        context.on(_ev::token{}, Char::token_kind(), content_begin, content_end);
        sink(lexy::lexeme<Reader>(content_begin, content_end));
    }

    return true;
}

template <typename Close, typename Char, typename Limit, typename... Escapes>
struct _del : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink      = context.on(_ev::list{}, reader.cur());
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
                    context.on(_ev::error{}, err);
                    return false;
                }
                // Try to parse the escape sequences.
                else if (auto result = lexy::rule_try_parse_result::backtracked;
                         // This tries to parse each escape in order until one doesn't backtrack.
                         // Then enters the if.
                         ((result = lexy::rule_parser<Escapes, _list_sink>::try_parse(context,
                                                                                      reader, sink),
                           result != lexy::rule_try_parse_result::backtracked)
                          || ...))
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

struct _escape_base
{};

template <typename Open, typename Close, typename Limit>
struct _delim_dsl
{
    /// Add tokens that will limit the delimited to detect a missing terminator.
    template <typename... Tokens>
    constexpr auto limit(Tokens...) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));
        return _delim_dsl<Open, Close, decltype((Limit{} / ... / Tokens{}))>{};
    }

    //=== rules ===//
    /// Sets the content.
    template <typename Char, typename... Escapes>
    constexpr auto operator()(Char, Escapes...) const
    {
        static_assert(lexy::is_token_rule<Char>);
        static_assert((std::is_base_of_v<_escape_base, Escapes> && ...));
        return no_whitespace(open() >> _del<Close, Char, Limit, Escapes...>{});
    }

    //=== access ===//
    /// Matches the open delimiter.
    constexpr auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    constexpr auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
constexpr auto delimited(Open, Close)
{
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
    return _delim_dsl<Open, Close, lexyd::_eof>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
constexpr auto delimited(Delim)
{
    static_assert(lexy::is_branch_rule<Delim>);
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
constexpr auto _escape_rule(Branches... branches)
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
            {
                context.on(_ev::backtracked{}, begin, reader.cur());
                return lexy::rule_try_parse_result::backtracked;
            }

            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::lexeme(reader, begin)));
        }
    };
};

template <typename Escape, typename... Branches>
struct _escape : decltype(_escape_rule<Escape>(Branches{}...)), _escape_base
{
    /// Adds a generic escape rule.
    template <typename Branch>
    constexpr auto rule(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the token.
    template <typename Token>
    constexpr auto capture(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return this->rule(_escape_cap<typename Token::token_engine>{});
    }

    /// Adds an escape rule that parses the symbol.
    template <const auto& Table, typename Rule>
    constexpr auto symbol(Rule rule) const
    {
        return this->rule(lexyd::symbol<Table>(rule));
    }
    template <const auto& Table>
    constexpr auto symbol() const
    {
        return this->rule(lexyd::symbol<Table>);
    }

#if LEXY_HAS_NTTP
    /// Adds an escape rule that replaces the escaped string with the replacement.
    template <lexy::_detail::string_literal Str, typename Value>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit(Value value) const
    {
        return rule(lexyd::lit<Str> >> value);
    }
    /// Adds an escape rule that replaces the escaped string with itself.
    template <lexy::_detail::string_literal Str>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit() const
    {
        return lit<Str>(value_str<Str>);
    }
#endif

    /// Adds an escape rule that replaces the escaped character with the replacement.
    template <auto C, typename Value>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit_c(Value value) const
    {
        return rule(lexyd::lit_c<C> >> value);
    }
    /// Adds an escape rule that replaces the escape character with itself.
    template <auto C>
    LEXY_DEPRECATED_ESCAPE constexpr auto lit_c() const
    {
        return lit_c<C>(value_c<C>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
constexpr auto escape(EscapeToken)
{
    static_assert(lexy::is_token_rule<EscapeToken>);
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
            auto err = lexy::make_error<Reader, lexy::forbidden_leading_zero>(pos, reader.cur());
            context.on(_ev::error{}, err);
        }
        else
        {
            auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
            auto err = lexy::make_error<Reader, lexy::forbidden_leading_zero>(pos, reader.cur());
            context.on(_ev::error{}, err);
        }
        else
        {
            auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, Base::name());
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
    constexpr auto operator()(Rule rule) const
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

#ifndef LEXY_DSL_INTEGER_HPP_INCLUDED
#define LEXY_DSL_INTEGER_HPP_INCLUDED

#include <climits>






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
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>,
                  "specialize integer_traits for your custom integer types");

    using type = T;

    static constexpr auto is_bounded = true;

    static constexpr auto _max = [] {
        if constexpr (std::is_same_v<T, char>)
            return CHAR_MAX; // NOLINT
        else if constexpr (std::is_same_v<T, signed char>)
            return SCHAR_MAX;
        else if constexpr (std::is_same_v<T, unsigned char>)
            return UCHAR_MAX; // NOLINT
        else if constexpr (std::is_same_v<T, wchar_t>)
            return WCHAR_MAX; // NOLINT
#if LEXY_HAS_CHAR8_T
        else if constexpr (std::is_same_v<T, char8_t>)
            return UCHAR_MAX; // NOLINT
#endif
        else if constexpr (std::is_same_v<T, char16_t>)
            return UINT_LEAST16_MAX;
        else if constexpr (std::is_same_v<T, char32_t>)
            return UINT_LEAST32_MAX;
        else if constexpr (std::is_same_v<T, signed short>)
            return SHRT_MAX;
        else if constexpr (std::is_same_v<T, unsigned short>)
            return USHRT_MAX;
        else if constexpr (std::is_same_v<T, signed int>)
            return INT_MAX;
        else if constexpr (std::is_same_v<T, unsigned int>)
            return UINT_MAX;
        else if constexpr (std::is_same_v<T, signed long>)
            return LONG_MAX;
        else if constexpr (std::is_same_v<T, unsigned long>)
            return ULONG_MAX;
        else if constexpr (std::is_same_v<T, signed long long>)
            return LLONG_MAX;
        else if constexpr (std::is_same_v<T, unsigned long long>)
            return ULLONG_MAX;
        else
            static_assert(_detail::error<T>);
    }();
    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, _max);

    template <int Radix>
    static constexpr void add_digit_unchecked(T& result, unsigned digit)
    {
        result = T(result * T(Radix) + T(digit));
    }

    template <int Radix>
    static constexpr bool add_digit_checked(T& result, unsigned digit)
    {
        constexpr auto can_use_unsigned = [] {
            if constexpr (sizeof(T) >= sizeof(unsigned))
                // If it's bigger or of the same size as unsigned, we can't use unsigned.
                return false;
            else
            {
                // We can do it if the worst-case does not overflow unsigned.
                auto worst_case = static_cast<unsigned>(_max);
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
            if (value > static_cast<unsigned>(_max))
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
            constexpr auto max_per_radix = T(_max / Radix);
            if (result > max_per_radix)
                return false;
            result = T(result * Radix);

            // result += digit
            if (result > T(_max - digit))
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

template <typename T, std::size_t N, int Radix>
constexpr bool _ndigits_can_overflow()
{
    using traits         = lexy::integer_traits<T>;
    auto max_digit_count = traits::template max_digit_count<Radix>;
    // We don't know whether the maximal value is a power of Radix,
    // so we have to be conservative and don't rule out overflow on the same count.
    return N >= max_digit_count;
}

// Parses T in the Base while checking for overflow.
template <typename T, typename Base>
struct _unbounded_integer_parser
{
    using traits      = lexy::integer_traits<T>;
    using result_type = typename traits::type;
    using base        = Base;

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
    using base        = Base;

    static constexpr auto radix = Base::radix;

    template <typename Iterator>
    static constexpr unsigned find_digit(Iterator& cur, Iterator end)
    {
        if constexpr (AssumeOnlyDigits)
        {
            if (cur == end)
                return unsigned(-1);
            else
                return Base::value(*cur++);
        }
        else
        {
            auto digit = 0u;
            do
            {
                if (cur == end)
                    return unsigned(-1);

                digit = Base::value(*cur++);
            } while (digit >= Base::radix);
            return digit;
        }
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
template <typename T, typename Base, bool AssumeOnlyDigits>
using _integer_parser
    = std::conditional_t<_is_bounded<T>, _bounded_integer_parser<T, Base, AssumeOnlyDigits>,
                         _unbounded_integer_parser<T, Base>>;

template <typename Rule, typename Sep, typename IntParser, typename Tag>
struct _int : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, bool& failed,
                                     typename Reader::iterator begin, Args&&... args)
            {
                failed   = false;
                auto end = reader.cur();

                using tag        = lexy::_detail::type_or<Tag, lexy::integer_overflow>;
                using error_type = lexy::error<typename Reader::canonical_reader, tag>;

                auto result = typename IntParser::result_type(0);
                if (!IntParser::parse(result, begin, end))
                    // Raise error but recover.
                    context.on(_ev::error{}, error_type(begin, end));

                return NextParser::parse(context, reader, LEXY_FWD(args)..., result);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto failed = true; // doesn't matter
            return lexy::rule_parser<Rule, _continuation>::try_parse(context, reader, failed,
                                                                     reader.cur(),
                                                                     LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto failed = true;
            auto begin  = reader.cur();

            // Parse the digits rule with the special continuation.
            auto result = lexy::rule_parser<Rule, _continuation>::parse(context, reader, failed,
                                                                        begin, LEXY_FWD(args)...);
            if (!failed)
            {
                // Propagate result of following rules.
                return result;
            }
            else
            {
                // Recover.
                context.on(_ev::recovery_start{}, reader.cur());
                if constexpr (std::is_void_v<Sep>)
                {
                    while (lexy::engine_try_match<typename IntParser::base::digit_set>(reader))
                    {}
                }
                else
                {
                    while (lexy::engine_try_match<typename IntParser::base::digit_set>(reader)
                           || lexy::engine_try_match<typename Sep::token_engine>(reader))
                    {}
                }
                context.on(_ev::recovery_finish{}, reader.cur());

                // Now try to convert this to an integer.
                return _continuation::parse(context, reader, failed, begin, LEXY_FWD(args)...);
            }
        }
    };
};

/// Parses the digits matched by the rule into an integer type.
template <typename T, typename Base, typename Rule>
constexpr auto integer(Rule)
{
    using parser = _integer_parser<T, Base, false>;
    return _int<Rule, void, parser, void>{};
}

template <typename T, typename Base>
constexpr auto integer(_digits<Base>)
{
    using parser = _integer_parser<T, Base, true>;
    return _int<_digits<Base>, void, parser, void>{};
}
template <typename T, typename Base, typename Sep>
constexpr auto integer(_digits_s<Base, Sep>)
{
    using parser = _integer_parser<T, Base, false>;
    return _int<_digits_s<Base, Sep>, Sep, parser, void>{};
}
template <typename T, typename Base>
constexpr auto integer(_digits_t<Base>)
{
    using parser = _integer_parser<T, Base, true>;
    return _int<_digits_t<Base>, void, parser, void>{};
}
template <typename T, typename Base, typename Sep>
constexpr auto integer(_digits_st<Base, Sep>)
{
    using parser = _integer_parser<T, Base, false>;
    return _int<_digits_st<Base, Sep>, Sep, parser, void>{};
}

template <typename T, typename Base, std::size_t N>
constexpr auto integer(_ndigits<N, Base>)
{
    using type
        = std::conditional_t<_ndigits_can_overflow<T, N, Base::radix>(), T, lexy::unbounded<T>>;
    using parser = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, void, parser, void>{};
}
template <typename T, typename Base, std::size_t N, typename Sep>
constexpr auto integer(_ndigits_s<N, Base, Sep>)
{
    using type
        = std::conditional_t<_ndigits_can_overflow<T, N, Base::radix>(), T, lexy::unbounded<T>>;
    using parser = _integer_parser<type, Base, false>;
    return _int<_ndigits_s<N, Base, Sep>, Sep, parser, void>{};
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
constexpr auto code_point_id = [] {
    using type   = std::conditional_t<_ndigits_can_overflow<lexy::code_point, N, Base::radix>(),
                                    lexy::code_point, lexy::unbounded<lexy::code_point>>;
    using parser = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, void, parser, lexy::invalid_code_point>{};
}();
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LABEL_HPP_INCLUDED
#define LEXY_DSL_LABEL_HPP_INCLUDED

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




#ifdef LEXY_IGNORE_DEPRECATED_LABEL
#    define LEXY_DEPRECATED_LABEL
#else
#    define LEXY_DEPRECATED_LABEL                                                                  \
        [[deprecated("`dsl::label/id()` has been deprecated; use productions instead")]]
#endif

namespace lexy
{
template <typename T, typename = void>
struct label
{};
template <typename T>
struct label<T, decltype(void(T::value))>
{
    constexpr operator decltype(T::value)() const
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
    constexpr auto operator()(Rule) const
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
LEXY_DEPRECATED_LABEL constexpr auto label = _lab<Label>{};

/// Matches with the specified id.
template <auto Id>
LEXY_DEPRECATED_LABEL constexpr auto id = _lab<std::integral_constant<int, Id>>{};
} // namespace lexyd

#endif // LEXY_DSL_LABEL_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED





namespace lexy
{
/// We've failed to match a lookahead.
struct lookahead_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "lookahead failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Needle, typename End, typename Tag>
struct _look : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using engine = lexy::engine_find_before<Needle, End>;
            if (!lexy::engine_peek<engine>(context, reader))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using engine = lexy::engine_find_before<Needle, End>;
            if (!lexy::engine_peek<engine>(context, reader))
            {
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::make_error<Reader, tag>(reader.cur());
                context.on(_ev::error{}, err);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _look<Needle, End, Error> error = {};
};

/// Looks for the Needle before End.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
constexpr auto lookahead(Needle, End)
{
    static_assert(lexy::is_token_rule<Needle> && lexy::is_token_rule<End>);
    return _look<typename Needle::token_engine, typename End::token_engine, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MEMBER_HPP_INCLUDED
#define LEXY_DSL_MEMBER_HPP_INCLUDED





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
    constexpr _mem_dsl(Fn = {}) {}

    template <typename Rule>
    constexpr auto operator=(Rule) const // NOLINT: it _is_ an unconventional assignment operator
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
            context.on(_ev::error{}, err);
        }
        else
        {
            Token::token_error(context, reader, token_engine::error_to_matcher(ec), pos);
        }
    }
};

/// Matches Token unless Except matches on the input Token matched.
template <typename Token, typename Except>
constexpr auto operator-(Token, Except)
{
    static_assert(lexy::is_token_rule<Token>);
    static_assert(lexy::is_token_rule<Except>);
    return _minus<Token, Except>{};
}
template <typename Token, typename E, typename Except>
constexpr auto operator-(_minus<Token, E>, Except except)
{
    static_assert(lexy::is_token_rule<Except>);
    return _minus<Token, decltype(E{} / except)>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PEEK_HPP_INCLUDED
#define LEXY_DSL_PEEK_HPP_INCLUDED




namespace lexy
{
/// We've failed to match a peek.
struct peek_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "peek failure";
    }
};

/// We've failed to match a peek not.
struct unexpected
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Engine, typename Tag>
struct _peek : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (!lexy::engine_peek<Engine>(context, reader))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!lexy::engine_peek<Engine>(context, reader))
            {
                using tag = lexy::_detail::type_or<Tag, lexy::peek_failure>;
                auto err  = lexy::make_error<Reader, tag>(reader.cur());
                context.on(_ev::error{}, err);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peek<Engine, Error> error = {};
};

template <typename Engine, typename Tag>
struct _peekn : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (lexy::engine_peek<Engine>(context, reader))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto copy = reader;
            if (auto begin = copy.cur(); lexy::engine_try_match<Engine>(copy))
            {
                auto end = copy.cur();
                context.on(_ev::backtracked{}, begin, end);

                using tag = lexy::_detail::type_or<Tag, lexy::unexpected>;
                auto err  = lexy::make_error<Reader, tag>(begin, end);
                context.on(_ev::error{}, err);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peekn<Engine, Error> error = {};
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek(Rule rule)
{
    using token = decltype(token(rule));
    return _peek<typename token::token_engine, void>{};
}

/// Check if at this reader position, the rule would not match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek_not(Rule rule)
{
    using token = decltype(token(rule));
    return _peekn<typename token::token_engine, void>{};
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
            context.on(_ev::token{}, lexy::position_token_kind, pos, pos);
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
/// Parses the rule of the production as if it were part of the current production.
template <typename Production>
constexpr auto inline_ = lexy::production_rule<Production>{};
} // namespace lexyd

namespace lexyd
{
template <typename Production>
struct _prd : rule_base
{
    using _rule = lexy::production_rule<Production>;

    static constexpr auto is_branch               = _rule::is_branch;
    static constexpr auto is_unconditional_branch = _rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser : lexy::_detail::production_parser<Production, NextParser>
    {};
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};

template <typename Production>
struct _rec : rule_base
{
    template <typename NextParser>
    struct parser : lexy::_detail::production_parser<Production, NextParser>
    {};
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
    inline constexpr auto(Name) = _##Name {}

LEXY_PUNCT(period, ".");
LEXY_PUNCT(comma, ",");
LEXY_PUNCT(colon, ":");
LEXY_PUNCT(semicolon, ";");

LEXY_PUNCT(exclamation_mark, "!");
LEXY_PUNCT(question_mark, "?");

LEXY_PUNCT(hyphen, "-");
LEXY_PUNCT(slash, "/");
LEXY_PUNCT(backslash, "\\");
LEXY_PUNCT(apostrophe, "'");
LEXY_PUNCT(ampersand, "&");
LEXY_PUNCT(caret, "^");
LEXY_PUNCT(asterisk, "*");
LEXY_PUNCT(tilde, "~");

LEXY_PUNCT(hash_sign, "#");
LEXY_PUNCT(dollar_sign, "$");
LEXY_PUNCT(at_sign, "@");
LEXY_PUNCT(percent_sign, "%");
LEXY_PUNCT(equal_sign, "=");
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
    using parser = lexy::_detail::final_parser;
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







namespace lexy
{
template <int I>
struct _sign
{
    constexpr operator int() const
    {
        return I;
    }
};

struct plus_sign : _sign<+1>
{};
struct minus_sign : _sign<-1>
{};
} // namespace lexy

namespace lexyd
{
template <typename Sign>
struct _sign : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., Sign{});
        }
    };
};

/// Matches a plus sign or nothing, producing +1.
constexpr auto plus_sign = if_(LEXY_LIT("+") >> _sign<lexy::plus_sign>{});
/// Matches a minus sign or nothing, producing +1 or -1.
constexpr auto minus_sign = if_(LEXY_LIT("-") >> _sign<lexy::minus_sign>{});

/// Matches a plus or minus sign or nothing, producing +1 or -1.
constexpr auto sign
    = if_(LEXY_LIT("+") >> _sign<lexy::plus_sign>{} | LEXY_LIT("-") >> _sign<lexy::minus_sign>{});
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
        using tag = lexy::_detail::type_or<Tag, lexy::exhausted_switch>;
        auto err  = lexy::make_error<Reader, tag>(save.cur(), reader.cur());
        context.on(_ev::error{}, err);
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
    constexpr auto case_(_br<Token, Value>) const
    {
        static_assert(lexy::is_token_rule<Token>, "case condition must be a token");
        return _switch<Rule, Error, Cases..., _switch_case<Token, Value>>{};
    }

    /// Adds a default value to the switch.
    template <typename Default>
    constexpr auto default_(Default) const
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
LEXY_DEPRECATED_SWITCH constexpr auto switch_(Rule)
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





namespace lexyd
{
template <std::size_t N, typename Rule>
constexpr auto _gen_times(Rule rule)
{
    if constexpr (N == 1)
        return rule;
    else
        return rule + _gen_times<N - 1>(rule);
}
template <std::size_t N, typename Rule, typename Sep>
constexpr auto _gen_times(Rule rule, Sep)
{
    if constexpr (N == 1)
        return rule + typename Sep::trailing_rule{};
    else
        return rule + typename Sep::rule{} + _gen_times<N - 1>(rule, Sep{});
}

template <std::size_t N, typename Rule, typename Sep>
struct _times : rule_base
{
    static constexpr auto _repeated_rule()
    {
        if constexpr (std::is_same_v<Sep, void>)
            return _gen_times<N>(Rule{});
        else
            return _gen_times<N>(Rule{}, Sep{});
    }

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using rule = decltype(_repeated_rule());
            return lexy::rule_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeats the rule N times in sequence.
template <std::size_t N, typename Rule>
constexpr auto times(Rule)
{
    static_assert(N > 0);
    return _times<N, Rule, void>{};
}

/// Repeates the rule N times in sequence separated by a separator.
template <std::size_t N, typename Rule, typename Sep>
constexpr auto times(Rule, Sep)
{
    static_assert(N > 0);
    return _times<N, Rule, Sep>{};
}

template <typename Rule>
constexpr auto twice(Rule rule)
{
    return times<2>(rule);
}
template <typename Rule, typename Sep>
constexpr auto twice(Rule rule, Sep sep)
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
    constexpr auto or_eof() const
    {
        return _until_eof<Condition>{};
    }
};

/// Matches anything until Condition matches.
/// Then matches Condition.
template <typename Condition>
constexpr auto until(Condition)
{
    static_assert(lexy::is_token_rule<Condition>);
    return _until<Condition>{};
}
} // namespace lexyd

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED




#endif // LEXY_DSL_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED
#define LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED

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
    static_assert(std::is_trivial_v<char_type>);

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

    buffer& operator=(const buffer& other) // NOLINT: we do guard against self-assignment
    {
        // Create a temporary buffer that owns the same memory as other but with our resource.
        // We then move assign it to *this.
        *this = buffer(other, _resource.get());
        return *this;
    }

    // NOLINTNEXTLINE: Unfortunately, sometimes move is not noexcept.
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
    const char_type* data() const noexcept
    {
        return _data;
    }

    std::size_t size() const noexcept
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

        if (size >= 4)
        {
            if (memory[0] == 0xFF && memory[1] == 0xFE && memory[2] == 0x00 && memory[3] == 0x00)
                return utf32_little(memory + 4, size - 4, resource);
            else if (memory[0] == 0x00 && memory[1] == 0x00 && memory[2] == 0xFE && memory[3])
                return utf32_big(memory + 4, size - 4, resource);
        }

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


namespace lexy_ext
{
/// Input that uses Compiler Explorer's stdin.
/// It consumes the entire stdin, which is then at EOF.
lexy::buffer<lexy::utf8_encoding> compiler_explorer_input()
{
    // We can't use ftell() to get file size
    // So instead use a conservative loop.
    lexy::_detail::buffer_builder<char> builder;
    while (true)
    {
        const auto buffer_size = builder.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from stdin,
        // commiting what we've just read.
        const auto read = std::fread(builder.write_data(), sizeof(char), buffer_size, stdin);
        builder.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            // We should have reached the end.
            LEXY_ASSERT(!std::ferror(stdin), "read error");
            LEXY_ASSERT(std::feof(stdin), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        builder.grow();
    }

    return lexy::buffer<lexy::utf8_encoding>(builder.read_data(), builder.read_size());
}

//=== convenience typedefs ===//
using compiler_explorer_lexeme = lexy::buffer_lexeme<lexy::utf8_encoding>;

template <typename Tag>
using compiler_explorer_error = lexy::buffer_error<Tag, lexy::utf8_encoding>;

template <typename Production>
using compiler_explorer_error_context = lexy::buffer_error_context<Production, lexy::utf8_encoding>;
} // namespace lexy_ext

#endif // LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>




namespace lexy_ext::_detail
{
// Split the context of the location into three parts: the one before underlined, the underlined
// one, and the one after. If underlined covers multiple lines, limit to the one of the context or
// the newline afterwards.
template <typename Location, typename Reader>
auto split_context(const Location& location, const lexy::lexeme<Reader>& underlined)
{
    struct result_t
    {
        lexy::lexeme<Reader> before;
        lexy::lexeme<Reader> underlined;
        lexy::lexeme<Reader> after;
    } result;

    auto context  = location.context();
    result.before = {context.begin(), underlined.begin()};

    auto underlined_end = underlined.begin();
    // Find either the end of the underline, or the end of the context.
    while (underlined_end != underlined.end() && underlined_end != context.end())
        ++underlined_end;

    if (underlined.begin() == underlined_end)
    {
        // We actually want the newline, so include it.
        auto newline = location.newline();
        while (underlined_end != underlined.end() && underlined_end != newline.end())
            ++underlined_end;

        result.underlined = {underlined.begin(), underlined_end};
        result.after      = {underlined_end, underlined_end}; // Nothing afterwards possible.
    }
    else
    {
        result.underlined = {underlined.begin(), underlined_end};
        result.after      = {underlined_end, context.end()};
    }

    return result;
}

enum class annotation_kind
{
    primary,
    secondary,
};

template <typename Input>
struct error_writer
{
    lexy::visualization_options opts;

    const auto* column() const
    {
        if (opts.is_set(lexy::visualize_use_unicode))
            return u8"│";
        else
            return u8"|";
    }

    const auto* underline(annotation_kind kind) const
    {
        switch (kind)
        {
        case annotation_kind::primary:
            return "^";
        case annotation_kind::secondary:
            return "~";
        }

        return "";
    }

    //=== writers ===//
    template <typename OutputIt, typename Writer>
    OutputIt write_message(OutputIt out, const Writer& message)
    {
        using namespace lexy::_detail;

        out    = write_color<color::red, color::bold>(out, opts);
        out    = write_str(out, "error: ");
        out    = write_color<color::reset>(out, opts);
        out    = message(out, opts);
        *out++ = '\n';

        return out;
    }

    template <typename OutputIt>
    OutputIt write_empty_annotation(OutputIt out) const
    {
        using namespace lexy::_detail;

        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = '\n';
        return out;
    }

    template <typename OutputIt, typename Location, typename Writer>
    OutputIt write_annotation(OutputIt out, annotation_kind kind, const Location& location,
                              const lexy::lexeme_for<Input>& _underlined,
                              const Writer&                  message) const
    {
        using namespace lexy::_detail;

        auto colorize_underline = [&](OutputIt out) {
            switch (kind)
            {
            case annotation_kind::primary:
                return write_color<color::red, color::bold>(out, opts);
            case annotation_kind::secondary:
                return write_color<color::yellow>(out, opts);
            }

            return out;
        };

        auto [before, underlined, after] = split_context(location, _underlined);

        //=== Line with file contents ===//
        // Location column.
        out    = write_format(out, "%4zd ", location.line_nr());
        out    = write_str(out, column());
        *out++ = ' ';

        // Print before underlined normally.
        out = lexy::visualize_to(out, before, opts);

        // Print underlined colored.
        out = colorize_underline(out);
        out = lexy::visualize_to(out, underlined, opts.reset(lexy::visualize_use_color));
        out = write_color<color::reset>(out, opts);

        // Print after underlined normally.
        out    = lexy::visualize_to(out, after, opts);
        *out++ = '\n';

        //==== Line with annotation ===//
        // Initial column.
        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = ' ';

        // Indent until the underline.
        auto indent_count = lexy::visualization_display_width(before, opts);
        for (auto i = 0u; i != indent_count; ++i)
            *out++ = ' ';

        // Colorize.
        out = colorize_underline(out);

        // Then underline.
        auto underline_count = lexy::visualization_display_width(underlined, opts);
        if (underline_count == 0)
            underline_count = 1;
        for (auto i = 0u; i != underline_count; ++i)
            out = write_str(out, underline(kind));
        *out++ = ' ';

        // Print the message.
        out    = message(out, opts.reset(lexy::visualize_use_color));
        *out++ = '\n';

        return write_color<color::reset>(out, opts);
    }
};
} // namespace lexy_ext::_detail

namespace lexy_ext::_detail
{
template <typename OutputIt, typename Production, typename Input, typename Reader, typename Tag>
OutputIt write_error(OutputIt out, const lexy::error_context<Production, Input>& context,
                     const lexy::error<Reader, Tag>& error, lexy::visualization_options opts)
{
    _detail::error_writer<Input> writer{opts};

    // Convert the context location and error location into line/column information.
    lexy_ext::input_location_finder finder(context.input());

    auto context_location = finder.find(context.position());
    auto location         = finder.find(error.position(), context_location);

    // Write the main error headline.
    out = writer.write_message(out, [&](OutputIt out, lexy::visualization_options) {
        out = lexy::_detail::write_str(out, "while parsing ");
        out = lexy::_detail::write_str(out, context.production());
        return out;
    });
    out = writer.write_empty_annotation(out);

    // Write an annotation for the context.
    if (location.line_nr() != context_location.line_nr())
    {
        out = writer.write_annotation(out, annotation_kind::secondary, context_location,
                                      {context.position(), 1},
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, "beginning here");
                                      });
        out = writer.write_empty_annotation(out);
    }

    // Write the main annotation.
    if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string());

        out = writer.write_annotation(out, annotation_kind::primary, location,
                                      {error.position(), error.index() + 1},
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string());

        out = writer.write_annotation(out, annotation_kind::primary, location,
                                      {error.begin(), error.end()},
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected keyword '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
    {
        out = writer.write_annotation(out, annotation_kind::primary, location,
                                      {error.position(), 1},
                                      [&](OutputIt out, lexy::visualization_options) {
                                          out = lexy::_detail::write_str(out, "expected '");
                                          out = lexy::_detail::write_str(out,
                                                                         error.character_class());
                                          out = lexy::_detail::write_str(out, "' character");
                                          return out;
                                      });
    }
    else
    {
        auto underlined = [&] {
            if (error.begin() == error.end())
                return lexy::lexeme_for<Input>(error.position(), 1);
            else
                return lexy::lexeme_for<Input>(error.begin(), error.end());
        }();

        out = writer.write_annotation(out, annotation_kind::primary, location, underlined,
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, error.message());
                                      });
    }

    return out;
}
} // namespace lexy_ext::_detail

namespace lexy_ext
{
struct _report_error
{
    struct _sink
    {
        std::size_t _count;

        using return_type = std::size_t;

        template <typename Production, typename Input, typename Reader, typename Tag>
        void operator()(const lexy::error_context<Production, Input>& context,
                        const lexy::error<Reader, Tag>&               error)
        {
            _detail::write_error(lexy::cfile_output_iterator{stderr}, context, error,
                                 {lexy::visualize_fancy});
            ++_count;
        }

        std::size_t finish() &&
        {
            std::fputs("\n", stderr);
            return _count;
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

// The error callback that prints to stderr.
constexpr auto report_error = _report_error{};
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

