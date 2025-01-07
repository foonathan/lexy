#define LEXY_HAS_UNICODE_DATABASE 1
// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED
#define LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_BASE_HPP_INCLUDED
#define LEXY_ACTION_BASE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

#ifndef LEXY_HAS_UNICODE_DATABASE
#    define LEXY_HAS_UNICODE_DATABASE 0
#endif

#ifndef LEXY_EXPERIMENTAL
#    define LEXY_EXPERIMENTAL 0
#endif

//=== utility traits===//
#define LEXY_MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define LEXY_FWD(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define LEXY_DECLVAL(...) lexy::_detail::declval<__VA_ARGS__>()

#define LEXY_DECAY_DECLTYPE(...) std::decay_t<decltype(__VA_ARGS__)>

/// Creates a new type from the instantiation of a template.
/// This is used to shorten type names.
#define LEXY_INSTANTIATION_NEWTYPE(Name, Templ, ...)                                               \
    struct Name : Templ<__VA_ARGS__>                                                               \
    {                                                                                              \
        using Templ<__VA_ARGS__>::Templ;                                                           \
    }

namespace lexy::_detail
{
template <typename... T>
constexpr bool error = false;

template <typename T>
std::add_rvalue_reference_t<T> declval();

template <typename T>
constexpr void swap(T& lhs, T& rhs) noexcept
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
//   See https://github.com/foonathan/lexy/issues/15.
#    if __cpp_nontype_template_parameter_class >= 201806 || __cpp_nontype_template_args >= 201911
#        define LEXY_HAS_NTTP 1
#    else
#        define LEXY_HAS_NTTP 0
#    endif
#endif

#if LEXY_HAS_NTTP
#    define LEXY_NTTP_PARAM auto
#else
#    define LEXY_NTTP_PARAM const auto&
#endif

//=== consteval ===//
#ifndef LEXY_HAS_CONSTEVAL
#    if defined(_MSC_VER) && !defined(__clang__)
//       Currently can't handle returning strings from consteval, check back later.
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

//=== constexpr ===//
#ifndef LEXY_HAS_CONSTEXPR_DTOR
#    if __cpp_constexpr_dynamic_alloc
#        define LEXY_HAS_CONSTEXPR_DTOR 1
#    else
#        define LEXY_HAS_CONSTEXPR_DTOR 0
#    endif
#endif

#if LEXY_HAS_CONSTEXPR_DTOR
#    define LEXY_CONSTEXPR_DTOR constexpr
#else
#    define LEXY_CONSTEXPR_DTOR
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
} // namespace lexy

#    define LEXY_CHAR_OF_u8 char
#    define LEXY_CHAR8_T ::lexy::_char8_t
#    define LEXY_CHAR8_STR(Str)                                                                    \
        LEXY_NTTP_STRING(::lexy::_detail::type_string, u8##Str)::c_str<LEXY_CHAR8_T>

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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED
#define LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_STD_HPP_INCLUDED
#define LEXY_DETAIL_STD_HPP_INCLUDED



//=== iterator tags ===//
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

//=== (constexpr) construct_at ===//
#if !LEXY_HAS_CONSTEXPR_DTOR

namespace lexy::_detail
{
// We don't have constexpr dtor's, so this is just a regular function.
template <typename T, typename... Args>
T* construct_at(T* ptr, Args&&... args)
{
    return ::new ((void*)ptr) T(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#elif defined(_MSC_VER)

namespace lexy::_detail
{
// MSVC can make it constexpr if marked with an attribute given by a macro.
template <typename T, typename... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
#    if defined(_MSVC_CONSTEXPR)
    _MSVC_CONSTEXPR
#    endif
    return ::new ((void*)ptr) T(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#else

namespace lexy::_detail
{
struct _construct_at_tag
{};
} // namespace lexy::_detail

namespace std
{
// GCC only allows constexpr placement new inside a function called `std::construct_at`.
// So we write our own.
template <typename T, typename... Args>
constexpr T* construct_at(lexy::_detail::_construct_at_tag, T* ptr, Args&&... args)
{
    return ::new ((void*)ptr) T(LEXY_FWD(args)...);
}
} // namespace std

namespace lexy::_detail
{
template <typename T, typename... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
    return std::construct_at(lexy::_detail::_construct_at_tag{}, ptr, LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif

#endif // LEXY_DETAIL_STD_HPP_INCLUDED


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

    template <typename... Args>
    constexpr void _construct(Args&&... args)
    {
        *this = _lazy_init_storage_trivial(0, LEXY_FWD(args)...);
    }
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
    LEXY_CONSTEXPR_DTOR void _construct(Args&&... args)
    {
        _detail::construct_at(&_value, LEXY_FWD(args)...);
        _init = true;
    }

    // Cannot add noexcept due to https://github.com/llvm/llvm-project/issues/59854.
    LEXY_CONSTEXPR_DTOR ~_lazy_init_storage_non_trivial() /* noexcept */
    {
        if (_init)
            _value.~T();
    }

    LEXY_CONSTEXPR_DTOR _lazy_init_storage_non_trivial(
        _lazy_init_storage_non_trivial&& other) noexcept
    : _init(other._init), _empty()
    {
        if (_init)
            _detail::construct_at(&_value, LEXY_MOV(other._value));
    }

    LEXY_CONSTEXPR_DTOR _lazy_init_storage_non_trivial& operator=(
        _lazy_init_storage_non_trivial&& other) noexcept
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
            _detail::construct_at(&_value, LEXY_MOV(other._value));
            _init = true;
        }
        else
        {
            // Both not initialized, nothing to do.
        }

        return *this;
    }
};

template <typename T>
constexpr auto _lazy_init_trivial = [] {
    // https://www.foonathan.net/2021/03/trivially-copyable/
    return std::is_trivially_destructible_v<T>          //
           && std::is_trivially_copy_constructible_v<T> //
           && std::is_trivially_copy_assignable_v<T>    //
           && std::is_trivially_move_constructible_v<T> //
           && std::is_trivially_move_assignable_v<T>;
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
        if (*this)
            this->_value = T(LEXY_FWD(args)...);
        else
            this->_construct(LEXY_FWD(args)...);

        return this->_value;
    }

    template <typename Fn, typename... Args>
    constexpr T& emplace_result(Fn&& fn, Args&&... args)
    {
        return emplace(LEXY_FWD(fn)(LEXY_FWD(args)...));
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
template <typename T>
class lazy_init<T&>
{
public:
    using value_type = T&;

    constexpr lazy_init() noexcept : _ptr(nullptr) {}

    constexpr T& emplace(T& ref)
    {
        _ptr = &ref;
        return ref;
    }

    template <typename Fn, typename... Args>
    constexpr T& emplace_result(Fn&& fn, Args&&... args)
    {
        return emplace(LEXY_FWD(fn)(LEXY_FWD(args)...));
    }

    constexpr explicit operator bool() const noexcept
    {
        return _ptr != nullptr;
    }

    constexpr T& operator*() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return *_ptr;
    }

    constexpr T* operator->() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _ptr;
    }

private:
    T* _ptr;
};
template <>
class lazy_init<void>
{
public:
    using value_type = void;

    constexpr lazy_init() noexcept : _init(false) {}

    constexpr void emplace()
    {
        _init = true;
    }
    template <typename Fn, typename... Args>
    constexpr void emplace_result(Fn&& fn, Args&&... args)
    {
        LEXY_FWD(fn)(LEXY_FWD(args)...);
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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED
#define LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_DETECT_HPP_INCLUDED
#define LEXY_DETAIL_DETECT_HPP_INCLUDED



namespace lexy::_detail
{
template <typename... Args>
using void_t = void;

template <template <typename...> typename Op, typename Void, typename... Args>
struct _detector : std::false_type
{
    template <typename Fallback>
    using type_or = Fallback;
};
template <template <typename...> typename Op, typename... Args>
struct _detector<Op, void_t<Op<Args...>>, Args...> : std::true_type
{
    template <typename Fallback>
    using type_or = Op<Args...>;
};

template <template <typename...> typename Op, typename... Args>
constexpr bool is_detected = _detector<Op, void, Args...>::value;

template <typename Fallback, template <typename...> typename Op, typename... Args>
using detected_or = typename _detector<Op, void, Args...>::template type_or<Fallback>;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_DETECT_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

    constexpr bool starts_with(basic_string_view prefix) const noexcept
    {
        return substr(0, prefix.size()) == prefix;
    }
    constexpr bool try_remove_prefix(basic_string_view prefix) noexcept
    {
        if (!starts_with(prefix))
            return false;

        remove_prefix(prefix.length());
        return true;
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
inline constexpr const auto* make_cstr = _string_view_holder<FnPtr>::value;
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
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = string_view("auto lexy::_detail::_full_type_name() [T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    function.try_remove_prefix("(anonymous namespace)::");
    return function;

#elif defined(__GNUC__)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1
#    if __GNUC__ > 8
#        define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1
#    else
#        define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 0
#    endif

    constexpr auto prefix
        = string_view("constexpr auto lexy::_detail::_full_type_name() [with T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    function.try_remove_prefix("{anonymous}::");
    return function;

#elif defined(_MSC_VER)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = string_view("auto __cdecl lexy::_detail::_full_type_name<");
    constexpr auto suffix = string_view(">(void)");

    auto function = string_view(__FUNCSIG__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    function.try_remove_prefix("struct ") || function.try_remove_prefix("class ");
    function.try_remove_prefix("`anonymous-namespace'::");
    return function;

#else
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 0
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 0

    return string_view("unknown-type");

#endif
}

template <typename T, int NsCount>
constexpr string_view _type_name()
{
    auto name = _full_type_name<T>();
    if (name.find('<') != string_view::npos && NsCount != 0)
        return name;

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
constexpr const char* type_name()
{
    if constexpr (_detail::is_detected<_detect_name_f, T>)
        return T::name();
    else if constexpr (_detail::is_detected<_detect_name_v, T>)
        return T::name;
    else if constexpr (LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME)
        return make_cstr<_type_name<T, NsCount>>;
    else
        return "unknown-type";
}

template <typename T, int NsCount>
inline constexpr const char* _type_id_holder = type_name<T, NsCount>();

// Returns a unique address for each type.
// For implementation reasons, it also doubles as the pointer to the name.
template <typename T, int NsCount = 1>
constexpr const char* const* type_id()
{
    if constexpr (_detail::is_detected<_detect_name_v, T> //
                  && !_detail::is_detected<_detect_name_f, T>)
    {
        // We can use the address of the static constexpr directly.
        return &T::name;
    }
    else
    {
        // We instantiate a variable template with a function unique by type.
        // As the variable is inline, there should be a single address only.
        return &_type_id_holder<T, NsCount>;
    }
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_NOOP_HPP_INCLUDED
#define LEXY_CALLBACK_NOOP_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_BASE_HPP_INCLUDED
#define LEXY_CALLBACK_BASE_HPP_INCLUDED



// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
    static constexpr auto invoke(F ClassT::*f,
                                 Args&&... args) -> decltype(_invoke(0, f, LEXY_FWD(args)...))
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

template <typename T, typename State>
using _detect_callback_state = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(State&)]);
template <typename T, typename State>
constexpr bool is_callback_state
    = _detail::is_detected<_detect_callback_state, T, std::decay_t<State>>;

template <typename T, typename State, typename... Args>
using _detect_callback_with_state_for
    = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(State&)](LEXY_DECLVAL(Args)...));
template <typename T, typename State, typename... Args>
constexpr bool is_callback_with_state_for
    = _detail::is_detected<_detect_callback_with_state_for, std::decay_t<T>, State, Args...>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink_callback_for = decltype(LEXY_DECLVAL(T&)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_sink_callback_for
    = _detail::is_detected<_detect_sink_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(const T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
} // namespace lexy

namespace lexy
{
template <typename Fn>
struct _fn_holder
{
    Fn fn;

    constexpr explicit _fn_holder(Fn fn) : fn(fn) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_detail::invoke(fn,
                                                                                LEXY_FWD(args)...))
    {
        return _detail::invoke(fn, LEXY_FWD(args)...);
    }
};

template <typename Fn>
using _fn_as_base = std::conditional_t<std::is_class_v<Fn>, Fn, _fn_holder<Fn>>;

template <typename... Fns>
struct _overloaded : _fn_as_base<Fns>...
{
    constexpr explicit _overloaded(Fns... fns) : _fn_as_base<Fns>(LEXY_MOV(fns))... {}

    using _fn_as_base<Fns>::operator()...;
};

template <typename... Op>
constexpr auto _make_overloaded(Op&&... op)
{
    if constexpr (sizeof...(Op) == 1)
        return (LEXY_FWD(op), ...);
    else
        return _overloaded(LEXY_FWD(op)...);
}
} // namespace lexy

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED

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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED



// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_GRAMMAR_HPP_INCLUDED
#define LEXY_GRAMMAR_HPP_INCLUDED

#include <cstdint>





//=== rule ===//
// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct rule_base
{};

struct branch_base : rule_base
{};

struct unconditional_branch_base : branch_base
{};

struct _token_base
{};
struct _char_class_base
{};
struct _lset_base
{};
struct _lit_base
{};
struct _sep_base
{};

struct _operation_base
{};
} // namespace lexyd

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;

template <typename T>
constexpr bool is_branch_rule = std::is_base_of_v<dsl::branch_base, T>;
template <typename T>
constexpr bool is_unconditional_branch_rule = std::is_base_of_v<dsl::unconditional_branch_base, T>;

template <typename T>
constexpr bool is_token_rule = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_char_class_rule = std::is_base_of_v<dsl::_char_class_base, T>;
template <typename T>
constexpr bool is_literal_rule = std::is_base_of_v<dsl::_lit_base, T>;
template <typename T>
constexpr bool is_literal_set_rule = std::is_base_of_v<dsl::_lset_base, T>;

template <typename T>
constexpr auto is_separator = std::is_base_of_v<lexy::dsl::_sep_base, T>;

template <typename T>
constexpr auto is_operation = std::is_base_of_v<lexy::dsl::_operation_base, T>;

template <typename... T>
constexpr bool _require_branch_rule = (is_branch_rule<T> && ...);
} // namespace lexy

#define LEXY_REQUIRE_BRANCH_RULE(Rule, Name)                                                       \
    static_assert(lexy::_require_branch_rule<Rule>, Name                                           \
                  " requires a branch condition."                                                  \
                  " You may need to use `>>` to specify the condition that is used for dispatch."  \
                  " See https://lexy.foonathan.net/learn/branching/ for more information.")

//=== predefined_token_kind ===//
namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind = UINT_LEAST16_MAX,

    error_token_kind      = UINT_LEAST16_MAX - 1,
    whitespace_token_kind = UINT_LEAST16_MAX - 2,
    any_token_kind        = UINT_LEAST16_MAX - 3,

    literal_token_kind  = UINT_LEAST16_MAX - 4,
    position_token_kind = UINT_LEAST16_MAX - 5,
    eof_token_kind      = UINT_LEAST16_MAX - 6,

    identifier_token_kind = UINT_LEAST16_MAX - 7,
    digits_token_kind     = UINT_LEAST16_MAX - 8,

    _smallest_predefined_token_kind = digits_token_kind,
};

template <typename T>
constexpr const char* token_kind_name(const T&) noexcept
{
    return "token";
}
constexpr const char* token_kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";

    case error_token_kind:
        return "error token";
    case whitespace_token_kind:
        return "whitespace";
    case any_token_kind:
        return "any";

    case literal_token_kind:
        return "literal";
    case position_token_kind:
        return "position";
    case eof_token_kind:
        return "EOF";

    case identifier_token_kind:
        return "identifier";
    case digits_token_kind:
        return "digits";
    }

    return ""; // unreachable
}

/// Specialize to define the token kind of a rule.
template <typename TokenRule>
constexpr auto token_kind_of = lexy::unknown_token_kind;

template <typename TokenRule>
constexpr auto token_kind_of<const TokenRule> = token_kind_of<TokenRule>;
} // namespace lexy

//=== production ===//
namespace lexy
{
template <typename Production>
using production_rule = LEXY_DECAY_DECLTYPE(Production::rule);

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

template <typename Production>
using _detect_max_recursion_depth = decltype(Production::max_recursion_depth);

template <typename EntryProduction>
LEXY_CONSTEVAL std::size_t max_recursion_depth()
{
    if constexpr (_detail::is_detected<_detect_max_recursion_depth, EntryProduction>)
        return EntryProduction::max_recursion_depth;
    else
        return 1024; // Arbitrary power of two.
}

template <typename T>
using _enable_production_or_operation = std::enable_if_t<is_production<T> || is_operation<T>>;

struct production_info
{
    const char* const* id;
    const char*        name;
    bool               is_token;
    bool               is_transparent;

    template <typename Production, typename = _enable_production_or_operation<Production>>
    constexpr production_info(Production)
    : id(_detail::type_id<Production>()), name(production_name<Production>()),
      is_token(is_token_production<Production>),
      is_transparent(is_transparent_production<Production>)
    {}

    friend constexpr bool operator==(production_info lhs, production_info rhs)
    {
        return lhs.id == rhs.id;
    }
    friend constexpr bool operator!=(production_info lhs, production_info rhs)
    {
        return !(lhs == rhs);
    }
};
} // namespace lexy

namespace lexy
{
template <typename Production>
using _detect_whitespace = decltype(Production::whitespace);

template <typename Production>
constexpr auto _production_defines_whitespace
    = lexy::_detail::is_detected<_detect_whitespace, Production>;

template <typename Production, typename WhitespaceProduction>
auto _production_whitespace()
{
    if constexpr (_production_defines_whitespace<Production>)
    {
        // We have whitespace defined in the production.
        return Production::whitespace;
    }
    else if constexpr (_production_defines_whitespace<WhitespaceProduction>)
    {
        // We have whitespace defined in the whitespace production.
        return WhitespaceProduction::whitespace;
    }

    // If we didn't have any cases, function returns void.
}
template <typename Production, typename WhitespaceProduction>
using production_whitespace = decltype(_production_whitespace<Production, WhitespaceProduction>());
} // namespace lexy

namespace lexy
{
template <typename To, typename... Args>
inline constexpr auto _is_convertible = false;
template <typename To, typename Arg>
inline constexpr auto _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;
template <>
inline constexpr auto _is_convertible<void> = true;

template <typename ParseState, typename Production>
using _detect_value_of =
    // We're testing a non-const ParseState on purpose, to handle cases where a user forgot to const
    // qualify value_of() (it causes a hard error instead of going to ::value).
    typename decltype(LEXY_DECLVAL(ParseState&).value_of(Production{}))::return_type;

template <typename Production>
using _detect_value = decltype(Production::value);

template <typename Production, typename Sink>
struct _sfinae_sink
{
    Sink _sink;

    using return_type = typename Sink::return_type;

    LEXY_FORCE_INLINE constexpr _sfinae_sink(Production, Sink&& sink) : _sink(LEXY_MOV(sink)) {}

    template <typename... Args>
    LEXY_FORCE_INLINE constexpr void operator()(Args&&... args)
    {
        if constexpr (!is_sink_callback_for<Sink, Args&&...>)
            // We're attempting to call a sink of Production with the given arguments, but no such
            // overload exists.
            static_assert(_detail::error<Production, Args...>,
                          "missing value sink callback overload for production");
        _sink(LEXY_FWD(args)...);
    }

    LEXY_FORCE_INLINE constexpr auto finish() &&
    {
        return LEXY_MOV(_sink).finish();
    }
};

template <typename Production, typename ParseState = void>
constexpr bool production_has_value_callback
    = lexy::_detail::is_detected<_detect_value_of, ParseState, Production>
      || lexy::_detail::is_detected<_detect_value, Production>;

template <typename Production, typename ParseState = void>
class production_value_callback
{
    static constexpr auto _get_value([[maybe_unused]] ParseState* state)
    {
        if constexpr (lexy::_detail::is_detected<_detect_value_of, ParseState, Production>)
            return state->value_of(Production{});
        else
            return Production::value;
    }
    using _type = decltype(_get_value(nullptr));

    static auto _return_type_callback()
    {
        if constexpr (lexy::is_callback<_type>)
            return _get_value(nullptr);
        else if constexpr (lexy::is_sink<_type, std::add_lvalue_reference_t<ParseState>>)
            return _get_value(nullptr).sink(LEXY_DECLVAL(ParseState&));
        else
            return _get_value(nullptr).sink();
    }

public:
    constexpr explicit production_value_callback(ParseState* state) : _state(state) {}

    template <typename State = ParseState, typename = std::enable_if_t<std::is_void_v<State>>>
    constexpr production_value_callback() : _state(nullptr)
    {}
    template <typename State = ParseState,
              typename       = std::enable_if_t<std::is_same_v<State, ParseState>>>
    constexpr explicit production_value_callback(State& state) : _state(&state)
    {}

    using return_type = typename decltype(_return_type_callback())::return_type;

    constexpr auto sink() const
    {
        if constexpr (lexy::is_sink<_type, std::add_lvalue_reference_t<ParseState>>)
        {
            return _sfinae_sink(Production{}, _get_value(_state).sink(*_state));
        }
        else
        {
            // We're attempting to obtain a sink for Production, but none was provided.
            // As Production uses a list rule, it needs a sink.
            static_assert(lexy::is_sink<_type>, "missing value sink for production");
            return _sfinae_sink(Production{}, _get_value(_state).sink());
        }
    }

    template <typename... Args>
    constexpr return_type operator()(Args&&... args) const
    {
        if constexpr (lexy::is_callback_with_state_for<_type, ParseState, Args&&...>
                      && !std::is_void_v<ParseState>)
        {
            return _get_value(_state)[*_state](LEXY_FWD(args)...);
        }
        else if constexpr (lexy::is_callback_for<_type, Args&&...>)
        {
            return _get_value(_state)(LEXY_FWD(args)...);
        }
        else if constexpr ((lexy::is_sink<_type>                                              //
                            || lexy::is_sink<_type, std::add_lvalue_reference_t<ParseState>>) //
                           &&_is_convertible<return_type, Args&&...>)
        {
            // We don't have a matching callback, but it is a single argument that has
            // the correct type already, or we return void and have no arguments.
            // Assume it came from the list sink and return the value without invoking a
            // callback.
            return (LEXY_FWD(args), ...);
        }
        else
        {
            // We're attempting to call the callback of Production with the given arguments, but no
            // such overload exists.
            static_assert(_detail::error<Production, Args...>,
                          "missing value callback overload for production");
        }
    }

private:
    ParseState* _state;
};
} // namespace lexy

#endif // LEXY_GRAMMAR_HPP_INCLUDED
// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_ITERATOR_HPP_INCLUDED
#define LEXY_DETAIL_ITERATOR_HPP_INCLUDED






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

template <typename Iterator, typename Sentinel>
constexpr Iterator next_clamped(Iterator iter, std::size_t n, Sentinel end)
{
    if constexpr (is_random_access_iterator<Iterator> && std::is_same_v<Iterator, Sentinel>)
    {
        auto remaining = std::size_t(end - iter);
        if (remaining < n)
            return end;
        else
            return iter + n;
    }
    else
    {
        for (auto i = 0u; i != n; ++i)
        {
            if (iter == end)
                break;
            ++iter;
        }
        return iter;
    }
}

// Used for assertions.
template <typename Iterator, typename Sentinel>
constexpr bool precedes([[maybe_unused]] Iterator first, [[maybe_unused]] Sentinel after)
{
    if constexpr (is_random_access_iterator<Iterator> && std::is_same_v<Iterator, Sentinel>)
        return first <= after;
    else
        return true;
}

// Requires: begin <= end_a && begin <= end_b.
// Returns min(end_a, end_b).
template <typename Iterator>
constexpr Iterator min_range_end(Iterator begin, Iterator end_a, Iterator end_b)
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

// Requires: begin <= end_a && begin <= end_b.
// Returns max(end_a, end_b).
template <typename Iterator>
constexpr Iterator max_range_end(Iterator begin, Iterator end_a, Iterator end_b)
{
    if constexpr (is_random_access_iterator<Iterator>)
    {
        LEXY_PRECONDITION(begin <= end_a && begin <= end_b);
        if (end_a <= end_b)
            return end_b;
        else
            return end_a;
    }
    else
    {
        auto cur = begin;
        while (true)
        {
            if (cur == end_a)
                return end_b;
            else if (cur == end_b)
                return end_a;

            ++cur;
        }
        return begin; // unreachable
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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

/// An encoding where the input is assumed to be valid UTF-8, but the char type is char.
struct utf8_char_encoding
{
    using char_type = char;
    using int_type  = char;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return std::is_same_v<OtherCharType, LEXY_CHAR8_T>;
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
                      || std::is_same_v<type, utf8_encoding>  //
                      || std::is_same_v<type, utf8_char_encoding>,
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

//=== encoding traits ===//
namespace lexy
{
template <typename Encoding>
constexpr auto is_unicode_encoding
    = std::is_same_v<Encoding, ascii_encoding> || std::is_same_v<Encoding, utf8_encoding>
      || std::is_same_v<Encoding, utf8_char_encoding> || std::is_same_v<Encoding, utf16_encoding>
      || std::is_same_v<Encoding, utf32_encoding>;

template <typename Encoding>
constexpr auto is_text_encoding
    = is_unicode_encoding<Encoding> || std::is_same_v<Encoding, default_encoding>;

template <typename Encoding>
constexpr auto is_byte_encoding = std::is_same_v<Encoding, byte_encoding>;

template <typename Encoding>
constexpr auto is_char_encoding = is_text_encoding<Encoding> || is_byte_encoding<Encoding>;

template <typename Encoding>
constexpr auto is_node_encoding = false;
} // namespace lexy

//=== impls ===//
namespace lexy::_detail
{
template <typename Encoding, typename CharT>
constexpr bool is_compatible_char_type = std::is_same_v<typename Encoding::char_type, CharT>
                                         || Encoding::template is_secondary_char_type<CharT>();

template <typename Encoding, typename CharT>
using require_secondary_char_type
    = std::enable_if_t<Encoding::template is_secondary_char_type<CharT>()>;

template <typename CharT>
constexpr bool is_ascii(CharT c)
{
    if constexpr (std::is_signed_v<CharT>)
        return 0 <= c && c <= 0x7F;
    else
        return c <= 0x7F;
}

template <typename TargetCharT, typename CharT>
LEXY_CONSTEVAL TargetCharT transcode_char(CharT c)
{
    if constexpr (std::is_same_v<CharT, TargetCharT>)
    {
        return c;
    }
#if !LEXY_HAS_CHAR8_T
    else if constexpr (std::is_same_v<CharT, char> && std::is_same_v<TargetCharT, LEXY_CHAR8_T>)
    {
        // If we don't have char8_t, `LEXY_LIT(u8"ä")` would have the type char, not LEXY_CHAR8_T
        // (which is unsigned char). So we disable checking in that case, to allow such usage. Note
        // that this prevents catching `LEXY_LIT("ä")`, but there is nothing we can do.
        return static_cast<LEXY_CHAR8_T>(c);
    }
#endif
    else
    {
        LEXY_ASSERT(is_ascii(c), "character type of string literal didn't match, "
                                 "so only ASCII characters are supported");
        // Note that we don't need to worry about signed/unsigned conversion, all ASCII values are
        // positive.
        return static_cast<TargetCharT>(c);
    }
}

template <typename Encoding, typename CharT>
LEXY_CONSTEVAL auto transcode_int(CharT c) -> typename Encoding::int_type
{
    return Encoding::to_int_type(transcode_char<typename Encoding::char_type>(c));
}
} // namespace lexy::_detail

#endif // LEXY_ENCODING_HPP_INCLUDED


namespace lexy
{
// A generic reader from an iterator range.
template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class _rr
{
public:
    using encoding = Encoding;
    using iterator = Iterator;

    struct marker
    {
        iterator _it;

        constexpr iterator position() const noexcept
        {
            return _it;
        }
    };

    constexpr explicit _rr(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end)
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(begin, end));
    }

    constexpr auto peek() const noexcept
    {
        if (_cur == _end)
            return encoding::eof();
        else
            return encoding::to_int_type(static_cast<typename encoding::char_type>(*_cur));
    }

    constexpr void bump() noexcept
    {
        LEXY_PRECONDITION(_cur != _end);
        ++_cur;
    }

    constexpr iterator position() const noexcept
    {
        return _cur;
    }

    constexpr marker current() const noexcept
    {
        return {_cur};
    }
    constexpr void reset(marker m) noexcept
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(m._it, _end));
        _cur = m._it;
    }

private:
    Iterator                   _cur;
    LEXY_EMPTY_MEMBER Sentinel _end;
};

// A special version where the iterators are pointers.
template <typename Encoding>
LEXY_INSTANTIATION_NEWTYPE(_pr, _rr, Encoding, const typename Encoding::char_type*);

// Aliases for the most common encodings.
LEXY_INSTANTIATION_NEWTYPE(_prd, _pr, lexy::default_encoding);
LEXY_INSTANTIATION_NEWTYPE(_pr8, _pr, lexy::utf8_encoding);
LEXY_INSTANTIATION_NEWTYPE(_prc, _pr, lexy::utf8_char_encoding);
LEXY_INSTANTIATION_NEWTYPE(_prb, _pr, lexy::byte_encoding);

template <typename Encoding, typename Iterator, typename Sentinel>
constexpr auto _range_reader(Iterator begin, Sentinel end)
{
    if constexpr (std::is_pointer_v<Iterator>)
    {
        if constexpr (std::is_same_v<Encoding, lexy::default_encoding>)
            return _prd(begin, end);
        else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
            return _pr8(begin, end);
        else if constexpr (std::is_same_v<Encoding, lexy::utf8_char_encoding>)
            return _prc(begin, end);
        else if constexpr (std::is_same_v<Encoding, lexy::byte_encoding>)
            return _prb(begin, end);
        else
            return _pr<Encoding>(begin, end);
    }
    else
    {
        return _rr<Encoding, Iterator, Sentinel>(begin, end);
    }
}
} // namespace lexy

namespace lexy
{
template <typename Input>
using input_reader = decltype(LEXY_DECLVAL(Input).reader());

template <typename Input>
constexpr bool input_is_view = std::is_trivially_copyable_v<Input>;

template <typename Reader, typename CharT>
constexpr bool char_type_compatible_with_reader
    = (std::is_same_v<CharT, typename Reader::encoding::char_type>)
      || Reader::encoding::template is_secondary_char_type<CharT>();
} // namespace lexy

namespace lexy
{
template <typename Reader>
struct _partial_input
{
    Reader _reader;

    constexpr explicit _partial_input(Reader r) : _reader(r) {}

    constexpr Reader reader() const&
    {
        return _reader;
    }
};

template <typename Reader>
constexpr auto partial_input(const Reader&, typename Reader::iterator begin,
                             typename Reader::iterator end)
{
    return _partial_input(_range_reader<typename Reader::encoding>(begin, end));
}

/// Creates an input that only reads until the given end.
template <typename Reader>
constexpr auto partial_input(const Reader& reader, typename Reader::iterator end)
{
    return partial_input(reader, reader.position(), end);
}
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED


//=== parse_events ===//
namespace lexy::parse_events
{
/// Parsing started.
/// Arguments: position
struct grammar_start
{};
/// Parsing finished succesfully.
/// Arguments: the reader at the final parse position.
struct grammar_finish
{};
/// Parsing finished unsuccesfully.
/// Arguments: the reader at the final parse position.
struct grammar_cancel
{};

/// Start of the current production.
/// Arguments: position
struct production_start
{};
/// End of the current production.
/// Arguments: position
struct production_finish
{};
/// Production is canceled.
/// Arguments: position
struct production_cancel
{};

/// Start of a chain of left-associative operations.
/// Arguments: position
/// Returns: a handle that needs to be passed to finish.
struct operation_chain_start
{};
/// Operation inside a chain.
/// Arguments: operation, position
struct operation_chain_op
{};
/// End of a chain of operations.
/// Arguments: handle, position
struct operation_chain_finish
{};

/// A token was consumed.
/// Arguments: kind, begin, end
struct token
{};

/// The input backtracked from end to begin.
/// Only meaningful for begin != end.
/// Arguments: begin, end
struct backtracked
{};

/// A parse error occurrs.
/// Arguments: error object
struct error
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

// Does not copy token tags.
template <typename Rule>
auto _copy_base_impl()
{
    if constexpr (lexy::is_unconditional_branch_rule<Rule>)
        return unconditional_branch_base{};
    else if constexpr (lexy::is_branch_rule<Rule>)
        return branch_base{};
    else
        return rule_base{};
}
template <typename Rule>
using _copy_base = decltype(_copy_base_impl<Rule>());
} // namespace lexyd

//=== parser ===//
#define LEXY_PARSER_FUNC LEXY_FORCE_INLINE constexpr

namespace lexy
{
template <typename Rule, typename NextParser>
using parser_for = typename Rule::template p<NextParser>;

template <typename BranchRule, typename Reader>
using branch_parser_for = typename BranchRule::template bp<Reader>;

template <typename Production, typename Reader>
struct _pb : lexy::branch_parser_for<lexy::production_rule<Production>, Reader>
{};
// We create a new type here to shorten its name.
template <typename Production, typename Reader>
using production_branch_parser = _pb<Production, Reader>;

/// A branch parser that takes a branch unconditionally and forwards to the regular parser.
template <typename Rule, typename Reader>
struct unconditional_branch_parser
{
    constexpr std::true_type try_parse(const void*, const Reader&)
    {
        return {};
    }

    template <typename Context>
    constexpr void cancel(Context&)
    {}

    template <typename NextParser, typename Context, typename... Args>
    LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
    {
        return parser_for<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
    }
};

/// A branch parser that parses a branch rule but with a special continuation.
template <typename BranchRule, typename Reader, template <typename> typename Continuation>
struct continuation_branch_parser
{
    branch_parser_for<BranchRule, Reader> impl;

    template <typename ControlBlock>
    constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
    {
        return impl.try_parse(cb, reader);
    }

    template <typename Context>
    constexpr void cancel(Context& context)
    {
        impl.cancel(context);
    }

    template <typename NextParser, typename Context, typename... Args>
    LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
    {
        return impl.template finish<Continuation<NextParser>>(context, reader, LEXY_FWD(args)...);
    }
};

/// A parser that does not support any arguments.
template <typename... PrevArgs>
struct pattern_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static std::true_type parse(Context&, Reader&, const PrevArgs&..., Args&&...)
    {
        // A rule is used inside a loop or similar situation, where it must not produce values, but
        // it did.
        static_assert(sizeof...(Args) == 0, "pattern rule must not produce any values");
        return {};
    }
};

/// A parser that forwards all arguments to a sink, which is the first argument.
struct sink_parser
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_PARSER_FUNC static std::true_type parse(Context&, Reader&, Sink& sink, Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0)
            sink(LEXY_FWD(args)...);

        return {};
    }
};

/// A parser that finishes a sink and continues with the next one.
template <typename NextParser>
struct sink_finish_parser
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_PARSER_FUNC static auto parse(Context& context, Reader& reader, Sink& sink, Args&&... args)
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., LEXY_MOV(sink).finish());
        }
    }
};
} // namespace lexy

//=== whitespace ===//
namespace lexy::_detail
{
template <typename NextParser>
struct automatic_ws_parser;
} // namespace lexy::_detail

namespace lexy
{
template <typename Context, typename NextParser,
          typename = lexy::production_whitespace<typename Context::production,
                                                 typename Context::whitespace_production>>
struct whitespace_parser : _detail::automatic_ws_parser<NextParser>
{};
// If we know the whitespace rule is void, go to NextParser immediately.
// This is both an optimization and also doesn't require inclusion of whitespace.hpp.
template <typename Context, typename NextParser>
struct whitespace_parser<Context, NextParser, void> : NextParser
{};
} // namespace lexy

//=== token parser ===//
namespace lexy
{
template <typename TokenRule, typename Reader>
using token_parser_for = typename TokenRule::template tp<Reader>;

template <typename TokenRule, typename Reader>
LEXY_FORCE_INLINE constexpr auto try_match_token(TokenRule, Reader& reader)
{
    lexy::token_parser_for<TokenRule, Reader> parser(reader);

    using try_parse_result = decltype(parser.try_parse(reader));
    if constexpr (std::is_same_v<try_parse_result, std::true_type>)
    {
        parser.try_parse(reader);
        reader.reset(parser.end);
        return std::true_type{};
    }
    else if constexpr (std::is_same_v<try_parse_result, std::false_type>)
    {
        // try_parse() is pure and we don't want to advance the reader, so no need to call it.
        return std::false_type{};
    }
    else
    {
        if (!parser.try_parse(reader))
            return false;

        reader.reset(parser.end);
        return true;
    }
}
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED



//=== parse_context ===//
namespace lexy
{
namespace _detail
{
    struct parse_context_var_base
    {
        const void*             id;
        parse_context_var_base* next;

        constexpr parse_context_var_base(const void* id) : id(id), next(nullptr) {}

        template <typename Context>
        constexpr void link(Context& context)
        {
            auto cb  = context.control_block;
            next     = cb->vars;
            cb->vars = this;
        }

        template <typename Context>
        constexpr void unlink(Context& context)
        {
            auto cb  = context.control_block;
            cb->vars = next;
        }
    };

    template <typename Id, typename T>
    struct parse_context_var : parse_context_var_base
    {
        static constexpr auto type_id = lexy::_detail::type_id<Id>();

        T value;

        explicit constexpr parse_context_var(T&& value)
        : parse_context_var_base(static_cast<const void*>(&type_id) /* NOLINT */),
          value(LEXY_MOV(value))
        {}

        template <typename ControlBlock>
        static constexpr T& get(const ControlBlock* cb)
        {
            for (auto cur = cb->vars; cur; cur = cur->next)
                if (cur->id == static_cast<const void*>(&type_id) /* NOLINT */)
                    return static_cast<parse_context_var*>(cur)->value;

            LEXY_ASSERT(false, "context variable hasn't been created");
            return static_cast<parse_context_var*>(cb->vars)->value;
        }
    };

    template <typename Handler, typename State = void>
    struct parse_context_control_block
    {
        using handler_type = Handler;
        using state_type   = State;

        LEXY_EMPTY_MEMBER Handler parse_handler;
        State*                    parse_state;

        parse_context_var_base* vars;

        int  cur_depth, max_depth;
        bool enable_whitespace_skipping;

        constexpr parse_context_control_block(Handler&& handler, State* state,
                                              std::size_t max_depth)
        : parse_handler(LEXY_MOV(handler)), parse_state(state), //
          vars(nullptr),                                        //
          cur_depth(0), max_depth(static_cast<int>(max_depth)), enable_whitespace_skipping(true)
        {}

        template <typename OtherHandler>
        constexpr parse_context_control_block(Handler&& handler,
                                              parse_context_control_block<OtherHandler, State>* cb)
        : parse_handler(LEXY_MOV(handler)), parse_state(cb->parse_state), //
          vars(cb->vars), cur_depth(cb->cur_depth), max_depth(cb->max_depth),
          enable_whitespace_skipping(cb->enable_whitespace_skipping)
        {}

        template <typename OtherHandler>
        constexpr void copy_vars_from(parse_context_control_block<OtherHandler, State>* cb)
        {
            vars                       = cb->vars;
            cur_depth                  = cb->cur_depth;
            max_depth                  = cb->max_depth;
            enable_whitespace_skipping = cb->enable_whitespace_skipping;
        }
    };
} // namespace _detail

// If a production doesn't define whitespace, we don't need to pass it and can shorten the template
// name.
template <typename Production>
using _whitespace_production_of
    = std::conditional_t<_production_defines_whitespace<Production>, Production, void>;

template <typename Handler, typename State, typename Production>
using _production_value_type =
    typename Handler::template value_callback<Production, State>::return_type;

template <typename Handler, typename State, typename Production,
          typename WhitespaceProduction = _whitespace_production_of<Production>>
struct _pc
{
    using handler_type = Handler;
    using state_type   = State;

    using production            = Production;
    using whitespace_production = WhitespaceProduction;
    using value_type            = _production_value_type<Handler, State, Production>;

    typename Handler::event_handler                       handler;
    _detail::parse_context_control_block<Handler, State>* control_block;
    _detail::lazy_init<value_type>                        value;

    constexpr explicit _pc(_detail::parse_context_control_block<Handler, State>* cb)
    : handler(Production{}), control_block(cb)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction)
    {
        // Update the whitespace production if necessary.
        // If the current production is a token or defines whitespace,
        // we change it to the current production (or void), otherwise keep it.
        using new_whitespace_production
            = std::conditional_t<is_token_production<ChildProduction> //
                                     || _production_defines_whitespace<ChildProduction>,
                                 _whitespace_production_of<ChildProduction>, WhitespaceProduction>;
        return _pc<Handler, State, ChildProduction, new_whitespace_production>(control_block);
    }

    constexpr auto value_callback()
    {
        using callback = typename Handler::template value_callback<Production, State>;
        return callback(control_block->parse_state);
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
    {
        return handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};
} // namespace lexy

//=== do_action ===//
namespace lexy::_detail
{
struct final_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader&, Args&&... args)
    {
        context.value.emplace_result(context.value_callback(), LEXY_FWD(args)...);
        return true;
    }
};

template <typename NextParser>
struct context_finish_parser
{
    template <typename Context, typename Reader, typename SubContext, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, SubContext& sub_context,
                                       Args&&... args)
    {
        // Might need to skip whitespace, according to the original context.
        using continuation
            = std::conditional_t<lexy::is_token_production<typename SubContext::production>,
                                 lexy::whitespace_parser<Context, NextParser>, NextParser>;

        // Pass the produced value to the next parser.
        if constexpr (std::is_void_v<typename SubContext::value_type>)
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        else
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       LEXY_MOV(*sub_context.value));
    }
};
} // namespace lexy::_detail

namespace lexy
{
constexpr void* no_parse_state = nullptr;

template <typename Handler, typename State, typename Production, typename Reader>
constexpr auto _do_action(_pc<Handler, State, Production>& context, Reader& reader)
{
    context.on(parse_events::grammar_start{}, reader.position());
    context.on(parse_events::production_start{}, reader.position());

    // We parse whitespace, theen the rule, then finish.
    using parser = lexy::whitespace_parser<
        LEXY_DECAY_DECLTYPE(context),
        lexy::parser_for<lexy::production_rule<Production>, _detail::final_parser>>;
    auto rule_result = parser::parse(context, reader);

    if (rule_result)
    {
        context.on(parse_events::production_finish{}, reader.position());
        context.on(parse_events::grammar_finish{}, reader);
    }
    else
    {
        context.on(parse_events::production_cancel{}, reader.position());
        context.on(parse_events::grammar_cancel{}, reader);
    }

    return rule_result;
}

template <typename Production, template <typename> typename Result, typename Handler,
          typename State, typename Reader>
constexpr auto do_action(Handler&& handler, State* state, Reader& reader)
{
    static_assert(!std::is_reference_v<Handler>, "need to move handler in");

    _detail::parse_context_control_block control_block(LEXY_MOV(handler), state,
                                                       max_recursion_depth<Production>());
    _pc<Handler, State, Production>      context(&control_block);

    auto rule_result = _do_action(context, reader);

    using value_type = typename decltype(context)::value_type;
    if constexpr (std::is_void_v<value_type>)
        return LEXY_MOV(control_block.parse_handler).template get_result<Result<void>>(rule_result);
    else if (context.value)
        return LEXY_MOV(control_block.parse_handler)
            .template get_result<Result<value_type>>(rule_result, LEXY_MOV(*context.value));
    else
        return LEXY_MOV(control_block.parse_handler)
            .template get_result<Result<value_type>>(rule_result);
}
} // namespace lexy

//=== value callback ===//
namespace lexy::_detail
{
struct void_value_callback
{
    constexpr void_value_callback() = default;
    template <typename State>
    constexpr explicit void_value_callback(State*)
    {}

    using return_type = void;

    constexpr auto sink() const
    {
        return lexy::noop.sink();
    }

    template <typename... Args>
    constexpr void operator()(Args&&...) const
    {}
};
} // namespace lexy::_detail

#endif // LEXY_ACTION_BASE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_VALIDATE_HPP_INCLUDED
#define LEXY_ACTION_VALIDATE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_ANY_REF_HPP_INCLUDED
#define LEXY_DETAIL_ANY_REF_HPP_INCLUDED



// Essentially a void*, but we can cast it in a constexpr context.
// The cost is an extra layer of indirection.

namespace lexy::_detail
{
template <typename T>
class any_holder;

// Store a pointer to this instead of a void*.
class any_base
{
public:
    any_base(const any_base&)            = delete;
    any_base& operator=(const any_base&) = delete;

    template <typename T>
    constexpr T& get() noexcept
    {
        return static_cast<any_holder<T>*>(this)->get();
    }
    template <typename T>
    constexpr const T& get() const noexcept
    {
        return static_cast<const any_holder<T>*>(this)->get();
    }

private:
    constexpr any_base() = default;
    ~any_base()          = default;

    template <typename T>
    friend class any_holder;
};

using any_ref  = any_base*;
using any_cref = const any_base*;

// Need to store the object in here.
template <typename T>
class any_holder : public any_base
{
public:
    constexpr explicit any_holder(T&& obj) : _obj(LEXY_MOV(obj)) {}

    constexpr T& get() noexcept
    {
        return _obj;
    }
    constexpr const T& get() const noexcept
    {
        return _obj;
    }

private:
    T _obj;
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ANY_REF_HPP_INCLUDED




// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
using _detect_append = decltype(LEXY_DECLVAL(Container&).append(LEXY_DECLVAL(Container&&)));
template <typename Container>
constexpr auto _has_append = _detail::is_detected<_detect_append, Container>;
} // namespace lexy

//=== as_list ===//
namespace lexy
{
template <typename Container>
struct _list_sink
{
    Container _result;

    using return_type = Container;

    template <typename C = Container, typename U>
    constexpr auto operator()(U&& obj) -> decltype(LEXY_DECLVAL(C&).push_back(LEXY_FWD(obj)))
    {
        return _result.push_back(LEXY_FWD(obj));
    }

    template <typename C = Container, typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(LEXY_DECLVAL(C&).emplace_back(LEXY_FWD(args)...))
    {
        return _result.emplace_back(LEXY_FWD(args)...);
    }

    constexpr Container&& finish() &&
    {
        return LEXY_MOV(_result);
    }
};

template <typename Container, typename AllocFn>
struct _list_alloc
{
    AllocFn _alloc;

    using return_type = Container;

    template <typename State>
    struct _with_state
    {
        State&         _state;
        const AllocFn& _alloc;

        constexpr Container operator()(Container&& container) const
        {
            return LEXY_MOV(container);
        }
        constexpr Container operator()(nullopt&&) const
        {
            return Container(_detail::invoke(_alloc, _state));
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const
            -> LEXY_DECAY_DECLTYPE((LEXY_DECLVAL(Container&).push_back(LEXY_FWD(args)), ...),
                                   LEXY_DECLVAL(Container))
        {
            Container result(_detail::invoke(_alloc, _state));
            if constexpr (_has_reserve<Container>)
                result.reserve(sizeof...(args));
            (result.emplace_back(LEXY_FWD(args)), ...);
            return result;
        }
    };

    template <typename State>
    constexpr auto operator[](State& state) const
    {
        return _with_state<State>{state, _alloc};
    }

    template <typename State>
    constexpr auto sink(State& state) const
    {
        return _list_sink<Container>{Container(_detail::invoke(_alloc, state))};
    }
};

template <typename Container>
struct _list
{
    using return_type = Container;

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
        -> LEXY_DECAY_DECLTYPE((LEXY_DECLVAL(Container&).push_back(LEXY_FWD(args)), ...),
                               LEXY_DECLVAL(Container))
    {
        Container result;
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));
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
        (result.emplace_back(LEXY_FWD(args)), ...);
        return result;
    }

    constexpr auto sink() const
    {
        return _list_sink<Container>{Container()};
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _list_sink<Container>{Container(allocator)};
    }

    template <typename AllocFn>
    constexpr auto allocator(AllocFn alloc_fn) const
    {
        return _list_alloc<Container, AllocFn>{alloc_fn};
    }
    constexpr auto allocator() const
    {
        return allocator([](const auto& alloc) { return alloc; });
    }
};

/// A callback with sink that creates a list of things (e.g. a `std::vector`, `std::list`, etc.).
/// It repeatedly calls `push_back()` and `emplace_back()`.
template <typename Container>
constexpr auto as_list = _list<Container>{};
} // namespace lexy

//=== as_collection ===//
namespace lexy
{
template <typename Container>
struct _collection_sink
{
    Container _result;

    using return_type = Container;

    template <typename C = Container, typename U>
    constexpr auto operator()(U&& obj) -> decltype(LEXY_DECLVAL(C&).insert(LEXY_FWD(obj)))
    {
        return _result.insert(LEXY_FWD(obj));
    }

    template <typename C = Container, typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(LEXY_DECLVAL(C&).emplace(LEXY_FWD(args)...))
    {
        return _result.emplace(LEXY_FWD(args)...);
    }

    constexpr Container&& finish() &&
    {
        return LEXY_MOV(_result);
    }
};

template <typename Container, typename AllocFn>
struct _collection_alloc
{
    AllocFn _alloc;

    using return_type = Container;

    template <typename State>
    struct _with_state
    {
        State&         _state;
        const AllocFn& _alloc;

        constexpr Container operator()(Container&& container) const
        {
            return LEXY_MOV(container);
        }
        constexpr Container operator()(nullopt&&) const
        {
            return Container(_detail::invoke(_alloc, _state));
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const
            -> LEXY_DECAY_DECLTYPE((LEXY_DECLVAL(Container&).insert(LEXY_FWD(args)), ...),
                                   LEXY_DECLVAL(Container))
        {
            Container result(_detail::invoke(_alloc, _state));
            if constexpr (_has_reserve<Container>)
                result.reserve(sizeof...(args));
            (result.emplace(LEXY_FWD(args)), ...);
            return result;
        }
    };

    template <typename State>
    constexpr auto operator[](State& state) const
    {
        return _with_state<State>{state, _alloc};
    }

    template <typename State>
    constexpr auto sink(State& state) const
    {
        return _collection_sink<Container>{Container(_detail::invoke(_alloc, state))};
    }
};

template <typename Container>
struct _collection
{
    using return_type = Container;

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
        -> LEXY_DECAY_DECLTYPE((LEXY_DECLVAL(Container&).insert(LEXY_FWD(args)), ...),
                               LEXY_DECLVAL(Container))
    {
        Container result;
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));
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
        (result.emplace(LEXY_FWD(args)), ...);
        return result;
    }

    constexpr auto sink() const
    {
        return _collection_sink<Container>{Container()};
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _collection_sink<Container>{Container(allocator)};
    }

    template <typename AllocFn>
    constexpr auto allocator(AllocFn alloc_fn) const
    {
        return _collection_alloc<Container, AllocFn>{alloc_fn};
    }
    constexpr auto allocator() const
    {
        return allocator([](const auto& alloc) { return alloc; });
    }
};

/// A callback with sink that creates an unordered collection of things (e.g. a `std::set`,
/// `std::unordered_map`, etc.). It repeatedly calls `insert()` and `emplace()`.
template <typename T>
constexpr auto as_collection = _collection<T>{};
} // namespace lexy

//=== concat ===//
namespace lexy
{
template <typename Container>
struct _concat
{
    using return_type = Container;

    constexpr Container operator()(nullopt&&) const
    {
        return Container();
    }

    template <typename... Tail>
    constexpr Container _call(Container&& head, Tail&&... tail) const
    {
        if constexpr (sizeof...(Tail) == 0)
            return LEXY_MOV(head);
        else
        {
            if constexpr (_has_reserve<Container>)
            {
                auto total_size = (head.size() + ... + tail.size());
                head.reserve(total_size);
            }

            auto append = [&head](Container&& container) {
                if constexpr (_has_append<Container>)
                {
                    head.append(LEXY_MOV(container));
                }
                else
                {
                    for (auto& elem : container)
                        head.push_back(LEXY_MOV(elem));
                }
            };
            (append(LEXY_MOV(tail)), ...);

            return LEXY_MOV(head);
        }
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_call(Container(LEXY_FWD(args))...))
    {
        return _call(Container(LEXY_FWD(args))...);
    }

    struct _sink
    {
        Container _result;

        using return_type = Container;

        constexpr void operator()(Container&& container)
        {
            if (_result.empty())
            {
                // We assign until we have items.
                // That way we get the existing allocator.
                _result = LEXY_MOV(container);
            }
            else if constexpr (_has_append<Container>)
            {
                _result.append(LEXY_MOV(container));
            }
            else
            {
                if constexpr (_has_reserve<Container>)
                {
                    auto capacity   = _result.capacity();
                    auto total_size = _result.size() + container.size();
                    if (total_size > capacity)
                    {
                        // If we need more space we reserve at least twice as much.
                        auto exp_capacity = 2 * capacity;
                        if (total_size > exp_capacity)
                            _result.reserve(total_size);
                        else
                            _result.reserve(exp_capacity);
                    }
                }

                for (auto& elem : container)
                    _result.push_back(LEXY_MOV(elem));
            }
        }

        constexpr Container&& finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

template <typename Container>
constexpr auto concat = _concat<Container>{};
} // namespace lexy

//=== collect ===//
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


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ERROR_HPP_INCLUDED
#define LEXY_ERROR_HPP_INCLUDED





namespace lexy
{
template <typename Reader, typename Tag>
class error;

/// Type erased generic failure.
template <typename Reader>
class error<Reader, void>
{
public:
    constexpr explicit error(typename Reader::iterator pos, const char* msg) noexcept
    : _pos(pos), _end(pos), _msg(msg)
    {}
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const char* msg) noexcept
    : _pos(begin), _end(end), _msg(msg)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, void>() const noexcept
    {
        return error<OtherReader, void>(_pos, _end, _msg);
    }

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* message() const noexcept
    {
        return _msg;
    }
    template <typename Tag>
    constexpr bool is(Tag = {}) const noexcept
    {
        return _detail::string_view(_msg) == _detail::type_name<Tag>();
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
    const char*               _msg;
};

/// Generic failure.
template <typename Reader, typename Tag>
class error : public error<Reader, void>
{
public:
    constexpr explicit error(typename Reader::iterator pos) noexcept
    : error<Reader, void>(pos, _detail::type_name<Tag>())
    {}
    constexpr explicit error(typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : error<Reader, void>(begin, end, _detail::type_name<Tag>())
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, Tag>() const noexcept
    {
        return error<OtherReader, Tag>(this->begin(), this->end());
    }
};

/// Expected the literal character sequence.
struct expected_literal
{};
template <typename Reader>
class error<Reader, expected_literal>
{
public:
    constexpr explicit error(typename Reader::iterator                   pos,
                             const typename Reader::encoding::char_type* str, std::size_t index,
                             std::size_t length) noexcept
    : _pos(pos), _str(str), _idx(index), _length(length)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, expected_literal>() const noexcept
    {
        return error<OtherReader, expected_literal>(_pos, _str, _idx, _length);
    }

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

    constexpr std::size_t index() const noexcept
    {
        return _idx;
    }

    constexpr std::size_t length() const noexcept
    {
        return _length;
    }

    constexpr auto character() const noexcept
    {
        return _str[_idx];
    }

private:
    typename Reader::iterator                   _pos;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _idx, _length;
};

/// Expected the given keyword.
/// Unlike expected_literal, this one looks at the following characters as well.
struct expected_keyword
{};
template <typename Reader>
class error<Reader, expected_keyword>
{
public:
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const typename Reader::encoding::char_type* str, std::size_t length)
    : _begin(begin), _end(end), _str(str), _length(length)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, expected_keyword>() const noexcept
    {
        return error<OtherReader, expected_keyword>(_begin, _end, _str, _length);
    }

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

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

    constexpr std::size_t length() const noexcept
    {
        return _length;
    }

private:
    typename Reader::iterator                   _begin;
    typename Reader::iterator                   _end;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _length;
};

/// Expected a character of the specified character class.
struct expected_char_class
{};
template <typename Reader>
class error<Reader, expected_char_class>
{
public:
    constexpr explicit error(typename Reader::iterator pos, const char* name) noexcept
    : _pos(pos), _name(name)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, expected_char_class>() const noexcept
    {
        return error<OtherReader, expected_char_class>(_pos, _name);
    }

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* name() const noexcept
    {
        return _name;
    }

private:
    typename Reader::iterator _pos;
    const char*               _name;
};

template <typename Input, typename Tag>
using error_for = error<input_reader<Input>, Tag>;
} // namespace lexy

namespace lexy
{
template <typename Input>
using _detect_parent_input = decltype(LEXY_DECLVAL(Input).parent_input());

/// Contains information about the context of an error, production is type-erased.
template <typename Input>
class error_context
{
public:
    constexpr explicit error_context(lexy::production_info production, const Input& input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : _input(&input), _pos(pos), _production(production.name)
    {}

    /// The input.
    constexpr const auto& input() const noexcept
    {
        if constexpr (_detail::is_detected<_detect_parent_input, Input>)
            return _input->parent_input();
        else
            return *_input;
    }

    /// The name of the production where the error occurred.
    const char* production() const noexcept
    {
        return _production;
    }

    /// The starting position of the production.
    constexpr auto position() const noexcept
    {
        return _pos;
    }

private:
    const Input*                           _input;
    typename input_reader<Input>::iterator _pos;
    const char*                            _production;
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

    template <typename Reader>
    friend class _vh;
};
} // namespace lexy

namespace lexy
{
template <typename Reader>
struct _validate_callbacks
{
    _detail::any_ref  sink;
    _detail::any_cref input;

    void (*generic)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator begin, const error<Reader, void>& error);
    void (*literal)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator begin, const error<Reader, expected_literal>& error);
    void (*keyword)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator begin, const error<Reader, expected_keyword>& error);
    void (*char_class)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                       typename Reader::iterator                 begin,
                       const error<Reader, expected_char_class>& error);

    template <typename Input, typename Sink>
    constexpr _validate_callbacks(const _detail::any_holder<const Input*>& input,
                                  _detail::any_holder<Sink>&               sink)
    : sink(&sink), input(&input),
      generic([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                 typename Reader::iterator begin, const error<Reader, void>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      }),
      literal([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                 typename Reader::iterator begin, const error<Reader, expected_literal>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      }),
      keyword([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                 typename Reader::iterator begin, const error<Reader, expected_keyword>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      }),
      char_class([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator                 begin,
                    const error<Reader, expected_char_class>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      })
    {}
};

template <typename Reader>
class _vh
{
public:
    template <typename Input, typename Sink>
    constexpr explicit _vh(const _detail::any_holder<const Input*>& input,
                           _detail::any_holder<Sink>&               sink)
    : _cb(input, sink)
    {}

    class event_handler
    {
        using iterator = typename Reader::iterator;

    public:
        constexpr event_handler(production_info info) : _begin(), _info(info) {}

        constexpr void on(_vh& handler, parse_events::production_start, iterator pos)
        {
            _begin = pos;

            _prev        = handler._top;
            handler._top = this;
        }
        constexpr void on(_vh& handler, parse_events::production_finish, iterator)
        {
            handler._top = _prev;
        }
        constexpr void on(_vh& handler, parse_events::production_cancel, iterator)
        {
            handler._top = _prev;
        }

        template <typename R, typename Tag>
        constexpr void on(_vh& handler, parse_events::error, const error<R, Tag>& error)
        {
            handler._cb.generic(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh& handler, parse_events::error, const error<R, void>& error)
        {
            handler._cb.generic(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh&                              handler, parse_events::error,
                          const error<R, expected_literal>& error)
        {
            handler._cb.literal(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh&                              handler, parse_events::error,
                          const error<R, expected_keyword>& error)
        {
            handler._cb.keyword(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh&                                 handler, parse_events::error,
                          const error<R, expected_char_class>& error)
        {
            handler._cb.char_class(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }

        template <typename Event, typename... Args>
        constexpr auto on(_vh&, Event, const Args&...)
        {
            return 0; // operation_chain_start must return something
        }

        constexpr iterator production_begin() const
        {
            return _begin;
        }

        constexpr production_info get_info() const
        {
            auto cur = this;
            while (cur->_info.is_transparent && cur->_prev != nullptr)
                cur = cur->_prev;
            return cur->_info;
        }

    private:
        iterator        _begin;
        production_info _info;
        event_handler*  _prev = nullptr;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename Result>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        using sink_t = _error_sink_t<typename Result::error_callback>;
        return Result(rule_parse_result, LEXY_MOV(_cb.sink->template get<sink_t>()).finish());
    }

private:
    _validate_callbacks<Reader> _cb;
    event_handler*              _top = nullptr;
};

template <typename State, typename Input, typename ErrorCallback>
struct validate_action
{
    const ErrorCallback* _callback;
    State*               _state = nullptr;

    using handler = _vh<lexy::input_reader<Input>>;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = validate_result<ErrorCallback>;

    constexpr explicit validate_action(const ErrorCallback& callback) : _callback(&callback) {}
    template <typename U = State>
    constexpr explicit validate_action(U& state, const ErrorCallback& callback)
    : _callback(&callback), _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        _detail::any_holder input_holder(&input);
        _detail::any_holder sink(_get_error_sink(*_callback));
        auto                reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(input_holder, sink), _state,
                                                        reader);
    }
};

template <typename Production, typename Input, typename ErrorCallback>
constexpr auto validate(const Input&         input,
                        const ErrorCallback& callback) -> validate_result<ErrorCallback>
{
    return validate_action<void, Input, ErrorCallback>(callback)(Production{}, input);
}

template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto validate(const Input& input, State& state,
                        const ErrorCallback& callback) -> validate_result<ErrorCallback>
{
    return validate_action<State, Input, ErrorCallback>(state, callback)(Production{}, input);
}
template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto validate(const Input& input, const State& state,
                        const ErrorCallback& callback) -> validate_result<ErrorCallback>
{
    return validate_action<const State, Input, ErrorCallback>(state, callback)(Production{}, input);
}
} // namespace lexy

#endif // LEXY_ACTION_VALIDATE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ANY_HPP_INCLUDED
#define LEXY_DSL_ANY_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_SWAR_HPP_INCLUDED
#define LEXY_DETAIL_SWAR_HPP_INCLUDED

#include <climits>
#include <cstdint>
#include <cstring>



#if defined(_MSC_VER)
#    include <intrin.h>
#endif

namespace lexy::_detail
{
// Contains the chars in little endian order; rightmost bits are first char.
using swar_int = std::uintmax_t;

// The number of chars that can fit into one SWAR.
template <typename CharT>
constexpr auto swar_length = [] {
    static_assert(sizeof(CharT) < sizeof(swar_int) && sizeof(swar_int) % sizeof(CharT) == 0);
    return sizeof(swar_int) / sizeof(CharT);
}();

template <typename CharT>
constexpr auto char_bit_size = sizeof(CharT) * CHAR_BIT;

template <typename CharT>
constexpr auto make_uchar(CharT c)
{
    if constexpr (std::is_same_v<CharT, LEXY_CHAR8_T>)
        // Not all libstdc++ support char8_t and std::make_unsigned_t.
        return c;
    else
        return std::make_unsigned_t<CharT>(c);
}
template <typename CharT>
using uchar_t = decltype(make_uchar(CharT()));

// Returns a swar_int filled with the specific char.
template <typename CharT>
constexpr swar_int swar_fill(CharT _c)
{
    auto c = make_uchar(_c);

    auto result = swar_int(0);
    for (auto i = 0u; i != swar_length<CharT>; ++i)
    {
        result <<= char_bit_size<CharT>;
        result |= c;
    }
    return result;
}

// Returns a swar_int filled with the complement of the specific char.
template <typename CharT>
constexpr swar_int swar_fill_compl(CharT _c)
{
    auto c = uchar_t<CharT>(~uchar_t<CharT>(_c));

    auto result = swar_int(0);
    for (auto i = 0u; i != swar_length<CharT>; ++i)
    {
        result <<= char_bit_size<CharT>;
        result |= c;
    }
    return result;
}

constexpr void _swar_pack(swar_int&, int) {}
template <typename H, typename... T>
constexpr void _swar_pack(swar_int& result, int index, H h, T... t)
{
    if (std::size_t(index) == char_bit_size<swar_int>)
        return;

    if (index >= 0)
        result |= swar_int(make_uchar(h)) << index;

    _swar_pack(result, index + int(char_bit_size<H>), t...);
}

template <typename CharT>
struct swar_pack_result
{
    swar_int    value;
    swar_int    mask;
    std::size_t count;

    constexpr CharT operator[](std::size_t idx) const
    {
        constexpr auto mask = (swar_int(1) << char_bit_size<CharT>)-1;
        return (value >> idx * char_bit_size<CharT>)&mask;
    }
};

// Returns a swar_int containing the specified characters.
// If more are provided than fit, will only take the first couple ones.
template <int SkipFirstNChars = 0, typename... CharT>
constexpr auto swar_pack(CharT... cs)
{
    using char_type = std::common_type_t<CharT...>;
    swar_pack_result<char_type> result{0, 0, 0};

    _swar_pack(result.value, -SkipFirstNChars * int(char_bit_size<char_type>), cs...);

    auto count = int(sizeof...(CharT)) - SkipFirstNChars;
    if (count <= 0)
    {
        result.mask  = 0;
        result.count = 0;
    }
    else if (count >= int(swar_length<char_type>))
    {
        result.mask  = swar_int(-1);
        result.count = swar_length<char_type>;
    }
    else
    {
        result.mask  = swar_int(swar_int(1) << count * int(char_bit_size<char_type>)) - 1;
        result.count = std::size_t(count);
    }

    return result;
}

// Returns the index of the char that is different between lhs and rhs.
template <typename CharT>
constexpr std::size_t swar_find_difference(swar_int lhs, swar_int rhs)
{
    if (lhs == rhs)
        return swar_length<CharT>;

    auto mask = lhs ^ rhs;

#if defined(__GNUC__)
    auto bit_idx = __builtin_ctzll(mask);
#elif defined(_MSC_VER) && defined(_WIN64)
    unsigned long bit_idx;
    _BitScanForward64(&bit_idx, mask);
#elif defined(_MSC_VER)
    unsigned long bit_idx = 0;
    if (!_BitScanForward(&bit_idx, static_cast<std::uint32_t>(mask))
        && _BitScanForward(&bit_idx, mask >> 32))
        bit_idx += 32;
#else
#    error "unsupported compiler; please file an issue"
#endif

    return std::size_t(bit_idx) / char_bit_size<CharT>;
}

// Returns true if v has a char less than N.
template <typename CharT, CharT N>
constexpr bool swar_has_char_less(swar_int v)
{
    // https://graphics.stanford.edu/~seander/bithacks.html#HasLessInWord

    constexpr auto offset      = swar_fill(CharT(N));
    auto           zero_or_msb = v - offset;

    constexpr auto msb_mask = swar_fill(CharT(1 << (char_bit_size<CharT> - 1)));
    auto           not_msb  = ~v & msb_mask;

    return zero_or_msb & not_msb;
}

// Returns true if v has a zero char.
template <typename CharT>
constexpr bool swar_has_zero(swar_int v)
{
    return swar_has_char_less<CharT, 1>(v);
}

// Returns true if v contains the specified char.
template <typename CharT, CharT C>
constexpr bool swar_has_char(swar_int v)
{
    if constexpr (C == 0)
    {
        return swar_has_zero<CharT>(v);
    }
    else
    {
        constexpr auto mask = swar_fill(C);
        return swar_has_zero<CharT>(v ^ mask);
    }
}
} // namespace lexy::_detail

namespace lexy::_detail
{
struct _swar_base
{};
template <typename Reader>
constexpr auto is_swar_reader = std::is_base_of_v<_swar_base, Reader>;

template <typename Derived>
class swar_reader_base : _swar_base
{
public:
    swar_int peek_swar() const
    {
        auto ptr = static_cast<const Derived&>(*this).position();

        swar_int result;
#if LEXY_IS_LITTLE_ENDIAN
        std::memcpy(&result, ptr, sizeof(swar_int));
#else
        using char_type = typename Derived::encoding::char_type;
        auto dst        = reinterpret_cast<char*>(&result);
        auto length     = sizeof(swar_int) / sizeof(char_type);
        for (auto i = 0u; i != length; ++i)
        {
            std::memcpy(dst + i, ptr + length - i - 1, sizeof(char_type));
        }
#endif
        return result;
    }

    void bump_swar()
    {
        auto ptr = static_cast<Derived&>(*this).position();
        ptr += swar_length<typename Derived::encoding::char_type>;
        static_cast<Derived&>(*this).reset({ptr});
    }
    void bump_swar(std::size_t char_count)
    {
        auto ptr = static_cast<Derived&>(*this).position();
        ptr += char_count;
        static_cast<Derived&>(*this).reset({ptr});
    }
};

constexpr std::size_t round_size_for_swar(std::size_t size_in_bytes)
{
    // We round up to the next multiple.
    if (auto remainder = size_in_bytes % sizeof(swar_int); remainder > 0)
        size_in_bytes += sizeof(swar_int) - remainder;
    // Then add one extra space of padding on top.
    size_in_bytes += sizeof(swar_int);
    return size_in_bytes;
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_SWAR_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_MATCH_HPP_INCLUDED
#define LEXY_ACTION_MATCH_HPP_INCLUDED



namespace lexy
{
class _mh
{
public:
    constexpr _mh() : _failed(false) {}

    class event_handler
    {
    public:
        constexpr event_handler(production_info) {}

        template <typename Error>
        constexpr void on(_mh& handler, parse_events::error, Error&&)
        {
            handler._failed = true;
        }

        template <typename Event, typename... Args>
        constexpr int on(_mh&, Event, const Args&...)
        {
            return 0; // operation_chain_start needs to return something
        }
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename>
    constexpr bool get_result(bool rule_parse_result) &&
    {
        return rule_parse_result && !_failed;
    }

private:
    bool _failed;
};

template <typename State, typename Input>
struct match_action
{
    State* _state = nullptr;

    using handler = _mh;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = bool;

    constexpr match_action() = default;
    template <typename U = State>
    constexpr explicit match_action(U& state) : _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        auto reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(), _state, reader);
    }
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    return match_action<void, Input>()(Production{}, input);
}
template <typename Production, typename Input, typename State>
constexpr bool match(const Input& input, State& state)
{
    return match_action<State, Input>(state)(Production{}, input);
}
template <typename Production, typename Input, typename State>
constexpr bool match(const Input& input, const State& state)
{
    return match_action<const State, Input>(state)(Production{}, input);
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

template <typename ImplOrTag, bool IsImpl = lexy::is_token_rule<ImplOrTag>>
struct _token_inherit;
template <typename Impl>
struct _token_inherit<Impl, true> : Impl // no need to inherit from _token_base
{};
template <typename Tag>
struct _token_inherit<Tag, false> : _token_base, Tag // need to turn it into a token
{};

// ImplOrTag is either a branch base that indicates whether the token is an unconditional branch,
// or an actual token rule whose implementation will be used.
template <typename Derived, typename ImplOrTag = branch_base>
struct token_base : _token_inherit<ImplOrTag>
{
    using token_type = Derived;

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<Derived, Reader> parser(reader);
            auto                                    result = parser.try_parse(reader);
            end                                            = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::token{}, Derived{}, reader.position(), end.position());
            reader.reset(end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    LEXY_PARSER_FUNC static bool token_parse(Context& context, Reader& reader)
    {
        auto                                    begin = reader.position();
        lexy::token_parser_for<Derived, Reader> parser(reader);

        using try_parse_result = decltype(parser.try_parse(reader));
        if constexpr (std::is_same_v<try_parse_result, std::true_type>)
        {
            parser.try_parse(reader);
        }
        else
        {
            if (!parser.try_parse(reader))
            {
                context.on(_ev::token{}, lexy::error_token_kind, reader.position(),
                           parser.end.position());
                parser.report_error(context, reader);
                reader.reset(parser.end);

                return false;
            }
        }

        context.on(_ev::token{}, typename Derived::token_type{}, begin, parser.end.position());
        reader.reset(parser.end);

        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!token_parse(context, reader))
                return false;
            else
                return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                           LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _toke<Tag, Derived> error = _toke<Tag, Derived>{};

    template <auto Kind>
    static constexpr _tokk<Kind, Derived> kind = _tokk<Kind, Derived>{};
};

// We forward all implementation to Token.
// We cannot directly inherit from Token as that wouldn't override the token_type.
template <auto Kind, typename Token>
struct _tokk : token_base<_tokk<Kind, Token>, Token>
{};

template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>, Token>
{
    // If we're overriding the error for a char class rule, we also want to change its error
    // reporting. Otherwise, rules such as `dsl::delimited()` building on char classes will generate
    // the "wrong" error.
    //
    // If it's not a char class, adding this function doesn't hurt.
    template <typename Reader, typename Context>
    static constexpr void char_class_report_error(Context&                  context,
                                                  typename Reader::iterator position)
    {
        auto err = lexy::error<Reader, Tag>(position, position);
        context.on(_ev::error{}, err);
    }

    template <typename Reader>
    struct tp : lexy::token_parser_for<Token, Reader>
    {
        constexpr explicit tp(const Reader& reader) : lexy::token_parser_for<Token, Reader>(reader)
        {}

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            // Report a different error.
            auto err = lexy::error<Reader, Tag>(reader.position(), this->end.position());
            context.on(_ev::error{}, err);
        }
    };
};
} // namespace lexyd

namespace lexy
{
template <auto Kind, typename Token>
constexpr auto token_kind_of<lexy::dsl::_tokk<Kind, Token>> = Kind;
template <typename Tag, typename Token>
constexpr auto token_kind_of<lexy::dsl::_toke<Tag, Token>> = token_kind_of<Token>;
} // namespace lexy

//=== token rule ===//
namespace lexyd
{
template <typename Rule>
struct _token : token_base<_token<Rule>>
{
    struct _production
    {
        static constexpr auto name                = "<token>";
        static constexpr auto max_recursion_depth = 0;
        static constexpr auto rule                = Rule{};
    };

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // We match a dummy production that only consists of the rule.
            auto success = lexy::do_action<
                _production,
                lexy::match_action<void, Reader>::template result_type>(lexy::_mh(),
                                                                        lexy::no_parse_state,
                                                                        reader);
            end = reader.current();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::missing_token>(reader.position(), end.position());
            context.on(_ev::error{}, err);
        }
    };
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


namespace lexyd
{
struct _any : token_base<_any, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;
            if constexpr (lexy::_detail::is_swar_reader<Reader>)
            {
                while (!lexy::_detail::swar_has_char<typename encoding::char_type, encoding::eof()>(
                    reader.peek_swar()))
                    reader.bump_swar();
            }

            while (reader.peek() != encoding::eof())
                reader.bump();

            end = reader.current();
            return {};
        }
    };
};

/// Matches anything and consumes all remaining characters.
constexpr auto any = _any{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_any> = lexy::any_token_kind;
}

#endif // LEXY_DSL_ANY_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_PARSE_TREE_HPP_INCLUDED
#define LEXY_PARSE_TREE_HPP_INCLUDED




// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED
#define LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED

#include <cstring>


#include <new>

#if 0 // NOLINT
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
            return ::operator new(bytes, std::align_val_t{alignment});
        else
            return ::operator new(bytes);
    }

    static void deallocate(void* ptr, std::size_t bytes, std::size_t alignment) noexcept
    {
#if LEXY_ENABLE_ASSERT
        // In debug mode, we fill freed memory with 0xFF to detect dangling lexemes.
        // For default, ASCII, bytes, this is just a noticable value.
        // For UTF-8, this is the EOF integer value as its an invalid code unit.
        // For UTF-16, this is the code point 0xFFFF, which is the replacement character.
        // For UTF-32, this is an out of range code point.
        std::memset(ptr, 0xFF, bytes);
#endif

#ifdef __cpp_sized_deallocation
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            ::operator delete(ptr, bytes, std::align_val_t{alignment});
        else
            ::operator delete(ptr, bytes);
#else
        (void)bytes;

        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            ::operator delete(ptr, std::align_val_t{alignment});
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
    constexpr explicit _memory_resource_ptr_empty(void*) noexcept {}

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

// clang-format off
template <typename MemoryResource>
using memory_resource_ptr
    = std::conditional_t<std::is_void_v<MemoryResource>,
            _memory_resource_ptr_empty<default_memory_resource>,
            std::conditional_t<std::is_empty_v<MemoryResource>,
                _memory_resource_ptr_empty<MemoryResource>,
                _memory_resource_ptr<MemoryResource>>>;
// clang-format on

template <typename MemoryResource, typename = std::enable_if_t<std::is_void_v<MemoryResource>
                                                               || std::is_empty_v<MemoryResource>>>
constexpr MemoryResource* get_memory_resource()
{
    return nullptr;
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_TOKEN_HPP_INCLUDED
#define LEXY_TOKEN_HPP_INCLUDED

#include <climits>
#include <cstdint>





// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_LEXEME_HPP_INCLUDED
#define LEXY_LEXEME_HPP_INCLUDED







namespace lexy
{
template <typename Reader>
class lexeme
{
public:
    using encoding  = typename Reader::encoding;
    using iterator  = typename Reader::iterator;
    using char_type = LEXY_DECAY_DECLTYPE(*LEXY_DECLVAL(iterator&));

    constexpr lexeme() noexcept : _begin(), _end() {}
    constexpr lexeme(iterator begin, iterator end) noexcept : _begin(begin), _end(end) {}
    constexpr lexeme(iterator pos, std::size_t size) noexcept
    : _begin(pos), _end(_detail::next(pos, size))
    {}

    constexpr explicit lexeme(const Reader& reader, iterator begin) noexcept
    : _begin(begin), _end(reader.position())
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator lexeme<OtherReader>() const noexcept
    {
        return lexeme<OtherReader>(this->begin(), this->end());
    }

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
        return _tk_map<LEXY_DECAY_DECLTYPE(TokenKind), Token>(lexy::_detail::index_sequence_for<>{},
                                                              nullptr, TokenKind);
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
template <typename TokenRule>
constexpr auto _has_special_token_kind = [] {
    using kind = LEXY_DECAY_DECLTYPE(lexy::token_kind_of<TokenRule>);
    return !std::is_same_v<kind, lexy::predefined_token_kind> && std::is_enum_v<kind>;
}();

/// What sort of token it is.
template <typename TokenKind = void>
class token_kind
{
    static_assert(std::is_void_v<TokenKind> || std::is_enum_v<TokenKind>,
                  "invalid type for TokenKind");
    using _underlying_type = lexy::_detail::type_or<TokenKind, int>;

    template <typename T>
    static constexpr bool _is_compatible_kind_type()
    {
        using type = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<type, lexy::predefined_token_kind>)
            // Always compatible.
            return true;
        else if constexpr (std::is_void_v<TokenKind>)
            // We neeed an integer for our token kind.
            return std::is_integral_v<T>;
        else
            // We need the same enumeration type.
            return std::is_same_v<type, TokenKind>;
    }

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
    constexpr token_kind(TokenRule) noexcept
    // We initialize it according to the external mapping.
    : token_kind(token_kind_map_for<TokenKind>.lookup(TokenRule{}))
    {
        // If unknown, override it by internal mapping.
        if (!*this)
        {
            constexpr auto token_rule_kind = lexy::token_kind_of<TokenRule>;
            if constexpr (_is_compatible_kind_type<decltype(token_rule_kind)>())
            {
                // The token has an associated kind of the same type.
                *this = token_kind(token_rule_kind);
            }
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

    constexpr bool ignore_if_empty() const noexcept
    {
        return _value == unknown_token_kind || _value == error_token_kind
               || _value == whitespace_token_kind;
    }

    constexpr const char* name() const noexcept
    {
        if (is_predefined())
            return token_kind_name(static_cast<predefined_token_kind>(_value));
        else
            return token_kind_name(get()); // ADL
    }
    friend constexpr const char* token_kind_name(token_kind kind) noexcept
    {
        return kind.name();
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
template <typename TokenRule, typename = std::enable_if_t<_has_special_token_kind<TokenRule>>>
token_kind(TokenRule) -> token_kind<LEXY_DECAY_DECLTYPE(lexy::token_kind_of<TokenRule>)>;
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
template <typename TokenRule, typename Reader,
          typename = std::enable_if_t<_has_special_token_kind<TokenRule>>>
token(TokenRule,
      lexy::lexeme<Reader>) -> token<Reader, LEXY_DECAY_DECLTYPE(lexy::token_kind_of<TokenRule>)>;

template <typename Input, typename TokenKind = void>
using token_for = token<lexy::input_reader<Input>, TokenKind>;
} // namespace lexy

#endif // LEXY_TOKEN_HPP_INCLUDED


namespace lexy
{
template <typename Node>
struct parse_tree_input_traits;
}

//=== internal: pt_node ===//
namespace lexy::_detail
{
template <typename Reader>
class pt_node;
template <typename Reader>
struct pt_node_token;
template <typename Reader>
struct pt_node_production;

template <typename Reader>
class pt_node
{
public:
    static constexpr auto type_token      = 0b0u;
    static constexpr auto type_production = 0b1u;

    static constexpr auto role_sibling = 0b0u;
    static constexpr auto role_parent  = 0b1u;

    //=== information about the current node ===//
    unsigned type() const noexcept
    {
        return _value & 0b1;
    }

    auto as_token() noexcept
    {
        return type() == type_token ? static_cast<pt_node_token<Reader>*>(this) : nullptr;
    }
    auto as_production() noexcept
    {
        return type() == type_production ? static_cast<pt_node_production<Reader>*>(this) : nullptr;
    }

    //=== information about the next node ===//
    void set_sibling(pt_node<Reader>* sibling) noexcept
    {
        _value = _make_packed_ptr(sibling, type(), role_sibling);
    }
    void set_parent(pt_node_production<Reader>* parent) noexcept
    {
        _value = _make_packed_ptr(parent, type(), role_parent);
    }

    unsigned next_role() const noexcept
    {
        return (_value & 0b10) >> 1;
    }

    pt_node<Reader>* next_node() const noexcept
    {
        // NOLINTNEXTLINE: We need pointer conversion.
        return reinterpret_cast<pt_node<Reader>*>(_value & ~std::uintptr_t(0b11));
    }

protected:
    explicit pt_node(unsigned type)
    // We initializy it to a null parent pointer.
    // This means it is automatically an empty child range.
    : _value(_make_packed_ptr(nullptr, type, role_parent))
    {}

private:
    static std::uintptr_t _make_packed_ptr(pt_node<Reader>* ptr, unsigned type, unsigned role)
    {
        auto result = reinterpret_cast<std::uintptr_t>(ptr);
        LEXY_PRECONDITION((result & 0b11) == 0);

        result |= (role & 0b1) << 1;
        result |= (type & 0b1);

        return result;
    }

    // Stores an address of the "next" node in the tree.
    // Stores a bit that remembers whether the next node is a sibling or parent (the role).
    // Stores a bit that remembers whether *this* node is a token or production.
    std::uintptr_t _value;
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
    : pt_node<Reader>(pt_node<Reader>::type_token), begin(begin), kind(kind)
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
            auto size = std::size_t(end - begin);
            LEXY_PRECONDITION(size <= UINT_LEAST32_MAX);
            end_impl = std::uint_least32_t(size);
        }
        else
        {
            end_impl = end;
        }
    }
};

template <typename Reader>
struct pt_node_production : pt_node<Reader>
{
    static constexpr std::size_t child_count_bits = sizeof(std::size_t) * CHAR_BIT - 2;

    const char* const* id;
    std::size_t        child_count : child_count_bits;
    std::size_t        token_production : 1;
    std::size_t        first_child_adjacent : 1;

    explicit pt_node_production(production_info info) noexcept
    : pt_node<Reader>(pt_node<Reader>::type_production), id(info.id), child_count(0),
      token_production(info.is_token), first_child_adjacent(true)
    {
        LEXY_PRECONDITION(!info.is_transparent);
    }

    pt_node<Reader>* first_child()
    {
        auto memory = static_cast<void*>(this + 1);
        if (child_count == 0)
        {
            // We don't have a child at all.
            return nullptr;
        }
        else if (first_child_adjacent)
        {
            // The first child is stored immediately afterwards.
            return static_cast<pt_node<Reader>*>(memory);
        }
        else
        {
            // We're only storing a pointer to the first child immediately afterwards.
            return *static_cast<pt_node<Reader>**>(memory);
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
        constexpr auto size = sizeof(T); // NOLINT: It's fine, we want to allocate for a pointer.
        LEXY_PRECONDITION(_cur_block);   // Forgot to call .init().
        LEXY_PRECONDITION(remaining_capacity() >= size); // Forgot to call .reserve().

        auto memory = _cur_pos;
        _cur_pos += size;
        return ::new (static_cast<void*>(memory)) T(LEXY_FWD(args)...);
    }

    void* top()
    {
        return _cur_pos;
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
template <typename Reader, typename TokenKind>
class _pt_node_kind;
template <typename Reader, typename TokenKind>
class _pt_node;

template <typename Reader, typename TokenKind = void, typename MemoryResource = void>
class parse_tree
{
    static_assert(lexy::is_char_encoding<typename Reader::encoding>);

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
    using node_kind = _pt_node_kind<Reader, TokenKind>;
    using node      = _pt_node<Reader, TokenKind>;

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

    //=== remaining input ===//
    lexy::lexeme<Reader> remaining_input() const noexcept
    {
        if (empty())
            return {};

        auto token = _root->next_node()->as_token();
        return {token->begin, token->end()};
    }

private:
    _detail::pt_buffer<MemoryResource>   _buffer;
    _detail::pt_node_production<Reader>* _root;
    std::size_t                          _size;
    std::size_t                          _depth;
};

template <typename Input, typename TokenKind = void, typename MemoryResource = void>
using parse_tree_for = lexy::parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>;

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::builder
{
public:
    class marker
    {
    public:
        marker() : marker(nullptr, 0) {}

    private:
        // Where to unwind when done.
        void* unwind_pos;
        // The current production node.
        // nullptr if using the container API.
        _detail::pt_node_production<Reader>* prod;
        // The number of children we've already added.
        std::size_t child_count;
        // The first and last child of the container.
        _detail::pt_node<Reader>* first_child;
        _detail::pt_node<Reader>* last_child;

        // For a production node, depth of the production node.
        // For a container node, depth of the children.
        std::size_t cur_depth;
        // The maximum local depth seen in the subtree beginning at the marker.
        std::size_t local_max_depth;

        explicit marker(void* unwind_pos, std::size_t cur_depth,
                        _detail::pt_node_production<Reader>* prod = nullptr)
        : unwind_pos(unwind_pos), prod(prod), child_count(0), first_child(nullptr),
          last_child(nullptr), cur_depth(cur_depth), local_max_depth(cur_depth)
        {}

        void clear()
        {
            first_child = nullptr;
            last_child  = nullptr;
            child_count = 0;
        }

        void insert(_detail::pt_node<Reader>* child)
        {
            if (first_child == nullptr)
            {
                // Initialize the pointers.
                first_child = last_child = child;
            }
            else
            {
                // last_child gets a sibling pointer.
                last_child->set_sibling(child);
                // child is now the last child.
                last_child = child;
            }

            ++child_count;
        }
        void insert_list(std::size_t length, _detail::pt_node<Reader>* first,
                         _detail::pt_node<Reader>* last)
        {
            if (length == 0)
                return;

            if (first_child == nullptr)
            {
                first_child = first;
                last_child  = last;
            }
            else
            {
                last_child->set_sibling(first);
                last_child = last;
            }

            child_count += length;
        }

        void insert_children_into(_detail::pt_node_production<Reader>* parent)
        {
            LEXY_PRECONDITION(parent->child_count == 0);
            if (child_count == 0)
                return;

            if (first_child == parent + 1)
            {
                parent->first_child_adjacent = true;
            }
            else
            {
                // This case happens either if the first child is in a new block or if we're
                // dealing with a container node. In either case, we've already reserved memory
                // for a pointer.
                auto memory = static_cast<void*>(parent + 1);
                ::new (memory) _detail::pt_node<Reader>*(first_child);
                parent->first_child_adjacent = false;
            }

            // last_child needs a pointer to the production.
            last_child->set_parent(parent);

            constexpr auto mask
                = ((std::size_t(1) << _detail::pt_node_production<Reader>::child_count_bits) - 1);
            parent->child_count = child_count & mask;
        }

        void update_size_depth(std::size_t& size, std::size_t& max_depth)
        {
            size += child_count;

            if (cur_depth == local_max_depth && child_count > 0)
                // We have children we haven't yet accounted for.
                ++local_max_depth;

            if (max_depth < local_max_depth)
                max_depth = local_max_depth;
        }

        friend builder;
    };

    //=== root node ===//
    explicit builder(parse_tree&& tree, production_info production) : _result(LEXY_MOV(tree))
    {
        // Empty the initial parse tree.
        _result._buffer.reset();

        // Allocate a new root node.
        // No need to reserve for the initial node.
        _result._root
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _result._size  = 1;
        _result._depth = 0;

        // Begin construction at the root.
        _cur = marker(_result._buffer.top(), 0, _result._root);
    }
    explicit builder(production_info production) : builder(parse_tree(), production) {}

    [[deprecated("Pass the remaining input, or `input.end()` if there is none.")]] parse_tree&&
        finish() &&
    {
        return LEXY_MOV(*this).finish(lexy::lexeme<Reader>());
    }
    parse_tree&& finish(typename Reader::iterator end) &&
    {
        return LEXY_MOV(*this).finish({end, end});
    }
    parse_tree&& finish(lexy::lexeme<Reader> remaining_input) &&
    {
        LEXY_PRECONDITION(_cur.prod == _result._root);

        _cur.insert_children_into(_cur.prod);
        _cur.update_size_depth(_result._size, _result._depth);

        _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
        auto node = _result._buffer
                        .template allocate<_detail::pt_node_token<Reader>>(lexy::eof_token_kind,
                                                                           remaining_input.begin(),
                                                                           remaining_input.end());
        _result._root->set_sibling(node);

        return LEXY_MOV(_result);
    }

    //=== production nodes ===//
    auto start_production(production_info production)
    {
        if (production.is_transparent)
            // Don't need to add a new node for a transparent production.
            return _cur;

        // Allocate a node for the production and append it to the current child list.
        // We reserve enough memory to allow for a trailing pointer.
        // This is only necessary if the first child is in a new block,
        // in which case we won't overwrite it.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node<Reader>*));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        // Note: don't append the node yet, we might still backtrack.

        // Subsequent insertions are to the new node, so update marker and return old one.
        auto old = LEXY_MOV(_cur);
        _cur     = marker(node, old.cur_depth + 1, node);
        return old;
    }

    void finish_production(marker&& m)
    {
        LEXY_PRECONDITION(_cur.prod || m.prod == _cur.prod);
        if (m.prod == _cur.prod)
            // We're finishing with a transparent production, do nothing.
            return;

        _cur.update_size_depth(_result._size, m.local_max_depth);
        _cur.insert_children_into(_cur.prod);

        // Insert the production node into the parent and continue with it.
        m.insert(_cur.prod);
        _cur = LEXY_MOV(m);
    }

    void cancel_production(marker&& m)
    {
        LEXY_PRECONDITION(_cur.prod || m.prod == _cur.prod);
        if (_cur.prod == m.prod)
            // We're backtracking a transparent production, do nothing.
            return;

        _result._buffer.unwind(_cur.unwind_pos);
        // Continue with parent.
        _cur = LEXY_MOV(m);
    }

    //=== container nodes ===//
    marker start_container()
    {
        auto unwind_pos = _result._buffer.top();
        if (_cur.prod && _cur.child_count == 0)
        {
            // If our parent production doesn't have any children,
            // we might need space for a first child pointer.
            //
            // If our parent is another container, its start_container() function took care of it.
            // If our parent already has a child, the first_child pointer is taken care of.
            _result._buffer.template allocate<_detail::pt_node<Reader>*>(nullptr);
        }

        // Create a new container marker and activate it.
        auto old = LEXY_MOV(_cur);
        _cur     = marker(unwind_pos, old.cur_depth);
        return old;
    }

    void set_container_production(production_info production)
    {
        LEXY_PRECONDITION(!_cur.prod);
        if (production.is_transparent)
            // If the production is transparent, we do nothing.
            return;

        // Allocate a new node for the production.
        // We definitely need space for node pointer at this point,
        // as the first child precedes it.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node<Reader>*));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _result._buffer.template allocate<_detail::pt_node<Reader>*>(nullptr);

        // Create a new container that will contain the production as its only child.
        // As such, it logically starts at the same position and depth as the current container.
        auto new_container = marker(_cur.unwind_pos, _cur.cur_depth);
        new_container.insert(node);

        // The production contains all the children.
        _cur.insert_children_into(node);

        // The local_max_depth of the new container is determined by the old maximum depth + 1.
        new_container.local_max_depth = [&] {
            if (_cur.cur_depth == _cur.local_max_depth && _cur.child_count > 0)
                // There are children we haven't yet accounted for.
                return _cur.local_max_depth + 1 + 1;
            else
                return _cur.local_max_depth + 1;
        }();
        _result._size += _cur.child_count;

        // And we continue with the current container.
        _cur = new_container;
    }

    void finish_container(marker&& m)
    {
        LEXY_PRECONDITION(!_cur.prod);

        // Insert the children of our container into the parent.
        m.insert_list(_cur.child_count, _cur.first_child, _cur.last_child);

        // We can't update size yet, it would be double counted.
        // We do need to update the max depth if necessary, however.
        std::size_t size = 0;
        _cur.update_size_depth(size, m.local_max_depth);

        // Continue with the parent.
        _cur = LEXY_MOV(m);
    }

    void cancel_container(marker&& m)
    {
        LEXY_PRECONDITION(!_cur.prod);

        // Deallocate everything we've inserted.
        _result._buffer.unwind(_cur.unwind_pos);
        // Continue with parent.
        _cur = LEXY_MOV(m);
    }

    //=== token nodes ===//
    void token(token_kind<TokenKind> _kind, typename Reader::iterator begin,
               typename Reader::iterator end)
    {
        if (_kind.ignore_if_empty() && begin == end)
            return;

        auto kind = token_kind<TokenKind>::to_raw(_kind);

        // We merge error tokens.
        if (kind == lexy::error_token_kind && _cur.last_child && _cur.last_child->as_token()
            && _cur.last_child->as_token()->kind == lexy::error_token_kind)
        {
            // No need to allocate a new node, just extend the previous node.
            _cur.last_child->as_token()->update_end(end);
        }
        else
        {
            // Allocate and append.
            _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
            auto node
                = _result._buffer.template allocate<_detail::pt_node_token<Reader>>(kind, begin,
                                                                                    end);
            _cur.insert(node);
        }
    }

    //=== accessors ===//
    std::size_t current_child_count() const noexcept
    {
        return _cur.child_count;
    }

private:
    parse_tree _result;
    marker     _cur;
};

template <typename Reader, typename TokenKind>
class _pt_node_kind
{
public:
    bool is_token() const noexcept
    {
        return _ptr->as_token() != nullptr;
    }
    bool is_production() const noexcept
    {
        return _ptr->as_production() != nullptr;
    }

    bool is_root() const noexcept
    {
        // Root node has a next node (the remaining input node) which has no next node.
        // We assume that _ptr is never the remaining input node, so we know that we have a next
        // node.
        return _ptr->next_node()->next_node() == nullptr;
    }
    bool is_token_production() const noexcept
    {
        return is_production() && _ptr->as_production()->token_production;
    }

    const char* name() const noexcept
    {
        if (auto prod = _ptr->as_production())
            return *prod->id;
        else if (auto token = _ptr->as_token())
            return token_kind<TokenKind>::from_raw(token->kind).name();
        else
        {
            LEXY_ASSERT(false, "unreachable");
            return nullptr;
        }
    }

    friend bool operator==(_pt_node_kind lhs, _pt_node_kind rhs)
    {
        if (lhs.is_token() && rhs.is_token())
            return lhs._ptr->as_token()->kind == rhs._ptr->as_token()->kind;
        else
            return lhs._ptr->as_production()->id == rhs._ptr->as_production()->id;
    }
    friend bool operator!=(_pt_node_kind lhs, _pt_node_kind rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator==(_pt_node_kind nk, token_kind<TokenKind> tk)
    {
        if (auto token = nk._ptr->as_token())
            return token_kind<TokenKind>::from_raw(token->kind) == tk;
        else
            return false;
    }
    friend bool operator==(token_kind<TokenKind> tk, _pt_node_kind nk)
    {
        return nk == tk;
    }
    friend bool operator!=(_pt_node_kind nk, token_kind<TokenKind> tk)
    {
        return !(nk == tk);
    }
    friend bool operator!=(token_kind<TokenKind> tk, _pt_node_kind nk)
    {
        return !(nk == tk);
    }

    friend bool operator==(_pt_node_kind nk, production_info info)
    {
        return nk.is_production() && nk._ptr->as_production()->id == info.id;
    }
    friend bool operator==(production_info info, _pt_node_kind nk)
    {
        return nk == info;
    }
    friend bool operator!=(_pt_node_kind nk, production_info info)
    {
        return !(nk == info);
    }
    friend bool operator!=(production_info info, _pt_node_kind nk)
    {
        return !(nk == info);
    }

private:
    explicit _pt_node_kind(_detail::pt_node<Reader>* ptr) : _ptr(ptr) {}

    _detail::pt_node<Reader>* _ptr;

    friend _pt_node<Reader, TokenKind>;
};

template <typename Reader, typename TokenKind>
class _pt_node
{
public:
    void* address() const noexcept
    {
        return _ptr;
    }

    auto kind() const noexcept
    {
        return _pt_node_kind<Reader, TokenKind>(_ptr);
    }

    auto parent() const noexcept
    {
        if (kind().is_root())
            // The root has itself as parent.
            return *this;

        // If we follow the sibling pointer, we reach a parent pointer.
        auto cur = _ptr;
        while (cur->next_role() == _detail::pt_node<Reader>::role_sibling)
            cur = cur->next_node();
        return _pt_node(cur->next_node());
    }

    class children_range
    {
    public:
        class iterator : public _detail::forward_iterator_base<iterator, _pt_node, _pt_node, void>
        {
        public:
            iterator() noexcept : _cur(nullptr) {}

            auto deref() const noexcept
            {
                return _pt_node(_cur);
            }

            void increment() noexcept
            {
                _cur = _cur->next_node();
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur == rhs._cur;
            }

        private:
            explicit iterator(_detail::pt_node<Reader>* ptr) noexcept : _cur(ptr) {}

            _detail::pt_node<Reader>* _cur;

            friend children_range;
        };

        bool empty() const noexcept
        {
            return size() == 0;
        }

        std::size_t size() const noexcept
        {
            if (auto prod = _node->as_production())
                return prod->child_count;
            else
                return 0;
        }

        iterator begin() const noexcept
        {
            if (auto prod = _node->as_production(); prod && prod->first_child())
                return iterator(prod->first_child());
            else
                return end();
        }
        iterator end() const noexcept
        {
            // The last child has a next pointer back to the parent,
            // so if we keep following it, we'll end up here.
            return iterator(_node);
        }

    private:
        explicit children_range(_detail::pt_node<Reader>* node) : _node(node)
        {
            LEXY_PRECONDITION(node);
        }

        _detail::pt_node<Reader>* _node;

        friend _pt_node;
    };

    auto children() const noexcept
    {
        return children_range(_ptr);
    }

    class sibling_range
    {
    public:
        class iterator : public _detail::forward_iterator_base<iterator, _pt_node, _pt_node, void>
        {
        public:
            iterator() noexcept : _cur() {}

            auto deref() const noexcept
            {
                return _pt_node(_cur);
            }

            void increment() noexcept
            {
                if (_cur->next_role() == _detail::pt_node<Reader>::role_parent)
                    // We're pointing to the parent, go to first child instead.
                    _cur = _cur->next_node()->as_production()->first_child();
                else
                    // We're pointing to a sibling, go there.
                    _cur = _cur->next_node();
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur == rhs._cur;
            }

        private:
            explicit iterator(_detail::pt_node<Reader>* ptr) noexcept : _cur(ptr) {}

            _detail::pt_node<Reader>* _cur;

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
        explicit sibling_range(_detail::pt_node<Reader>* node) noexcept : _node(node) {}

        _detail::pt_node<Reader>* _node;

        friend _pt_node;
    };

    auto siblings() const noexcept
    {
        return sibling_range(_ptr);
    }

    bool is_last_child() const noexcept
    {
        // We're the last child if our pointer points to the parent.
        return _ptr->next_role() == _detail::pt_node<Reader>::role_parent;
    }

    auto position() const noexcept -> typename Reader::iterator
    {
        // Find the first descendant that is a token.
        auto cur = _ptr;
        while (cur->type() == _detail::pt_node<Reader>::type_production)
        {
            cur = cur->as_production()->first_child();
            LEXY_PRECONDITION(cur);
        }

        return cur->as_token()->begin;
    }

    auto lexeme() const noexcept
    {
        if (auto token = _ptr->as_token())
            return lexy::lexeme<Reader>(token->begin, token->end());
        else
            return lexy::lexeme<Reader>();
    }

    auto covering_lexeme() const noexcept
    {
        if (auto token = _ptr->as_token())
            return lexy::lexeme<Reader>(token->begin, token->end());

        auto begin = position();

        auto sibling = _ptr;
        while (true)
        {
            auto next_role = sibling->next_role();
            sibling        = sibling->next_node();
            // If we went to parent, we need to continue finding siblings.
            if (next_role == _detail::pt_node<Reader>::role_sibling)
                break;
        }
        auto end = _pt_node(sibling).position();

        LEXY_PRECONDITION(begin == end || end != typename Reader::iterator());
        return lexy::lexeme<Reader>(begin, end);
    }

    auto token() const noexcept
    {
        LEXY_PRECONDITION(kind().is_token());

        auto token = _ptr->as_token();
        auto kind  = token_kind<TokenKind>::from_raw(token->kind);
        return lexy::token<Reader, TokenKind>(kind, token->begin, token->end());
    }

    friend bool operator==(_pt_node lhs, _pt_node rhs) noexcept
    {
        return lhs._ptr == rhs._ptr;
    }
    friend bool operator!=(_pt_node lhs, _pt_node rhs) noexcept
    {
        return lhs._ptr != rhs._ptr;
    }

private:
    explicit _pt_node(_detail::pt_node<Reader>* ptr) noexcept : _ptr(ptr) {}

    _detail::pt_node<Reader>* _ptr;

    friend parse_tree<Reader, TokenKind>;
    friend parse_tree_input_traits<_pt_node<Reader, TokenKind>>;
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
            return {_ev, node(_cur)};
        }

        void increment() noexcept
        {
            if (_ev == traverse_event::enter)
            {
                auto child = _cur->as_production()->first_child();
                if (child)
                {
                    // We go to the first child next.
                    if (child->as_token())
                        _ev = traverse_event::leaf;
                    else
                        _ev = traverse_event::enter;

                    _cur = child;
                }
                else
                {
                    // Don't have children, exit.
                    _ev = traverse_event::exit;
                }
            }
            else
            {
                // We follow the next pointer.

                if (_cur->next_role() == _detail::pt_node<Reader>::role_parent)
                    // We go back to a production for the second time.
                    _ev = traverse_event::exit;
                else if (_cur->next_node()->as_production())
                    // We're having a production as sibling.
                    _ev = traverse_event::enter;
                else
                    // Token as sibling.
                    _ev = traverse_event::leaf;

                _cur = _cur->next_node();
            }
        }

        bool equal(iterator rhs) const noexcept
        {
            return _ev == rhs._ev && _cur == rhs._cur;
        }

    private:
        _detail::pt_node<Reader>* _cur = nullptr;
        traverse_event            _ev;

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
            _begin._cur = n._ptr;
            _begin._ev  = traverse_event::leaf;

            _end = _detail::next(_begin);
        }
        else
        {
            _begin._cur = n._ptr;
            _begin._ev  = traverse_event::enter;

            _end._cur = n._ptr;
            _end._ev  = traverse_event::exit;
            ++_end; // half-open range
        }
    }

    iterator _begin, _end;

    friend parse_tree;
};
} // namespace lexy

#if LEXY_EXPERIMENTAL
namespace lexy
{
template <typename Reader, typename TokenKind>
struct parse_tree_input_traits<_pt_node<Reader, TokenKind>>
{
    using _node = _pt_node<Reader, TokenKind>;

    using char_encoding = typename Reader::encoding;

    static bool is_null(_node cur) noexcept
    {
        return cur._ptr == nullptr;
    }

    static _node null() noexcept
    {
        return _node(nullptr);
    }

    static _node first_child(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        if (auto prod = cur._ptr->as_production())
            return _node(prod->first_child());
        else
            return _node(nullptr);
    }

    static _node sibling(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur._ptr->next_role() == _detail::pt_node<Reader>::role_sibling
                   ? _node(cur._ptr->next_node())
                   : _node(nullptr);
    }

    template <typename Kind>
    static bool has_kind(_node cur, const Kind& kind) noexcept
    {
        return !is_null(cur) && cur.kind() == kind;
    }

    using iterator = typename Reader::iterator;

    static iterator position_begin(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur.position();
    }
    static iterator position_end(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur.covering_lexeme().end();
    }

    static auto lexeme(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur.lexeme();
    }
};
} // namespace lexy
#endif

#endif // LEXY_PARSE_TREE_HPP_INCLUDED


namespace lexy
{
template <typename Tree, typename Reader>
class _pth
{
public:
    template <typename Input, typename Sink>
    explicit _pth(Tree& tree, const _detail::any_holder<const Input*>& input,
                  _detail::any_holder<Sink>& sink)
    : _tree(&tree), _depth(0), _validate(input, sink)
    {}

    class event_handler
    {
        using iterator = typename Reader::iterator;

    public:
        event_handler(production_info info) : _validate(info) {}

        void on(_pth& handler, parse_events::grammar_start, iterator)
        {
            LEXY_PRECONDITION(handler._depth == 0);

            handler._builder.emplace(LEXY_MOV(*handler._tree), _validate.get_info());
        }
        void on(_pth& handler, parse_events::grammar_finish, Reader& reader)
        {
            LEXY_PRECONDITION(handler._depth == 0);

            auto begin = reader.position();
            lexy::try_match_token(dsl::any, reader);
            auto end = reader.position();

            *handler._tree = LEXY_MOV(*handler._builder).finish({begin, end});
        }
        void on(_pth& handler, parse_events::grammar_cancel, Reader&)
        {
            LEXY_PRECONDITION(handler._depth == 0);

            handler._tree->clear();
        }

        void on(_pth& handler, parse_events::production_start ev, iterator pos)
        {
            if (handler._depth++ > 0)
                _marker = handler._builder->start_production(_validate.get_info());

            _validate.on(handler._validate, ev, pos);
        }

        void on(_pth& handler, parse_events::production_finish ev, iterator pos)
        {
            if (--handler._depth > 0)
            {
                if (handler._builder->current_child_count() == 0)
                    handler._builder->token(lexy::position_token_kind, _validate.production_begin(),
                                            _validate.production_begin());
                handler._builder->finish_production(LEXY_MOV(_marker));
            }

            _validate.on(handler._validate, ev, pos);
        }

        void on(_pth& handler, parse_events::production_cancel ev, iterator pos)
        {
            if (--handler._depth > 0)
            {
                // Cancelling the production removes all nodes from the tree.
                // To ensure that the parse tree remains lossless, we add everything consumed by it
                // as an error token.
                handler._builder->cancel_production(LEXY_MOV(_marker));
                handler._builder->token(lexy::error_token_kind, _validate.production_begin(), pos);
            }

            _validate.on(handler._validate, ev, pos);
        }

        auto on(_pth& handler, lexy::parse_events::operation_chain_start, iterator)
        {
            // As we don't know the production yet (or whether it is actually an operation),
            // we create a container node to decide later.
            return handler._builder->start_container();
        }
        template <typename Operation>
        void on(_pth& handler, lexy::parse_events::operation_chain_op, Operation op, iterator)
        {
            // We set the production of the current container.
            // This will do a "left rotation" on the parse tree, making a new container the parent.
            handler._builder->set_container_production(op);
        }
        template <typename Marker>
        void on(_pth& handler, lexy::parse_events::operation_chain_finish, Marker&& marker,
                iterator)
        {
            handler._builder->finish_container(LEXY_MOV(marker));
        }

        template <typename TokenKind>
        void on(_pth& handler, parse_events::token, TokenKind kind, iterator begin, iterator end)
        {
            handler._builder->token(kind, begin, end);
        }

        template <typename Error>
        void on(_pth& handler, parse_events::error ev, Error&& error)
        {
            _validate.on(handler._validate, ev, LEXY_FWD(error));
        }

        template <typename Event, typename... Args>
        auto on(_pth& handler, Event ev, Args&&... args)
        {
            return _validate.on(handler._validate, ev, LEXY_FWD(args)...);
        }

    private:
        typename Tree::builder::marker      _marker;
        typename _vh<Reader>::event_handler _validate;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename T>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        LEXY_PRECONDITION(_depth == 0);
        return LEXY_MOV(_validate).template get_result<T>(rule_parse_result);
    }

private:
    lexy::_detail::lazy_init<typename Tree::builder> _builder;
    Tree*                                            _tree;
    int                                              _depth;

    _vh<Reader> _validate;
};

template <typename State, typename Input, typename ErrorCallback, typename TokenKind = void,
          typename MemoryResource = void>
struct parse_as_tree_action
{
    using tree_type = lexy::parse_tree_for<Input, TokenKind, MemoryResource>;

    tree_type*           _tree;
    const ErrorCallback* _callback;
    State*               _state = nullptr;

    using handler = _pth<tree_type, lexy::input_reader<Input>>;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = validate_result<ErrorCallback>;

    constexpr explicit parse_as_tree_action(tree_type& tree, const ErrorCallback& callback)
    : _tree(&tree), _callback(&callback)
    {}
    template <typename U = State>
    constexpr explicit parse_as_tree_action(U& state, tree_type& tree,
                                            const ErrorCallback& callback)
    : _tree(&tree), _callback(&callback), _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        _detail::any_holder input_holder(&input);
        _detail::any_holder sink(_get_error_sink(*_callback));
        auto                reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(*_tree, input_holder, sink), _state,
                                                        reader);
    }
};

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input&                                                      input,
                   const ErrorCallback& callback) -> validate_result<ErrorCallback>
{
    return parse_as_tree_action<void, Input, ErrorCallback, TokenKind,
                                MemoryResource>(tree, callback)(Production{}, input);
}
template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename State, typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, State& state,
                   const ErrorCallback& callback) -> validate_result<ErrorCallback>
{
    return parse_as_tree_action<State, Input, ErrorCallback, TokenKind,
                                MemoryResource>(state, tree, callback)(Production{}, input);
}
template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename State, typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const State& state,
                   const ErrorCallback& callback) -> validate_result<ErrorCallback>
{
    return parse_as_tree_action<const State, Input, ErrorCallback, TokenKind,
                                MemoryResource>(state, tree, callback)(Production{}, input);
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_TRACE_HPP_INCLUDED
#define LEXY_ACTION_TRACE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED
#define LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED





namespace lexy::_detail
{
// Note: we can't use type_string<auto...>, it doesn't work on older GCC.
template <typename CharT, CharT... Cs>
struct type_string
{
    using char_type = CharT;

    template <template <typename C, C...> typename T>
    using rename = T<CharT, Cs...>;

    static constexpr auto size = sizeof...(Cs);

    template <typename T = char_type>
    static constexpr T c_str[sizeof...(Cs) + 1] = {transcode_char<T>(Cs)..., T()};
};
} // namespace lexy::_detail

#if LEXY_HAS_NTTP // string NTTP implementation



namespace lexy::_detail
{
template <std::size_t N, typename CharT>
struct string_literal
{
    CharT data[N];

    using char_type = CharT;

    LEXY_CONSTEVAL string_literal(const CharT* str) : data{}
    {
        for (auto i = 0u; i != N; ++i)
            data[i] = str[i];
    }
    LEXY_CONSTEVAL string_literal(CharT c) : data{}
    {
        data[0] = c;
    }

    static LEXY_CONSTEVAL auto size()
    {
        return N;
    }
};
template <std::size_t N, typename CharT>
string_literal(const CharT (&)[N]) -> string_literal<N - 1, CharT>;
template <typename CharT>
string_literal(CharT) -> string_literal<1, CharT>;

template <template <typename C, C... Cs> typename T, string_literal Str, std::size_t... Idx>
auto _to_type_string(index_sequence<Idx...>)
{
    return T<typename decltype(Str)::char_type, Str.data[Idx]...>{};
}
template <template <typename C, C... Cs> typename T, string_literal Str>
using to_type_string
    = decltype(_to_type_string<T, Str>(make_index_sequence<decltype(Str)::size()>{}));
} // namespace lexy::_detail

#    define LEXY_NTTP_STRING(T, Str)                                                               \
        ::lexy::_detail::to_type_string<T, ::lexy::_detail::string_literal(Str)>

#elif defined(__GNUC__) // literal implementation

#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#    ifdef __clang__
#        pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#    endif

template <typename CharT, CharT... Cs>
constexpr ::lexy::_detail::type_string<CharT, Cs...> operator""_lexy_string_udl()
{
    return {};
}

#    define LEXY_NTTP_STRING(T, Str) decltype(Str##_lexy_string_udl)::rename<T>

#    pragma GCC diagnostic pop

#else // string<Cs...> macro implementation

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

template <template <typename CharT, CharT...> typename T, typename TypeString, std::size_t Size,
          std::size_t MaxSize>
struct macro_type_string
{
    static_assert(Size <= MaxSize, "string out of range");
    using type = typename TypeString::template rename<T>;
};

} // namespace lexy::_detail

#    define LEXY_NTTP_STRING_LENGTH(Str) (sizeof(Str) / sizeof(Str[0]) - 1)

// extract Ith character if not out of bounds
#    define LEXY_NTTP_STRING1(Str, I)                                                              \
        ::std::conditional_t<                                                                      \
            (I < LEXY_NTTP_STRING_LENGTH(Str)),                                                    \
            ::lexy::_detail::type_string<::LEXY_DECAY_DECLTYPE(Str[0]),                            \
                                         (I >= LEXY_NTTP_STRING_LENGTH(Str) ? Str[0] : Str[I])>,   \
            ::lexy::_detail::type_string<::LEXY_DECAY_DECLTYPE(Str[0])>>

// recursively split the string in two
#    define LEXY_NTTP_STRING2(Str, I)                                                              \
        ::lexy::_detail::cat<LEXY_NTTP_STRING1(Str, I), LEXY_NTTP_STRING1(Str, I + 1)>
#    define LEXY_NTTP_STRING4(Str, I)                                                              \
        ::lexy::_detail::cat<LEXY_NTTP_STRING2(Str, I), LEXY_NTTP_STRING2(Str, I + 2)>
#    define LEXY_NTTP_STRING8(Str, I)                                                              \
        ::lexy::_detail::cat<LEXY_NTTP_STRING4(Str, I), LEXY_NTTP_STRING4(Str, I + 4)>
#    define LEXY_NTTP_STRING16(Str, I)                                                             \
        ::lexy::_detail::cat<LEXY_NTTP_STRING8(Str, I), LEXY_NTTP_STRING8(Str, I + 8)>
#    define LEXY_NTTP_STRING32(Str, I)                                                             \
        ::lexy::_detail::cat<LEXY_NTTP_STRING16(Str, I), LEXY_NTTP_STRING16(Str, I + 16)>

// instantiate with overflow check
#    define LEXY_NTTP_STRING(T, Str)                                                               \
        ::lexy::_detail::macro_type_string<T, LEXY_NTTP_STRING32(Str, 0),                          \
                                           LEXY_NTTP_STRING_LENGTH(Str), 32>::type

#endif

#endif // LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_LOCATION_HPP_INCLUDED
#define LEXY_INPUT_LOCATION_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CODE_POINT_HPP_INCLUDED
#define LEXY_CODE_POINT_HPP_INCLUDED

#include <cstdint>



#if LEXY_HAS_UNICODE_DATABASE
#    define LEXY_UNICODE_CONSTEXPR constexpr
#else
#    define LEXY_UNICODE_CONSTEXPR
#endif

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

    //=== general category ===//
    enum general_category_t
    {
    // NOLINTNEXTLINE: can't use parentheses here
#define LEXY_UNICODE_CATEGORY(Short, Long) Short, Long = Short

        LEXY_UNICODE_CATEGORY(Lu, uppercase_letter),
        LEXY_UNICODE_CATEGORY(Ll, lowercase_letter),
        LEXY_UNICODE_CATEGORY(Lt, titlecase_letter),
        LEXY_UNICODE_CATEGORY(Lm, modifier_letter),
        LEXY_UNICODE_CATEGORY(Lo, other_letter),

        LEXY_UNICODE_CATEGORY(Mn, nonspacing_mark),
        LEXY_UNICODE_CATEGORY(Mc, spacing_mark),
        LEXY_UNICODE_CATEGORY(Me, enclosing_mark),

        LEXY_UNICODE_CATEGORY(Nd, decimal_number),
        LEXY_UNICODE_CATEGORY(Nl, letter_number),
        LEXY_UNICODE_CATEGORY(No, other_number),

        LEXY_UNICODE_CATEGORY(Pc, connector_punctuation),
        LEXY_UNICODE_CATEGORY(Pd, dash_punctuation),
        LEXY_UNICODE_CATEGORY(Ps, open_punctuation),
        LEXY_UNICODE_CATEGORY(Pe, closing_punctuation),
        LEXY_UNICODE_CATEGORY(Pi, initial_puncutation),
        LEXY_UNICODE_CATEGORY(Pf, final_puncutation),
        LEXY_UNICODE_CATEGORY(Po, other_punctuation),

        LEXY_UNICODE_CATEGORY(Sm, math_symbol),
        LEXY_UNICODE_CATEGORY(Sc, currency_symbol),
        LEXY_UNICODE_CATEGORY(Sk, modifier_symbol),
        LEXY_UNICODE_CATEGORY(So, other_symbol),

        LEXY_UNICODE_CATEGORY(Zs, space_separator),
        LEXY_UNICODE_CATEGORY(Zl, line_separator),
        LEXY_UNICODE_CATEGORY(Zp, paragraph_separator),

        LEXY_UNICODE_CATEGORY(Cc, control),
        LEXY_UNICODE_CATEGORY(Cf, format),
        LEXY_UNICODE_CATEGORY(Cs, surrogate),
        LEXY_UNICODE_CATEGORY(Co, private_use),
        LEXY_UNICODE_CATEGORY(Cn, unassigned),

#undef LEXY_UNICODE_CATEGORY
    };

    template <general_category_t... Cats>
    struct _gc_group
    {
        const char* name;

        friend constexpr bool operator==(_gc_group, general_category_t cat)
        {
            return ((cat == Cats) || ...);
        }
        friend constexpr bool operator==(general_category_t cat, _gc_group)
        {
            return ((cat == Cats) || ...);
        }

        friend constexpr bool operator!=(_gc_group, general_category_t cat)
        {
            return !(_gc_group{} == cat);
        }
        friend constexpr bool operator!=(general_category_t cat, _gc_group)
        {
            return !(_gc_group{} == cat);
        }
    };

#define LEXY_UNICODE_CATEGORY_GROUP(Name, Short, Long, ...)                                        \
    static constexpr _gc_group<__VA_ARGS__> Short{"code-point." Name};                             \
    static constexpr _gc_group<__VA_ARGS__> Long = Short

    LEXY_UNICODE_CATEGORY_GROUP("cased-letter", LC, cased_letter, Lu, Ll, Lt);
    LEXY_UNICODE_CATEGORY_GROUP("letter", L, letter, Lu, Ll, Lt, Lm, Lo);
    LEXY_UNICODE_CATEGORY_GROUP("mark", M, mark, Mn, Mc, Me);
    LEXY_UNICODE_CATEGORY_GROUP("number", N, number, Nd, Nl, No);
    LEXY_UNICODE_CATEGORY_GROUP("punctuation", P, punctuation, Pc, Pd, Ps, Pe, Pi, Pf, Po);
    LEXY_UNICODE_CATEGORY_GROUP("symbol", S, symbol, Sm, Sc, Sk, So);
    LEXY_UNICODE_CATEGORY_GROUP("separator", Z, separator, Zs, Zl, Zp);
    LEXY_UNICODE_CATEGORY_GROUP("other", C, other, Cc, Cf, Cs, Co, Cn);

#undef LEXY_UNICODE_CATEGORY_GROUP

    LEXY_UNICODE_CONSTEXPR general_category_t general_category() const noexcept;

    //=== comparision ===//
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

LEXY_UNICODE_CONSTEXPR code_point simple_case_fold(code_point cp) noexcept;
} // namespace lexy

namespace lexy::_detail
{
constexpr const char* general_category_name(lexy::code_point::general_category_t category)
{
    switch (category)
    {
    case lexy::code_point::Lu:
        return "code-point.uppercase-letter";
    case lexy::code_point::Ll:
        return "code-point.lowercase-letter";
    case lexy::code_point::Lt:
        return "code-point.titlecase-letter";
    case lexy::code_point::Lm:
        return "code-point.modifier-letter";
    case lexy::code_point::Lo:
        return "code-point.other-letter";

    case lexy::code_point::Mn:
        return "code-point.nonspacing-mark";
    case lexy::code_point::Mc:
        return "code-point.combining-mark";
    case lexy::code_point::Me:
        return "code-point.enclosing-mark";

    case lexy::code_point::Nd:
        return "code-point.decimal-number";
    case lexy::code_point::Nl:
        return "code-point.letter-number";
    case lexy::code_point::No:
        return "code-point.other-number";

    case lexy::code_point::Pc:
        return "code-point.connector-punctuation";
    case lexy::code_point::Pd:
        return "code-point.dash-punctuation";
    case lexy::code_point::Ps:
        return "code-point.open-punctuation";
    case lexy::code_point::Pe:
        return "code-point.close-punctuation";
    case lexy::code_point::Pi:
        return "code-point.initial-quote-punctuation";
    case lexy::code_point::Pf:
        return "code-point.final-quote-punctuation";
    case lexy::code_point::Po:
        return "code-point.other-punctuation";

    case lexy::code_point::Sm:
        return "code-point.math-symbol";
    case lexy::code_point::Sc:
        return "code-point.currency-symbol";
    case lexy::code_point::Sk:
        return "code-point.modifier-symbol";
    case lexy::code_point::So:
        return "code-point.other-symbol";

    case lexy::code_point::Zs:
        return "code-point.space-separator";
    case lexy::code_point::Zl:
        return "code-point.line-separator";
    case lexy::code_point::Zp:
        return "code-point.paragraph-separator";

    case lexy::code_point::Cc:
        return "code-point.control";
    case lexy::code_point::Cf:
        return "code-point.format";
    case lexy::code_point::Cs:
        return "code-point.surrogate";
    case lexy::code_point::Co:
        return "code-point.private-use";
    case lexy::code_point::Cn:
        return "code-point.not-assigned";
    }

    return nullptr; // unreachable
}
} // namespace lexy::_detail

#if LEXY_HAS_UNICODE_DATABASE
// AUTOGENERATED FILE --- DO NOT EDIT!
// Generated by `support/generate-unicode-db.py`.

#define LEXY_UNICODE_DATABASE_VERSION "15.0.0"

namespace lexy::_unicode_db
{
constexpr const char* block_starts = "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\021\025\026\027\030\031\032\033\034\035\036\037 !\"#$%&'(!)*+,-./0'\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\0211\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\0212\021\021\0213\021456789\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021:;;;;;;;;<<<<<<<<<<<<<<<<<<<<<<<<<\021=>?@ABCDEFGH\021IJKLMNOPQRSTUVWXYZ[\\]^_`a\021\021\021bcdeeeeeeeeef\021\021\021\021geeeeeeeeeeeeeee\021\021heeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\021\021ijeekl\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021m\021\021\021\021noeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeep\021qreeeeeeeeeseeeeeeeeeeeeeeeeeetuvwxyz{|''}eeee~\177\200\201e\202ee\203\204\205ee\206\207\210e\211\212\213\214''\215\216\217'\220\221eeee\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\222\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\223\224\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\225\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\226eeeeeeeeeeee\021\021\227eeeee\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\230\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\231eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\232\233eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\234"
    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\234";
constexpr const char* blocks = "\000\000\000\000\000\000\000\000\000\001\001\001\001\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\003\003\003\004\003\003\003\005\006\003\007\003\010\003\003\011\011\011\011\011\011\011\011\011\011\003\003\007\007\007\003\003\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\005\003\006\013\014\013\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\005\007\006\007\000\000\000\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\003\004\004\004\004\016\003\013\016\017\020\007\021\016\013\016\007\022\022\013\023\003\024\013\022\017\025\022\022\022\003\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\027\015\026\015\026\015\026\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\030\026\015\026\015\026\015\031\015\032\026\015\026\015\033\026\015\034\034\026\015\015\035\036\037\026\015\034 \015!\"\026\015\015\015!#\015$\026\015\026\015\026\015%\026\015%\015\015\026\015%\026\015&&\026\015\026\015'\026\015\015(\026\015\015\015(((()*\015)*\015)*\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015)*\015\026\015+,\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015-\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\015\015\015\015\015.\026\015/0\015\015\026\015123\026\015\026\015\026\015\026\015\026\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015(\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015444444444555555544\013\013\013\013555555555555\013\013\013\013\013\013\013\013\013\013\013\013\013\01344444\013\013\013\013\013\013\0135\0135\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\0136666666666666666666666666666666666666666666666666666666666666666666667666666666666666666666666666666666666666666\026\015\026\0155\013\026\015889\015\015\015\003:8888\013\013;\024<<<8=8>>\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\0128\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015?\015\015\015\015\015\015\015\015\015\015\015\015@AB\027\027\027CD\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015EF\015\015GH\007\026\015I\026\015\015---"
    "JJJJJJJJJJJJJJJJ\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\01666666KK\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015L\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\0158MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM885\003\003\003\003\003\003\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\003\01088\016\016\00486666666666666666666666666666666NNNNNNNNNNNNNN\010N\003NN\003NN\003N88888888(((((((((((((((((((((((((((8888((((\003\00388888888888\021\021\021\021\021\021\007\007\007\003\003\004\003\003\016\016NNNNNNNNNNN\003\021\003\003\003((((((((((((((((((((((((((((((((5((((((((((NNNNNNNNNNNNN6NNNNNNN\011\011\011\011\011\011\011\011\011\011\003\003\003\003((N(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\003(NNNNNNN\021\01666NNNN55NN\016666N((\011\011\011\011\011\011\011\011\011\011(((\016\016(\003\003\003\003\003\003\003\003\003\003\003\003\003\0038\021(N((((((((((((((((((((((((((((((NNNNNNNNNNNNNNNN6666666666688(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((NNNNNNNNNNN(88888888888888\011\011\011\011\011\011\011\011\011\011(((((((((((((((((((((((((((((((((66666666655\016\003\003\0035886\004\004"
    "((((((((((((((((((((((NN665NNNNNNNNN5NNN5NNNN688\003\003\003\003\003\003\003\003\003\003\003\003\003\003\0038(((((((((((((((((((((((((66688\0038(((((((((((88888((((((((((((((((((((((((\013((((((8\021\02188888866666666(((((((((((((((((((((((((((((((((((((((((56666666666NNNNNNNNNNNN66\021NNNNNNN666666NNNNNNNNNNNNNNNNNNNO((((((((((((((((((((((((((((((((((((((((((((((((((((((NO6(OOONNNNNNNNOOOO6OO(6666NNN((((((((((NN\003\003\011\011\011\011\011\011\011\011\011\011\0035(((((((((((((((NOO8((((((((88((88((((((((((((((((((((((8(((((((8(888((((886(OOONNNN88OO88OO6(88888888O8888((8(((NN88\011\011\011\011\011\011\011\011\011\011((\004\004\022\022\022\022\022\022\016\004(\003688NNO8((((((8888((88((((((((((((((((((((((8(((((((8((8((8((8868OOONN8888NN88NN6888N8888888((((8(8888888\011\011\011\011\011\011\011\011\011\011NN(((N\0038888888888NNO8(((((((((8(((8((((((((((((((((((((((8(((((((8((8(((((886(OOONNNNN8NNO8OO688(888888888888888((NN88\011\011\011\011\011\011\011\011\011\011\003\0048888888(NNN6668NOO8((((((((88((88((((((((((((((((((((((8(((((((8((8(((((886(ONONNNN88OO88OO688888886NO8888((8(((NN88\011\011\011\011\011\011\011\011\011\011\016(\022\022\022\022\022\0228888888888N(8((((((888(((8((((888((8(8((888((888(((888((((((((((((8888OONOO888OOO8OOO688(888888O88888888888888\011\011\011\011\011\011\011\011\011\011\022\022\022\016\016\016\016\016\016\004\01688888"
    "NOOON((((((((8(((8(((((((((((((((((((((((8((((((((((((((((886(NNNOOOO8NNN8NNN68888888NN8(((88(88((NN88\011\011\011\011\011\011\011\011\011\0118888888\003\022\022\022\022\022\022\022\016(NOO\003((((((((8(((8(((((((((((((((((((((((8((((((((((8(((((886(ONOOOOO8NOO8OON68888888OO888888((8((NN88\011\011\011\011\011\011\011\011\011\0118((O888888888888NNOO(((((((((8(((8(((((((((((((((((((((((((((((((((((((((((66(OOONNNN8OOO8OOO6(\0168888(((O\022\022\022\022\022\022\022(((NN88\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\016((((((8NOO8((((((((((((((((((888((((((((((((((((((((((((8(((((((((8(88(((((((88868888OOONNN8N8OOOOOOOO888888\011\011\011\011\011\011\011\011\011\01188OO\003888888888888((((((((((((((((((((((((((((((((((((((((((((((((N(PNNNNNNN8888\004((((((5666666N6\003\011\011\011\011\011\011\011\011\011\011\003\0038888888888888888888888888888888888888((8(8(((((8((((((((((((((((((((((((8(8((((((((((N(PNNNNNN6NN(88(((((85866666N68\011\011\011\011\011\011\011\011\011\01188((((88888888888888888888888888888888(\016\016\016\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\016\003\016\016\01666\016\016\016\016\016\016\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\022\0166\0166\0166\005\006\005\006QQ((((((((8((((((((((((((((((((((((((((((((((((8888NNNNNNNNNNNNNNONNNN6\00366(((((NNNNNNNNNNN8NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN8\016\016\016\016\016\016\016\0166\016\016\016\016\016\0168\016\016\003\003\003\003\003\016\016\016\016\003\0038888888888888888888888888888888888888"
    "(((((((((((((((((((((((((((((((((((((((((((OONNNNONNNNN6O66OONN(\011\011\011\011\011\011\011\011\011\011\003\003\003\003\003\003((((((OONN((((NNN(OOO((OOOOOOO(((NNNN(((((((((((((NOONNOOOOOON(O\011\011\011\011\011\011\011\011\011\011OOON\016\016RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR8R88888R88\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0034\015\015\015(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8((((88(((((((8(8((((88(((((((((((((((((((((((((((((((((((((((((8((((88(((((((((((((((((((((((((((((((((8((((88(((((((8(8((((88(((((((((((((((8(((((((((((((((((((((((((((((((((((((((((((((((((((((((((8((((88(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88666\003\003\003\003\003\003\003\003\003SSSSSSSSS\022\022\022\022\022\022\022\022\022\022\022888((((((((((((((((\016\016\016\016\016\016\016\016\016\016888888\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\02788TTTTTT88"
    "\010((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\016\003(((((((((((((((((\002((((((((((((((((((((((((((\005\006888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\003\003\003UUU((((((((8888888((((((((((((((((((NN6Q888888888(((((((((((((((((((NNQ\003\003888888888((((((((((((((((((NN888888888888(((((((((((((8(((8NN888888888888((((((((((((((((((((((((((((((((((((((((((((((((((((66ONNNNNNNOOOOOOOONOO66666666666\003\003\0035\003\003\003\004(688\011\011\011\011\011\011\011\011\011\011888888\022\022\022\022\022\022\022\022\022\022888888\003\003\003\003\003\003\010\003\003\003\003666\0216\011\011\011\011\011\011\011\011\011\011888888(((((((((((((((((((((((((((((((((((5(((((((((((((((((((((((((((((((((((((((((((((((((((((8888888(((((VV((((((((((((((((((((((((((((((((((N(88888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888888888"
    "(((((((((((((((((((((((((((((((8NNNOOOONNOOO8888OONOOOOOO6668888\016888\003\003\011\011\011\011\011\011\011\011\011\011((((((((((((((((((((((((((((((88(((((88888888888((((((((((((((((((((((((((((((((((((((((((((8888((((((((((((((((((((((((((888888\011\011\011\011\011\011\011\011\011\011S888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016(((((((((((((((((((((((NNOON88\003\003(((((((((((((((((((((((((((((((((((((((((((((((((((((ONONNNNNNN86ONOONNNNNNNNOOOOOONN66666666886\011\011\011\011\011\011\011\011\011\011888888\011\011\011\011\011\011\011\011\011\011888888\003\003\003\003\003\003\0035\003\003\003\003\003\0038866666666666666KNN66666666666NNN8888888888888888888888888888888888888888888888888NNNNO(((((((((((((((((((((((((((((((((((((((((((((((6ONNNNNONOOOOONOQ((((((((888\011\011\011\011\011\011\011\011\011\011\003\003\003\003\003\003\003\016\016\016\016\016\016\016\016\016\016666666666\016\016\016\016\016\016\016\016\016\003\0038NNO((((((((((((((((((((((((((((((ONNNNOONNQ6NN((\011\011\011\011\011\011\011\011\011\011((((((((((((((((((((((((((((((((((((((((((((6ONNOOONONNNQQ88888888\003\003\003\003((((((((((((((((((((((((((((((((((((OOOOOOOONNNNNNNNOON6888\003\003\003\003\003\011\011\011\011\011\011\011\011\011\011888(((\011\011\011\011\011\011\011\011\011\011((((((((((((((((((((((((((((((555555\003\003WXYZZ[\\]^8888888___________________________________________88___\003\003\003\003\003\003\003\00388888888666\0036666666666666Q6666666((((6((((((6((Q66(88888"
    "\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015444444444444444444444444444444444444444444444444444444444444444\015\015\015\015\015\015\015\015\015\015\015\015\0154\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0154444444444444444444444444444444444444666666666666666666666666666666666666666NNNNNNNNNNNNNN66666666666\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\015\015\015\015`\015\015a\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\015\015\015\015\015\015\015bbbbbbbb\015\015\015\015\015\01588bbbbbb88\015\015\015\015\015\015\015\015bbbbbbbb\015\015\015\015\015\015\015\015bbbbbbbb\015\015\015\015\015\01588bbbbbb88\015\015\015\015\015\015\015\0158b8b8b8b\015\015\015\015\015\015\015\015bbbbbbbb\015\015\015\015\015\015\015\015\015\015\015\015\015\01588\015\015\015\015\015\015\015\015cccccccc\015\015\015\015\015\015\015\015cccccccc\015\015\015\015\015\015\015\015cccccccc\015\015\015\015\0158\015\015bbdde\013f\013\013\013\015\015\0158\015\015gggge\013\013\013\015\015\015\01588\015\015bbhh8\013\013\013\015\015\015\015\015\015\015\015bbiiI\013\013\01388\015\015\0158\015\015jjkke\013\0138\002\002\002\002\002\002\002\002\002\002\002\021ll\021\021\010\010\010\010\010\010\003\003\020\025\005\020\020\025\005\020\003\003\003\003\003\003\003\003mn\021\021\021\021\021\002\003\003\003\003\003\003\003\003\003\020\025\003\003\003\003\014\014\003\003\003\007\005\006\003\003\003\003\003\003\003\003\003\003\003\007\003\014\003\003\003\003\003\003\003\003\003\003\002\021\021\021\021\0218\021\021\021\021\021\021\021\021\021\021\022488\022\022\022\022\022\022\007\007\007\005\0064\022\022\022\022\022\022\022\022\022\022\007\007\007\005\00684444444444444888\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\0048888888888888886666666666666KKKK6KKK666666666666888888888888888"
    "\016\016\027\016\016\016\016\027\016\016\015\027\027\027\015\015\027\027\027\015\016\027\016\016o\027\027\027\027\027\016\016\016\016\016\016\027\016p\016\027\016qr\027\027s\015\027\027t\027\015((((\015\016\016\015\015\027\027\007\007\007\007\007\027\015\015\015\015\016\007\016\016\015\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022uuuuuuuuuuuuuuuuvvvvvvvvvvvvvvvvUUU\026\015UUUU\022\016\0168888\007\007\007\007\007\016\016\016\016\016\007\007\016\016\016\016\007\016\016\007\016\016\007\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\016\016\007\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\005\006\005\006\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\016\016\016\016\016\016\016\005\006\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888888888888888888888888\016\016\016\016\016\016\016\016\016\016\016888888888888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016wwwwwwwwwwwwwwwwwwwwwwwwwwxxxxxxxxxxxxxxxxxxxxxxxxxx\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"
    "\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\005\006\005\006\005\006\005\006\005\006\005\006\005\006\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\005\006\005\006\005\006\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016"
    "\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\026\015yz{\015\015\026\015\026\015\026\015|}~\177\015\026\015\015\026\015\015\015\015\015\01544\200\200\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\016\016\016\016\016\016\026\015\026\015666\026\01588888\003\003\003\003\022\003\003\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0158\01588888\01588((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888885\003888888888888886(((((((((((((((((((((((888888888(((((((8(((((((8(((((((8(((((((8(((((((8(((((((8(((((((8(((((((8NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN"
    "\003\003\020\025\020\025\003\003\003\020\025\003\020\025\003\003\003\003\003\003\003\003\003\010\003\003\010\003\020\025\003\003\020\025\005\006\005\006\005\006\005\006\003\003\003\003\003\201\003\003\003\003\003\003\003\003\003\003\010\010\003\003\003\003\010\003\005\003\003\003\003\003\003\003\003\003\003\003\003\003\016\016\003\003\003\005\006\005\006\005\006\005\006\0108888888888888888888888888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688888888888888888888888888\016\016\016\016\016\016\016\016\016\016\016\0168888\002\003\003\003\0165(U\005\006\005\006\005\006\005\006\005\006\016\016\005\006\005\006\005\006\005\006\010\005\006\006\016UUUUUUUUU6666QQ\01055555\016\016UUU5(\003\016\0168((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8866\013\01355(\010((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\003555(88888(((((((((((((((((((((((((((((((((((((((((((8((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8\016\016\022\022\022\022\016\016\016\016\016\016\016\016\016\016((((((((((((((((((((((((((((((((\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888888(((((((((((((((("
    "\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\022\022\022\022\022\022\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016(((((((((((((((((((((5(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888((((((((((((((((((((((((((((((((((((((((555555\003\003"
    "((((((((((((5\003\003\003((((((((((((((((\011\011\011\011\011\011\011\011\011\011((88888888888888888888\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015(6KKK\003NNNNNNNN66\0035\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\01544NN((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((UUUUUUUUUU66\003\003\003\003\003\00388888888\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013555555555\013\013\026\015\026\015\026\015\026\015\026\015\026\015\026\015\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\0154\015\015\015\015\015\015\015\015\026\015\026\015\202\026\015\026\015\026\015\026\015\026\0155\013\013\026\015\203\015(\026\015\026\015\015\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\204\205\206\207\204\015\210\211\212\213\026\015\026\015\026\015\026\015\026\015\026\015\026\015\026\015\214\215\216\026\015\026\01588888\026\0158\0158\015\026\015\026\015888888888888888888888888444\026\015(44\015(((((((N(((6((((N(((((((((((((((((((((((OONNO\016\016\016\0166888\022\022\022\022\022\022\016\016\004\016888888((((((((((((((((((((((((((((((((((((((((((((((((((((\003\003\003\00388888888OO((((((((((((((((((((((((((((((((((((((((((((((((((OOOOOOOOOOOOOOOO6N88888888\003\003\011\011\011\011\011\011\011\011\011\011888888666666666666666666((((((\003\003\003(\003((N\011\011\011\011\011\011\011\011\011\011((((((((((((((((((((((((((((NNNNN666\003\003(((((((((((((((((((((((NNNNNNNNNNNOQ88888888888\003(((((((((((((((((((((((((((((888NNNO(((((((((((((((((((((((((((((((((((((((((((((((6OONNNNOONNOOQ\003\003\003\003\003\003\003\003\003\003\003\003\00385\011\011\011\011\011\011\011\011\011\0118888\003\003(((((N5(((((((((\011\011\011\011\011\011\011\011\011\011(((((8"
    "(((((((((((((((((((((((((((((((((((((((((NNNNNNOONNOONN888888888(((N((((((((NO88\011\011\011\011\011\011\011\011\011\01188\003\003\003\003((((((((((((((((5((((((\016\016\016(ONO((((((((((((((((((((((((((((((((((((((((((((((((((N(NNN((NN(((((N6(6(888888888888888888888888((5\003\003(((((((((((ONNOO\003\003(55O68888888888((((((88((((((88((((((888888888(((((((8(((((((8\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0134444\015\015\015\015\015\015\015\015\0154\013\0138888\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217\217(((((((((((((((((((((((((((((((((((OONOONOO\003Q688\011\011\011\011\011\011\011\011\011\011888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888888888888(((((((((((((((((((((((8888(((((((((((((((((((((((((((((((((((((((((((((((((8888\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220"
    "\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888888888888888888888888888888888888\015\015\015\015\015\015\015888888888888\015\015\015\015\01588888(N((((((((((\007(((((((((((((8(((((8(8((8((8((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\0138888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\222\222\222\222\222\222(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
    "((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\006\005\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88((((((((((((((((((((((((((((((((((((((((((((((((((((((8888888\01688888888888888888888888888888888((((((((((\222\222\004\016\016\0166666666666666666\003\003\003\003\003\003\003\005\006\0038888886666666666666666\003\010\010\014\014\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\003\003\005\006\003\003\003\003\014\014\014\003\003\0038\003\003\003\003\010\005\006\005\006\005\006\003\003\003\007\010\007\007\0078\003\004\003\0038888\222(\222(\2228\222(\222(\222(\222(\222((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88\0218\003\003\003\004\003\003\003\005\006\003\007\003\010\003\003\011\011\011\011\011\011\011\011\011\011\003\003\007\007\007\003\003\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\005\003\006\013\014\013\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\005\007\006\007\005\006\003\005\006\003\003((((((((((5(((((((((((((((((((((((((((((((((((((((((((((\223\223(((((((((((((((((((((((((((((((888((((((88((((((88((((((88(((888\004\004\007\013\016\004\0048\016\007\007\007\007\016\0168888888888\021\021\021\016\01688((((((((((((8((((((((((((((((((((((((((8(((((((((((((((((((8((8(((((((((((((((88((((((((((((((8888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888"
    "\003\003\0038888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022888\016\016\016\016\016\016\016\016\016UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\016\016\0168\016\016\016\016\016\016\016\016\016\016\016\016\016888\01688888888888888888888888888888888888888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01668888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((888(((((((((((((((((((((((((((((((((((((((((((((((((8888888888888886\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\0228888((((((((((((((((((((((((((((((((\022\022\022\022888888888((((((((((((((((((((U((((((((U88888((((((((((((((((((((((((((((((((((((((NNNNN88888((((((((((((((((((((((((((((((8\003((((((((((((((((((((((((((((((((((((8888((((((((\003UUUUU888888888888888888888888888888888888888888\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88\011\011\011\011\011\011\011\011\011\011888888\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\224\2248888\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0158888"
    "((((((((((((((((((((((((((((((((((((((((88888888((((((((((((((((((((((((((((((((((((((((((((((((((((88888888888\003\225\225\225\225\225\225\225\225\225\225\2258\225\225\225\225\225\225\225\225\225\225\225\225\225\225\2258\225\225\225\225\225\225\2258\225\2258\015\015\015\015\015\015\015\015\015\015\0158\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0158\015\015\015\015\015\015\0158\015\0158888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((888888888((((((((((((((((((((((8888888888((((((((88888888888888888888888845544484444444444444444444444444444444444444444448444444444888888888888888888888888888888888888888888888888888888888888888888888((((((88(8((((((((((((((((((((((((((((((((((((((((((((8((888(88(((((((((((((((((((((((8\003\022\022\022\022\022\022\022\022(((((((((((((((((((((((\016\016\022\022\022\022\022\022\022(((((((((((((((((((((((((((((((88888888\022\022\022\022\022\022\022\022\022888888888888888888888888888888888888888888888888(((((((((((((((((((8((88888\022\022\022\022\022((((((((((((((((((((((\022\022\022\022\022\022888\003((((((((((((((((((((((((((88888\0038888888888888888888888888888888888888888888888888888888888888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888\022\022((\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\02288\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"
    "(NNN8NN88888NNNN((((8(((8(((((((((((((((((((((((((((((8866688886\022\022\022\022\022\022\022\022\0228888888\003\003\003\003\003\003\003\003\0038888888(((((((((((((((((((((((((((((\022\022\003(((((((((((((((((((((((((((((\022\022\02288888888888888888888888888888888((((((((\016((((((((((((((((((((((((((((668888\022\022\022\022\022\003\003\003\003\003\003\003888888888((((((((((((((((((((((((((((((((((((((((((((((((((((((888\003\003\003\003\003\003\003((((((((((((((((((((((88\022\022\022\022\022\022\022\022(((((((((((((((((((88888\022\022\022\022\022\022\022\022((((((((((((((((((8888888\003\003\003\003888888888888\022\022\022\022\022\022\02288888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888888888888888888888888888888888888888888888888888888===================================================8888888888888\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0158888888\022\022\022\022\022\022((((((((((((((((((((((((((((((((((((NNNN88888888\011\011\011\011\011\011\011\011\011\011888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\0228((((((((((((((((((((((((((((((((((((((((((8NN\01088((888888888888888888888888888888888888888888888888888888888888888888888888888666(((((((((((((((((((((((((((((\022\022\022\022\022\022\022\022\022\022(88888888((((((((((((((((((((((66666666666\022\022\022\022\003\003\003\003\0038888888888888888888888((((((((((((((((((6666\003\003\003\00388888888888888888888888888888888888888(((((((((((((((((((((\022\022\022\022\022\022\02288888888888888888888(((((((((((((((((((((((888888888ONO(((((((((((((((((((((((((((((((((((((((((((((((((((((NNNNNNNNNNNNNN6\003\003\003\003\003\003\0038888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\011\011\011\011\011\011\011\011\011\0116((NN(8888888886NNO(((((((((((((((((((((((((((((((((((((((((((((OOONNNNOO66\003\003\021\003\003\003\003N8888888888\02188(((((((((((((((((((((((((8888888\011\011\011\011\011\011\011\011\011\011888888NNN((((((((((((((((((((((((((((((((((((NNNNNONNNNNN668\011\011\011\011\011\011\011\011\011\011\003\003\003\003(OO(88888888(((((((((((((((((((((((((((((((((((6\003\003(888888888NNO((((((((((((((((((((((((((((((((((((((((((((((((OOONNNNNNNNNOQ((((\003\003\003\0036666\003ON\011\011\011\011\011\011\011\011\011\011(\003(\003\003\0038\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\02288888888888"
    "((((((((((((((((((8(((((((((((((((((((((((((OOONNNOONQ6N\003\003\003\003\003\003N((N88888888888888888888888888888888888888888888888888888888888888(((((((8(8((((8(((((((((((((((8((((((((((\003888888(((((((((((((((((((((((((((((((((((((((((((((((NOOONNNNNN6688888\011\011\011\011\011\011\011\011\011\011888888NNOO8((((((((88((88((((((((((((((((((((((8(((((((8((8(((((866(OONOOOO88OO88OOQ88(888888O88888(((((OO886666666888666668888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((OOONNNNNNNNOO6NNO6((((\003\003\003\003\003\011\011\011\011\011\011\011\011\011\011\003\0038\0036(((888888888888888888888888888888((((((((((((((((((((((((((((((((((((((((((((((((OOONNNNNNONOOOONNO66((\003(88888888\011\011\011\011\011\011\011\011\011\0118888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((OOONNNN88OOOONNO66\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003((((NN8888888888888888888888888888888888"
    "((((((((((((((((((((((((((((((((((((((((((((((((OOONNNNNNNNOONO6N\003\003\003(88888888888\011\011\011\011\011\011\011\011\011\011888888\003\003\003\003\003\003\003\003\003\003\003\003\0038888888888888888888(((((((((((((((((((((((((((((((((((((((((((NONOONNNNNNQ6(\003888888\011\011\011\011\011\011\011\011\011\011888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((88NNNOONNNNONNNN68888\011\011\011\011\011\011\011\011\011\011\022\022\003\003\003\016(((((((88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888((((((((((((((((((((((((((((((((((((((((((((OOONNNNNNNNNO66\0038888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022888888888888((((((((88(88((((((((8((8((((((((((((((((((((((((OOOOOO8OO88NNQ6(O(O6\003\003\003888888888\011\011\011\011\011\011\011\011\011\0118888888888888888888888888888888888888888888888888888888888888888888888((((((((88(((((((((((((((((((((((((((((((((((((((OOONNNN88NNOOOO6(\003(O888888888888888888888888888"
    "(NNNNNNNNNN((((((((((((((((((((((((((((((((((((((((66NNNNO(NNNN\003\003\003\003\003\003\003\003688888888(NNNNNNOONNN((((((((((((((((((((((((((((((((((((((((((((((NNNNNNNNNNNNNO66\003\003\003(\003\003\003\003\0038888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888888\003\003\003\003\003\003\003\003\003\003888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((8(((((((((((((((((((((((((((((((((((((ONNNNNNN8NNNNNNO6(\003\003\003\003\0038888888888\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022888\003\003((((((((((((((((((((((((((((((88NNNNNNNNNNNNNNNNNNNNNN8ONNNNNNNONNONN8888888888888888888888888888888888888888888888888888888888888888888888888(((((((8((8((((((((((((((((((((((((((((((((((((((NNNNNN888N8NN8NNN6N66(N88888888\011\011\011\011\011\011\011\011\011\011888888((((((8((8((((((((((((((((((((((((((((((((OOOOO8NN8OONO6(8888888\011\011\011\011\011\011\011\011\011\01188888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((NNOO\003\0038888888NN(O(((((((((((((8((((((((((((((((((((((((((((((((((OONNNNN888OONQ6\003\003\003\003\003\003\003\003\003\003\003\003\003\011\011\011\011\011\011\011\011\011\01188888888888888888888888888888888888888888888888888888888888888888888888888888888888888(888888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\004\004\004\004\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888888888888\003((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU8\003\003\003\003\00388888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
    "((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\003\0038888888888888((((((((((((((((((((((((((((((((((((((((((((((((\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\0216((((((66666666666666688888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888888(((((((((((((((((((((((((((((((8\011\011\011\011\011\011\011\011\011\0118888\003\003(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8\011\011\011\011\011\011\011\011\011\011888888((((((((((((((((((((((((((((((8866666\0038888888888((((((((((((((((((((((((((((((((((((((((((((((((6666666\003\003\003\003\003\016\016\016\0165555\003\0168888888888\011\011\011\011\011\011\011\011\011\0118\022\022\022\022\022\022\0228(((((((((((((((((((((88888(((((((((((((((((((88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\003\003\003\00388888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888N(OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO8888888NNNN5555555555555888888888888888888888888888888888888888888888888888888888888888855\0035688888888888OO88888888888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888888888888888888888888888888888888888888(((((((((8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888885555855555558558(((((((((((((((((((((((((((((((((((888888888888888(88888888888888888888888888888(((88(88888888888888((((88888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888(((((((((((((888(((((((((8888888((((((((((88\0166N\003\021\021\021\02188888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "66666666666666666666666666666666666666666666668866666666666666666666666888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888888888888888888888888888888888888888888888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016QQ666\016\016\016QQQQQQ\021\021\021\021\021\021\021\02166666666\016\0166666666\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0166666\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016666\01688888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022888888888888"
    "\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\0158\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\0278\027\02788\02788\027\02788\027\027\027\0278\027\027\027\027\027\027\027\027\015\015\015\0158\0158\015\015\015\015\015\015\0158\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\0278\027\027\027\02788\027\027\027\027\027\027\027\0278\027\027\027\027\027\027\0278\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\0278\027\027\027\0278\027\027\027\027\0278\027888\027\027\027\027\027\027\0278\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\01588\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\007\015\015\015\015"
    "\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\027\01588\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\0116666666666666666666666666666666666666666666666666666666\016\016\016\01666666666666666666666666666666666666666666666666666\016\016\016\016\016\016\016\0166\016\016\016\016\016\016\016\016\016\016\016\016\016\0166\016\016\003\003\003\003\00388888888888888866666866666666666666688888888888888888888888888888888888888888888888888888888888888888888888888888888\015\015\015\015\015\015\015\015\015\015(\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015888888\015\015\015\015\015\015888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888NNNNNNN8NNNNNNNNNNNNNNNNN88NNNNNNN8NN8NNNNN8888844444444444444444444444444444444444444444444444444444444444444888888888888888888888888888888888N8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "(((((((((((((((((((((((((((((((((((((((((((((8886666666555555588\011\011\011\011\011\011\011\011\011\0118888(\01688888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888((((((((((((((((((((((((((((((688888888888888888((((((((((((((((((((((((((((((((((((((((((((6666\011\011\011\011\011\011\011\011\011\01188888\0048888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((((((((((((((((((((((56666\011\011\011\011\011\011\011\011\011\01188888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888(((((((8((((8((8(((((((((((((((8"
    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88\022\022\022\022\022\022\022\022\022666666688888888888888888888888888888888888888888\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\226\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015666N66658888\011\011\011\011\011\011\011\011\011\0118888\003\003888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\022\022\022\004\022\022\022\0228888888888888888888888888888888888888888888888888888888888888888888888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\02288888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "((((8(((((((((((((((((((((((((((8((8(88(8((((((((((8((((8(8(888888(8888(8(8(8(((8((8(88(8(8(8(8(8((8(88((((8(((((((8((((8((((8(8((((((((((8(((((((((((((((((88888(((8(((((8(((((((((((((((((8888888888888888888888888888888888888888888888888888\007\00788888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888888888\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\016\016\016\016\016\016\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\016\016\016\016\016\016\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\227\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688888888888888888888888888888888888888888888888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888\016\016\016\016\016\016\016\016\0168888888\016\01688888888888888\016\016\016\016\016\0168888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\013\013\013\013\013\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888\016\016\016\016\016\016\016\016\016\016\016\016\016888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888\016\016\016\016\016\016\016\016\016\016\016\0168888\016888888888888888\016\016\016\016\016\016\016\016\016\016\016\0168888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688888888\016\016\016\016\016\016\016\016\016\016888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\01688\016\016888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016888888888888\016\016\016\016\016\016\016\016\016\016\016\016\016\01688\016\016\016\016\016\016\016\016\016\016\016\016\016888\016\016\016\016\016\016\016\016\0168888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\016\016\016\016\016\016\01688888888\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888\016\016\016\016\016\016\016\016\0168888888\016\016\016\016\016\016\016\016\0168888888\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\0168888888888888888888888888888888888888\011\011\011\011\011\011\011\011\011\011888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888888888888888888888888888888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
    "((((((((((((((((((((((((((((((88((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888888888888(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((8888888888888888888888888888888((((((((((((((((((((((((((((((8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"
    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((88888((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((888888888888888888888888888888888888888888888888888888888888888888888888888888888\021888888888888888888888888888888\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888886666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666668888888888888888"
    "\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\221\22188";

constexpr std::size_t property_index(char32_t code_point)
{
    LEXY_PRECONDITION(code_point <= 0x10FFFF);
    auto high = (code_point >> 8) & 0xFFFF;
    auto low  = code_point & 0xFF;

    auto block_start = static_cast<unsigned char>(block_starts[high]);
    auto block_index = block_start * 256u + low;
    return static_cast<unsigned char>(blocks[block_index]);
}

constexpr lexy::code_point::general_category_t category[] = {lexy::code_point::Cc,lexy::code_point::Cc,lexy::code_point::Zs,lexy::code_point::Po,lexy::code_point::Sc,lexy::code_point::Ps,lexy::code_point::Pe,lexy::code_point::Sm,lexy::code_point::Pd,lexy::code_point::Nd,lexy::code_point::Lu,lexy::code_point::Sk,lexy::code_point::Pc,lexy::code_point::Ll,lexy::code_point::So,lexy::code_point::Lo,lexy::code_point::Pi,lexy::code_point::Cf,lexy::code_point::No,lexy::code_point::Ll,lexy::code_point::Po,lexy::code_point::Pf,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lo,lexy::code_point::Lu,lexy::code_point::Lt,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lm,lexy::code_point::Lm,lexy::code_point::Mn,lexy::code_point::Mn,lexy::code_point::Cn,lexy::code_point::Lm,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Me,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Mn,lexy::code_point::Mc,lexy::code_point::Lo,lexy::code_point::Mc,lexy::code_point::Lu,lexy::code_point::No,lexy::code_point::Ll,lexy::code_point::Nl,lexy::code_point::Mn,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lt,lexy::code_point::Lu,lexy::code_point::Lt,lexy::code_point::Ll,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Cf,lexy::code_point::Zl,lexy::code_point::Zp,lexy::code_point::Sm,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::So,lexy::code_point::Lu,lexy::code_point::Nl,lexy::code_point::Nl,lexy::code_point::So,lexy::code_point::So,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lm,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Ll,lexy::code_point::Cs,lexy::code_point::Co,lexy::code_point::Lo,lexy::code_point::Lm,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::Lu,lexy::code_point::So,};

enum binary_properties_t
{
    whitespace,
    join_control,
    alphabetic,
    uppercase,
    lowercase,
    xid_start,
    xid_continue,
    _property_count,
};
static_assert(static_cast<int>(_property_count) <= 8);

constexpr std::uint_least8_t binary_properties[] = {0,1,1,0,0,0,0,0,0,64,108,0,64,116,0,116,0,0,0,116,64,0,108,108,108,116,108,108,108,108,108,108,108,108,108,108,108,108,108,108,100,108,100,108,108,108,108,108,108,108,108,108,116,100,64,84,0,20,108,108,108,108,108,116,108,116,116,116,116,116,116,108,116,108,108,0,108,108,68,68,68,64,108,64,116,100,100,116,116,116,116,116,116,116,116,108,116,108,108,100,108,100,116,108,108,108,108,108,2,1,1,96,108,108,108,96,108,108,116,12,20,108,108,108,108,108,108,108,108,4,108,108,108,108,108,108,108,108,108,108,108,108,108,116,0,0,4,68,108,108,108,12,};

constexpr std::int_least32_t case_folding_offset[] = {0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,0,0,0,775,0,0,1,0,-121,-268,210,206,205,79,202,203,207,211,209,213,214,218,217,219,0,2,1,-97,-56,-130,10795,-163,10792,-195,69,71,0,0,0,116,0,0,116,38,37,64,63,1,8,-30,-25,-15,-22,-54,-48,-60,-64,-7,80,0,15,48,0,0,0,0,7264,0,-8,0,0,-6222,-6221,-6212,-6210,-6211,-6204,-6180,35267,-3008,-58,-7615,-8,-8,-74,-9,-7173,-86,-100,-112,-128,-126,0,0,0,0,-7517,-8383,-8262,0,28,16,0,26,0,-10743,-3814,-10727,-10780,-10749,-10783,-10782,-10815,0,-35332,-42280,-42308,-42319,-42315,-42305,-42258,-42282,-42261,928,-48,-42307,-35384,-38864,0,0,0,0,40,39,34,0,};
} // namespace lexy::_unicode_db

constexpr lexy::code_point::general_category_t lexy::code_point::general_category() const noexcept
{
    if (!is_valid())
        return general_category_t::unassigned;

    auto idx = _unicode_db::property_index(_value);
    return _unicode_db::category[idx];
}

constexpr lexy::code_point lexy::simple_case_fold(code_point cp) noexcept
{
    if (!cp.is_valid())
        return cp;

    auto idx    = _unicode_db::property_index(cp.value());
    auto offset = _unicode_db::case_folding_offset[idx];
    return code_point(char32_t(std::int_least32_t(cp.value()) + offset));
}

namespace lexy::_detail
{
template <lexy::_unicode_db::binary_properties_t... Props>
LEXY_FORCE_INLINE constexpr bool code_point_has_properties(char32_t cp)
{
    constexpr auto mask = ((1 << Props) | ...);

    auto idx   = _unicode_db::property_index(cp);
    auto props = _unicode_db::binary_properties[idx];
    return (props & mask) != 0;
}
} // namespace lexy::_detail

#    define LEXY_UNICODE_PROPERTY(Name) ::lexy::_unicode_db::Name

#else
namespace lexy::_detail
{
template <int... Props>
bool code_point_has_properties(char32_t cp); // not implemented
} // namespace lexy::_detail

#    define LEXY_UNICODE_PROPERTY(Name) 0

#endif

#endif // LEXY_CODE_POINT_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CHAR_CLASS_HPP_INCLUDED
#define LEXY_DSL_CHAR_CLASS_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_CODE_POINT_HPP_INCLUDED
#define LEXY_DETAIL_CODE_POINT_HPP_INCLUDED



//=== encoding ===//
namespace lexy::_detail
{
template <typename Encoding>
constexpr std::size_t encode_code_point(char32_t cp, typename Encoding::char_type* buffer,
                                        std::size_t size)
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
    {
        LEXY_PRECONDITION(size >= 1);

        *buffer = char(cp);
        return 1;
    }
    else if constexpr (std::is_same_v<Encoding,
                                      lexy::utf8_encoding> //
                       || std::is_same_v<Encoding, lexy::utf8_char_encoding>)
    {
        using char_type = typename Encoding::char_type;
        // Taken from http://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html.
        if (cp <= 0x7F)
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char_type(cp);
            return 1;
        }
        else if (cp <= 0x07'FF)
        {
            LEXY_PRECONDITION(size >= 2);

            auto first  = (cp >> 6) & 0x1F;
            auto second = (cp >> 0) & 0x3F;

            buffer[0] = char_type(0xC0 | first);
            buffer[1] = char_type(0x80 | second);
            return 2;
        }
        else if (cp <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 3);

            auto first  = (cp >> 12) & 0x0F;
            auto second = (cp >> 6) & 0x3F;
            auto third  = (cp >> 0) & 0x3F;

            buffer[0] = char_type(0xE0 | first);
            buffer[1] = char_type(0x80 | second);
            buffer[2] = char_type(0x80 | third);
            return 3;
        }
        else
        {
            LEXY_PRECONDITION(size >= 4);

            auto first  = (cp >> 18) & 0x07;
            auto second = (cp >> 12) & 0x3F;
            auto third  = (cp >> 6) & 0x3F;
            auto fourth = (cp >> 0) & 0x3F;

            buffer[0] = char_type(0xF0 | first);
            buffer[1] = char_type(0x80 | second);
            buffer[2] = char_type(0x80 | third);
            buffer[3] = char_type(0x80 | fourth);
            return 4;
        }
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
    {
        if (cp <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char16_t(cp);
            return 1;
        }
        else
        {
            // Algorithm implemented from
            // https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF.
            LEXY_PRECONDITION(size >= 2);

            auto u_prime       = cp - 0x1'0000;
            auto high_ten_bits = u_prime >> 10;
            auto low_ten_bits  = u_prime & 0b0000'0011'1111'1111;

            buffer[0] = char16_t(0xD800 + high_ten_bits);
            buffer[1] = char16_t(0xDC00 + low_ten_bits);
            return 2;
        }
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf32_encoding>)
    {
        LEXY_PRECONDITION(size >= 1);

        *buffer = cp;
        return 1;
    }
    else
    {
        static_assert(lexy::_detail::error<Encoding>,
                      "cannot encode a code point in this encoding");
        (void)cp;
        (void)buffer;
        (void)size;
        return 0;
    }
}
} // namespace lexy::_detail

//=== parsing ===//
namespace lexy::_detail
{
enum class cp_error
{
    success,
    eof,
    leads_with_trailing,
    missing_trailing,
    surrogate,
    overlong_sequence,
    out_of_range,
};

template <typename Reader>
struct cp_result
{
    char32_t                cp;
    cp_error                error;
    typename Reader::marker end;
};

template <typename Reader>
constexpr cp_result<Reader> parse_code_point(Reader reader)
{
    if constexpr (std::is_same_v<typename Reader::encoding, lexy::ascii_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.current()};

        auto cur = reader.peek();
        reader.bump();

        auto cp = static_cast<char32_t>(cur);
        if (cp <= 0x7F)
            return {cp, cp_error::success, reader.current()};
        else
            return {cp, cp_error::out_of_range, reader.current()};
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf8_encoding> //
                       || std::is_same_v<typename Reader::encoding, lexy::utf8_char_encoding>)
    {
        using uchar_t                = unsigned char;
        constexpr auto payload_lead1 = 0b0111'1111;
        constexpr auto payload_lead2 = 0b0001'1111;
        constexpr auto payload_lead3 = 0b0000'1111;
        constexpr auto payload_lead4 = 0b0000'0111;
        constexpr auto payload_cont  = 0b0011'1111;

        constexpr auto pattern_lead1 = 0b0 << 7;
        constexpr auto pattern_lead2 = 0b110 << 5;
        constexpr auto pattern_lead3 = 0b1110 << 4;
        constexpr auto pattern_lead4 = 0b11110 << 3;
        constexpr auto pattern_cont  = 0b10 << 6;

        auto first = uchar_t(reader.peek());
        if ((first & ~payload_lead1) == pattern_lead1)
        {
            // ASCII character.
            reader.bump();
            return {first, cp_error::success, reader.current()};
        }
        else if ((first & ~payload_cont) == pattern_cont)
        {
            return {{}, cp_error::leads_with_trailing, reader.current()};
        }
        else if ((first & ~payload_lead2) == pattern_lead2)
        {
            reader.bump();

            auto second = uchar_t(reader.peek());
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto result = char32_t(first & payload_lead2);
            result <<= 6;
            result |= char32_t(second & payload_cont);

            // C0 and C1 are overlong ASCII.
            if (first == 0xC0 || first == 0xC1)
                return {result, cp_error::overlong_sequence, reader.current()};
            else
                return {result, cp_error::success, reader.current()};
        }
        else if ((first & ~payload_lead3) == pattern_lead3)
        {
            reader.bump();

            auto second = uchar_t(reader.peek());
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto third = uchar_t(reader.peek());
            if ((third & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto result = char32_t(first & payload_lead3);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);

            auto cp = result;
            if (0xD800 <= cp && cp <= 0xDFFF)
                return {cp, cp_error::surrogate, reader.current()};
            else if (first == 0xE0 && second < 0xA0)
                return {cp, cp_error::overlong_sequence, reader.current()};
            else
                return {cp, cp_error::success, reader.current()};
        }
        else if ((first & ~payload_lead4) == pattern_lead4)
        {
            reader.bump();

            auto second = uchar_t(reader.peek());
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto third = uchar_t(reader.peek());
            if ((third & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto fourth = uchar_t(reader.peek());
            if ((fourth & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto result = char32_t(first & payload_lead4);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);
            result <<= 6;
            result |= char32_t(fourth & payload_cont);

            auto cp = result;
            if (cp > 0x10'FFFF)
                return {cp, cp_error::out_of_range, reader.current()};
            else if (first == 0xF0 && second < 0x90)
                return {cp, cp_error::overlong_sequence, reader.current()};
            else
                return {cp, cp_error::success, reader.current()};
        }
        else // FE or FF
        {
            return {{}, cp_error::eof, reader.current()};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf16_encoding>)
    {
        constexpr auto payload1 = 0b0000'0011'1111'1111;
        constexpr auto payload2 = payload1;

        constexpr auto pattern1 = 0b110110 << 10;
        constexpr auto pattern2 = 0b110111 << 10;

        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.current()};

        auto first = char16_t(reader.peek());
        if ((first & ~payload1) == pattern1)
        {
            reader.bump();
            if (reader.peek() == Reader::encoding::eof())
                return {{}, cp_error::missing_trailing, reader.current()};

            auto second = char16_t(reader.peek());
            if ((second & ~payload2) != pattern2)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            // We've got a valid code point.
            auto result = char32_t(first & payload1);
            result <<= 10;
            result |= char32_t(second & payload2);
            result |= 0x10000;
            return {result, cp_error::success, reader.current()};
        }
        else if ((first & ~payload2) == pattern2)
        {
            return {{}, cp_error::leads_with_trailing, reader.current()};
        }
        else
        {
            // Single code unit code point; always valid.
            reader.bump();
            return {first, cp_error::success, reader.current()};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf32_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.current()};

        auto cur = reader.peek();
        reader.bump();

        auto cp = cur;
        if (cp > 0x10'FFFF)
            return {cp, cp_error::out_of_range, reader.current()};
        else if (0xD800 <= cp && cp <= 0xDFFF)
            return {cp, cp_error::surrogate, reader.current()};
        else
            return {cp, cp_error::success, reader.current()};
    }
    else
    {
        static_assert(lexy::_detail::error<typename Reader::encoding>,
                      "no known code point for this encoding");
        return {};
    }
}

template <typename Reader>
constexpr void recover_code_point(Reader& reader, cp_result<Reader> result)
{
    switch (result.error)
    {
    case cp_error::success:
        // Consume the entire code point.
        reader.reset(result.end);
        break;
    case cp_error::eof:
        // We don't need to do anything to "recover" from EOF.
        break;

    case cp_error::leads_with_trailing:
        // Invalid code unit, consume to recover.
        LEXY_PRECONDITION(result.end.position() == reader.position());
        reader.bump();
        break;

    case cp_error::missing_trailing:
    case cp_error::surrogate:
    case cp_error::out_of_range:
    case cp_error::overlong_sequence:
        // Consume all the invalid code units to recover.
        reader.reset(result.end);
        break;
    }
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_CODE_POINT_HPP_INCLUDED





namespace lexy::_detail
{
struct ascii_set
{
    bool contains[128];

    constexpr ascii_set() : contains{} {}

    template <typename Fn>
    constexpr void visit(Fn fn) const
    {
        for (auto i = 0; i != 128; ++i)
            if (contains[i])
                fn(i);
    }

    template <typename Fn>
    constexpr void visit_range(Fn fn) const
    {
        auto range_begin = -1;
        auto last_char   = range_begin;
        visit([&](int c) {
            if (range_begin == -1)
            {
                range_begin = c;
                last_char   = c;
            }
            else if (last_char + 1 == c)
            {
                last_char = c;
            }
            else
            {
                fn(range_begin, last_char);
                range_begin = c;
                last_char   = c;
            }
        });
        if (range_begin != -1)
            fn(range_begin, last_char);
    }

    constexpr void insert(int c)
    {
        contains[c] = true;
    }
    constexpr void insert(int lower, int upper)
    {
        for (auto i = lower; i <= upper; ++i)
            contains[i] = true;
    }
    constexpr void insert(const ascii_set& other)
    {
        other.visit([&](int c) { contains[c] = true; });
    }
    constexpr void remove(const ascii_set& other)
    {
        other.visit([&](int c) { contains[c] = false; });
    }
};

template <std::size_t RangeCount, std::size_t SingleCount>
struct compressed_ascii_set
{
    char range_lower[RangeCount == 0 ? 1 : RangeCount];
    char range_upper[RangeCount == 0 ? 1 : RangeCount];
    char singles[SingleCount == 0 ? 1 : SingleCount];

    static constexpr std::size_t range_count()
    {
        return RangeCount;
    }
    static constexpr std::size_t single_count()
    {
        return SingleCount;
    }
};

template <typename T>
constexpr auto compress_ascii_set()
{
    constexpr auto set = T::char_class_ascii();

    constexpr auto count = [&set] {
        struct result_t
        {
            std::size_t range_count;
            std::size_t single_count;
        } result{0, 0};

        set.visit_range([&](int lower, int upper) {
            if (lower != upper)
                ++result.range_count;
            else
                ++result.single_count;
        });

        return result;
    }();

    compressed_ascii_set<count.range_count, count.single_count> result{};

    auto cur_range  = 0u;
    auto cur_single = 0u;
    set.visit_range([&](int lower, int upper) {
        if (lower != upper)
        {
            result.range_lower[cur_range] = char(lower);
            result.range_upper[cur_range] = char(upper);
            ++cur_range;
        }
        else
        {
            result.singles[cur_single] = char(lower);
            ++cur_single;
        }
    });

    return result;
}
} // namespace lexy::_detail

namespace lexy::_detail
{
template <const auto& CompressedAsciiSet,
          typename = make_index_sequence<CompressedAsciiSet.range_count()>,
          typename = make_index_sequence<CompressedAsciiSet.single_count()>>
struct ascii_set_matcher;
template <const auto& CompressedAsciiSet, std::size_t... RangeIdx, std::size_t... SingleIdx>
struct ascii_set_matcher<CompressedAsciiSet, index_sequence<RangeIdx...>,
                         index_sequence<SingleIdx...>>
{
    template <typename Encoding>
    static LEXY_CONSTEVAL auto to_int_type(char c)
    {
        return Encoding::to_int_type(static_cast<typename Encoding::char_type>(c));
    }

    template <typename Encoding>
    LEXY_FORCE_INLINE static constexpr bool match([[maybe_unused]] typename Encoding::int_type cur)
    {
        return
            // It must be in one of the ranges...
            ((to_int_type<Encoding>(CompressedAsciiSet.range_lower[RangeIdx]) <= cur
              && cur <= to_int_type<Encoding>(CompressedAsciiSet.range_upper[RangeIdx]))
             || ...)
            // or one of the single characters.
            || ((cur == to_int_type<Encoding>(CompressedAsciiSet.singles[SingleIdx])) || ...);
    }
};
} // namespace lexy::_detail

namespace lexyd
{
template <typename CharSet>
constexpr auto _cas = lexy::_detail::compress_ascii_set<CharSet>();

template <typename Derived>
struct char_class_base : token_base<Derived>, _char_class_base
{
    //=== "virtual" functions ===//
    // static const char* char_class_name();
    // static ascii_set char_class_ascii();

    static constexpr bool char_class_unicode()
    {
        return true;
    }

    static constexpr std::false_type char_class_match_cp(char32_t)
    {
        return {};
    }

    template <typename Reader, typename Context>
    static constexpr void char_class_report_error(Context&                  context,
                                                  typename Reader::iterator position)
    {
        constexpr auto name = Derived::char_class_name();
        auto           err  = lexy::error<Reader, lexy::expected_char_class>(position, name);
        context.on(_ev::error{}, err);
    }

    /// Returns true if c contains only characters from the char class.
    /// If it returns false, it may still be valid, it just couldn't be detected.
    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int)
    {
        return std::false_type{};
    }

    //=== provided functions ===//
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);

            using matcher = lexy::_detail::ascii_set_matcher<_cas<Derived>>;
            if (matcher::template match<typename Reader::encoding>(reader.peek()))
            {
                reader.bump();
                end = reader.current();
                return true;
            }

            if constexpr (std::is_same_v<decltype(Derived::char_class_match_cp(char32_t())),
                                         std::false_type>)
            {
                return false;
            }
            else if constexpr (lexy::is_unicode_encoding<typename Reader::encoding>)
            {
                static_assert(Derived::char_class_unicode(),
                              "cannot use this character class with Unicode encoding");

                // Parse one code point.
                auto result = lexy::_detail::parse_code_point(reader);
                if (result.error != lexy::_detail::cp_error::success)
                    return false;

                if (!Derived::char_class_match_cp(result.cp))
                    return false;

                end = result.end;
                return true;
            }
            else
            {
                static_assert(!Derived::char_class_unicode(),
                              "cannot use this character class with non-Unicode char encodings");

                if (reader.peek() == Reader::encoding::eof())
                    return false;

                auto cp = static_cast<char32_t>(reader.peek());
                reader.bump();

                if (!Derived::char_class_match_cp(cp))
                    return false;

                end = reader.current();
                return true;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            Derived::template char_class_report_error<Reader>(context, reader.position());
        }
    };
};
} // namespace lexyd

#define LEXY_CHAR_CLASS(Name, Rule)                                                                \
    [] {                                                                                           \
        static_assert(::lexy::is_char_class_rule<LEXY_DECAY_DECLTYPE(Rule)>);                      \
        struct c : ::lexyd::char_class_base<c>                                                     \
        {                                                                                          \
            static constexpr auto char_class_unicode()                                             \
            {                                                                                      \
                return (Rule).char_class_unicode();                                                \
            }                                                                                      \
            static LEXY_CONSTEVAL auto char_class_name()                                           \
            {                                                                                      \
                return Name;                                                                       \
            }                                                                                      \
            static LEXY_CONSTEVAL auto char_class_ascii()                                          \
            {                                                                                      \
                return (Rule).char_class_ascii();                                                  \
            }                                                                                      \
            static constexpr auto char_class_match_cp(char32_t cp)                                 \
            {                                                                                      \
                return (Rule).char_class_match_cp(cp);                                             \
            }                                                                                      \
        };                                                                                         \
        return c{};                                                                                \
    }()

namespace lexyd
{
template <typename CharT, CharT... C>
struct _lit;
template <char32_t... Cp>
struct _lcp;

// Implementation helper for the literal overloads.
template <char32_t Cp>
struct _ccp : char_class_base<_ccp<Cp>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        if constexpr (Cp <= 0x7F)
            result.insert(Cp);
        return result;
    }

    static constexpr auto char_class_match_cp([[maybe_unused]] char32_t cp)
    {
        if constexpr (Cp <= 0x7F)
            return std::false_type{};
        else
            return cp == Cp;
    }
};
template <unsigned char Byte>
struct _cb : char_class_base<_cb<Byte>>
{
    static constexpr auto char_class_unicode()
    {
        return Byte <= 0x7F;
    }

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "byte";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        if constexpr (Byte <= 0x7F)
            result.insert(Byte);
        return result;
    }

    static constexpr auto char_class_match_cp([[maybe_unused]] char32_t cp)
    {
        if constexpr (Byte <= 0x7F)
            return std::false_type{};
        else
            return cp == Byte;
    }
};

template <typename C, typename = std::enable_if_t<lexy::is_char_class_rule<C>>>
constexpr auto _make_char_class(C c)
{
    return c;
}
template <typename CharT, CharT C,
          typename = std::enable_if_t<C <= 0x7F || std::is_same_v<CharT, char32_t>
                                      || std::is_same_v<CharT, unsigned char>>>
constexpr auto _make_char_class(_lit<CharT, C>)
{
    if constexpr (std::is_same_v<CharT, unsigned char>)
        return _cb<C>{};
    else
        return _ccp<static_cast<char32_t>(C)>{};
}
template <char32_t CP>
constexpr auto _make_char_class(_lcp<CP>)
{
    return _ccp<CP>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename... Cs>
struct _calt : char_class_base<_calt<Cs...>>
{
    static_assert(sizeof...(Cs) > 1);

    static constexpr auto char_class_unicode()
    {
        constexpr auto non_unicode = (!Cs::char_class_unicode() || ...);
        static_assert(!non_unicode
                          // If at least one is non-Unicode, either they all must be non-Unicode or
                          // only match ASCII.
                          || ((!Cs::char_class_unicode()
                               || std::is_same_v<decltype(Cs::char_class_match_cp(0)),
                                                 std::false_type>)
                              && ...),
                      "cannot mix bytes and Unicode char classes");
        return !non_unicode;
    }

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "union";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        (result.insert(Cs::char_class_ascii()), ...);
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if constexpr ((std::is_same_v<decltype(Cs::char_class_match_cp(cp)), std::false_type>
                       && ...))
            return std::false_type{};
        else
            return (Cs::char_class_match_cp(cp) || ...);
    }
};

template <typename R1, typename R2>
constexpr auto operator/(R1 r1, R2 r2)
    -> _calt<decltype(_make_char_class(r1)), decltype(_make_char_class(r2))>
{
    return {};
}

template <typename... Cs, typename C>
constexpr auto operator/(_calt<Cs...>, C c) -> _calt<Cs..., decltype(_make_char_class(c))>
{
    return {};
}
template <typename C, typename... Cs>
constexpr auto operator/(C c, _calt<Cs...>) -> _calt<decltype(_make_char_class(c)), Cs...>
{
    return {};
}

template <typename... Cs, typename... Ds>
constexpr auto operator/(_calt<Cs...>, _calt<Ds...>) -> _calt<Cs..., Ds...>
{
    return {};
}
} // namespace lexyd

namespace lexyd
{
template <typename C>
struct _ccomp : char_class_base<_ccomp<C>>
{
    static constexpr auto char_class_unicode()
    {
        return C::char_class_unicode();
    }

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "complement";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x00, 0x7F);
        result.remove(C::char_class_ascii());
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if (cp <= 0x7F)
            // If we haven't matched an ASCII character so far, this was intentional.
            return false;

        if constexpr (std::is_same_v<decltype(C::char_class_match_cp(cp)), std::false_type>)
            return true;
        else
            return !C::char_class_match_cp(cp);
    }
};

template <typename C>
constexpr auto operator-(C c) -> _ccomp<decltype(_make_char_class(c))>
{
    return {};
}
template <typename C>
constexpr auto operator-(_ccomp<C>) -> C
{
    return {};
}
} // namespace lexyd

namespace lexyd
{
template <typename Set, typename Minus>
struct _cminus : char_class_base<_cminus<Set, Minus>>
{
    // calt does the correct logic as well, so re-use it.
    static constexpr auto char_class_unicode()
    {
        return _calt<Set, Minus>::char_class_unicode();
    }

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "minus";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        auto result = Set::char_class_ascii();
        result.remove(Minus::char_class_ascii());
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if constexpr (std::is_same_v<decltype(Set::char_class_match_cp(cp)), std::false_type>)
            return std::false_type{};
        else if constexpr (std::is_same_v<decltype(Minus::char_class_match_cp(cp)),
                                          std::false_type>)
            // We don't match ASCII at this point: they only reach this point if the ASCII table
            // failed.
            return cp > 0x7F && Set::char_class_match_cp(cp);
        else
            // Same as above, no ASCII.
            return cp > 0x7F && Set::char_class_match_cp(cp) && !Minus::char_class_match_cp(cp);
    }
};

template <typename Set, typename Minus>
constexpr auto operator-(Set, Minus minus)
{
    return _cminus<Set, decltype(_make_char_class(minus))>{};
}

template <typename Set, typename Minus, typename OtherMinus>
constexpr auto operator-(_cminus<Set, Minus>, OtherMinus other)
{
    return Set{} - _calt<Minus, decltype(_make_char_class(other))>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename... Cs>
struct _cand : char_class_base<_cand<Cs...>>
{
    static_assert(sizeof...(Cs) > 1);

    // calt does the correct logic as well, so re-use it.
    static constexpr auto char_class_unicode()
    {
        return _calt<Cs...>::char_class_unicode();
    }

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "intersection";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        for (auto c = 0; c <= 0x7F; ++c)
            if ((Cs::char_class_ascii().contains[c] && ...))
                result.insert(c);
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if constexpr ((std::is_same_v<decltype(Cs::char_class_match_cp(cp)), std::false_type>
                       && ...))
            return std::false_type{};
        else
            return (Cs::char_class_match_cp(cp) && ...);
    }
};

template <typename C1, typename C2>
constexpr auto operator&(C1 c1, C2 c2)
    -> _cand<decltype(_make_char_class(c1)), decltype(_make_char_class(c2))>
{
    return {};
}

template <typename... Cs, typename C>
constexpr auto operator&(_cand<Cs...>, C c) -> _cand<Cs..., decltype(_make_char_class(c))>
{
    return {};
}
template <typename C, typename... Cs>
constexpr auto operator&(C c, _cand<Cs...>) -> _cand<decltype(_make_char_class(c)), Cs...>
{
    return {};
}

template <typename... Cs, typename... Ds>
constexpr auto operator&(_cand<Cs...>, _cand<Ds...>) -> _cand<Cs..., Ds...>
{
    return {};
}
} // namespace lexyd

#endif // LEXY_DSL_CHAR_CLASS_HPP_INCLUDED


namespace lexyd
{
template <typename Predicate>
struct _cp : char_class_base<_cp<Predicate>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        if constexpr (std::is_void_v<Predicate>)
            return "code-point";
        else
            return lexy::_detail::type_name<Predicate>();
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        if constexpr (std::is_void_v<Predicate>)
        {
            lexy::_detail::ascii_set result;
            result.insert(0x00, 0x7F);
            return result;
        }
        else
        {
            lexy::_detail::ascii_set result;
            for (auto c = 0; c <= 0x7F; ++c)
                if (Predicate{}(lexy::code_point(char32_t(c))))
                    result.insert(c);
            return result;
        }
    }

    static constexpr bool char_class_match_cp([[maybe_unused]] char32_t cp)
    {
        if constexpr (std::is_void_v<Predicate>)
            return true;
        else
            return Predicate{}(lexy::code_point(cp));
    }

    //=== dsl ===//
    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _cp<P>{};
    }

    template <char32_t Low, char32_t High>
    constexpr auto range() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.range";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return Low <= cp.value() && cp.value() <= High;
            }
        };

        return if_<predicate>();
    }

    template <char32_t... CPs>
    constexpr auto set() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.set";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return ((cp.value() == CPs) || ...);
            }
        };

        return if_<predicate>();
    }

    constexpr auto ascii() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.ASCII";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return cp.is_ascii();
            }
        };

        return if_<predicate>();
    }
    constexpr auto bmp() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.BMP";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return cp.is_bmp();
            }
        };

        return if_<predicate>();
    }
    constexpr auto noncharacter() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.non-character";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return cp.is_noncharacter();
            }
        };

        return if_<predicate>();
    }

    template <lexy::code_point::general_category_t Category>
    constexpr auto general_category() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return lexy::_detail::general_category_name(Category);
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                // Note: can't use `cp.is_noncharacter()` for `Cn` as `Cn` also includes all code
                // points that are currently unassigned.
                if constexpr (Category == lexy::code_point::Cc)
                    return cp.is_control();
                else if constexpr (Category == lexy::code_point::Cs)
                    return cp.is_surrogate();
                else if constexpr (Category == lexy::code_point::Co)
                    return cp.is_private_use();
                else
                    return cp.general_category() == Category;
            }
        };

        return if_<predicate>();
    }

    template <const auto& GcGroup>
    struct _group_pred;
    template <lexy::code_point::general_category_t... Cats,
              const lexy::code_point::_gc_group<Cats...>& GcGroup>
    struct _group_pred<GcGroup>
    {
        static LEXY_CONSTEVAL auto name()
        {
            return GcGroup.name;
        }

        constexpr bool operator()(lexy::code_point cp) const
        {
            return cp.general_category() == GcGroup;
        }
    };
    template <const auto& GcGroup>
    constexpr auto general_category() const
    {
        return if_<_group_pred<GcGroup>>();
    }
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp<void>{};
} // namespace lexyd

namespace lexy
{
// The void-version without predicate logically matches any input (modulo encoding errors, of
// course).
template <>
inline constexpr auto token_kind_of<lexy::dsl::_cp<void>> = lexy::any_token_kind;
} // namespace lexy

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED









//=== lit_matcher ===//
namespace lexy::_detail
{
template <std::size_t CurCharIndex, typename CharT, CharT... Cs, typename Reader>
constexpr auto match_literal(Reader& reader)
{
    static_assert(lexy::is_char_encoding<typename Reader::encoding>);
    using char_type = typename Reader::encoding::char_type;
    if constexpr (CurCharIndex >= sizeof...(Cs))
    {
        (void)reader;
        return std::true_type{};
    }
    // We only use SWAR if the reader supports it and we have enough to fill at least one.
    else if constexpr (is_swar_reader<Reader> && sizeof...(Cs) >= swar_length<char_type>)
    {
        // Try and pack as many characters into a swar as possible, starting at the current
        // index.
        constexpr auto pack = swar_pack<CurCharIndex>(transcode_char<char_type>(Cs)...);

        // Do a single swar comparison.
        if ((reader.peek_swar() & pack.mask) == pack.value)
        {
            reader.bump_swar(pack.count);

            // Recurse with the incremented index.
            return bool(match_literal<CurCharIndex + pack.count, CharT, Cs...>(reader));
        }
        else
        {
            auto partial = swar_find_difference<CharT>(reader.peek_swar() & pack.mask, pack.value);
            reader.bump_swar(partial);
            return false;
        }
    }
    else
    {
        static_assert(CurCharIndex == 0);

        // Compare each code unit, bump on success, cancel on failure.
        return ((reader.peek() == transcode_int<typename Reader::encoding>(Cs)
                     ? (reader.bump(), true)
                     : false)
                && ...);
    }
}
} // namespace lexy::_detail

//=== lit_trie ===//
namespace lexy::_detail
{
template <typename Encoding, template <typename> typename CaseFolding, std::size_t MaxCharCount,
          typename... CharClasses>
struct lit_trie
{
    using encoding  = Encoding;
    using char_type = typename Encoding::char_type;

    template <typename Reader>
    using reader = CaseFolding<Reader>;

    static constexpr auto max_node_count = MaxCharCount + 1; // root node
    static constexpr auto max_transition_count
        = max_node_count == 1 ? 1 : max_node_count - 1; // it is a tree
    static constexpr auto node_no_match = std::size_t(-1);

    std::size_t node_count;
    std::size_t node_value[max_node_count];
    // Index of a char class that must not match at the end.
    // This is used for keywords.
    std::size_t node_char_class[max_node_count];

    char_type   transition_char[max_transition_count];
    std::size_t transition_from[max_transition_count];
    std::size_t transition_to[max_transition_count];

    LEXY_CONSTEVAL lit_trie()
    : node_count(1), node_value{}, node_char_class{}, transition_char{}, transition_from{},
      transition_to{}
    {
        node_value[0]      = node_no_match;
        node_char_class[0] = sizeof...(CharClasses);
    }

    template <typename CharT>
    LEXY_CONSTEVAL std::size_t insert(std::size_t from, CharT _c)
    {
        auto c = transcode_char<char_type>(_c);

        // We need to find a transition.
        // In a tree, we're always having node_count - 1 transitions, so that's the upper bound.
        // Everytime we add a transition from a node, its transition index is >= its node index.
        // As such, we start looking at the node index.
        for (auto i = from; i != node_count - 1; ++i)
        {
            if (transition_from[i] == from && transition_char[i] == c)
                return transition_to[i];
        }

        auto to             = node_count;
        node_value[to]      = node_no_match;
        node_char_class[to] = sizeof...(CharClasses);

        auto trans             = node_count - 1;
        transition_char[trans] = c;
        transition_from[trans] = from; // trans >= from as from < node_count
        transition_to[trans]   = to;

        ++node_count;
        return to;
    }

    template <typename CharT, CharT... C>
    LEXY_CONSTEVAL std::size_t insert(std::size_t pos, type_string<CharT, C...>)
    {
        return ((pos = insert(pos, C)), ...);
    }

    LEXY_CONSTEVAL auto node_transitions(std::size_t node) const
    {
        struct
        {
            std::size_t length;
            std::size_t index[max_transition_count];
        } result{};

        // We need to consider the same range only.
        for (auto i = node; i != node_count - 1; ++i)
            if (transition_from[i] == node)
            {
                result.index[result.length] = i;
                ++result.length;
            }

        return result;
    }
};

template <typename... CharClasses>
struct char_class_list
{
    template <typename Encoding, template <typename> typename CaseFolding, std::size_t N>
    using trie_type = lit_trie<Encoding, CaseFolding, N, CharClasses...>;

    static constexpr auto size = sizeof...(CharClasses);

    template <typename... T>
    constexpr auto operator+(char_class_list<T...>) const
    {
        return char_class_list<CharClasses..., T...>{};
    }
};

template <typename CurrentCaseFolding, typename... Literals>
struct _merge_case_folding;
template <>
struct _merge_case_folding<void>
{
    template <typename Reader>
    using case_folding = Reader;
};
template <typename CurrentCaseFolding>
struct _merge_case_folding<CurrentCaseFolding>
{
    template <typename Reader>
    using case_folding = typename CurrentCaseFolding::template reader<Reader>;
};
template <typename CurrentCaseFolding, typename H, typename... T>
struct _merge_case_folding<CurrentCaseFolding, H, T...>
: _merge_case_folding<std::conditional_t<std::is_void_v<CurrentCaseFolding>,
                                         typename H::lit_case_folding, CurrentCaseFolding>,
                      T...>
{
    static_assert(std::is_same_v<CurrentCaseFolding,
                                 typename H::lit_case_folding> //
                      || std::is_void_v<CurrentCaseFolding>
                      || std::is_void_v<typename H::lit_case_folding>,
                  "cannot mix literals with different case foldings in a literal_set");
};

template <typename Reader>
using lit_no_case_fold = Reader;

template <typename Encoding, typename... Literals>
LEXY_CONSTEVAL auto make_empty_trie()
{
    constexpr auto max_char_count = (0 + ... + Literals::lit_max_char_count);

    // Merge all mentioned character classes in a single list.
    constexpr auto char_classes
        = (lexy::_detail::char_class_list{} + ... + Literals::lit_char_classes);

    // Need to figure out case folding as well.
    return typename decltype(char_classes)::template trie_type<
        Encoding, _merge_case_folding<void, Literals...>::template case_folding, max_char_count>{};
}
template <typename Encoding, typename... Literals>
using lit_trie_for = decltype(make_empty_trie<Encoding, Literals...>());

template <std::size_t CharClassIdx, bool, typename...>
struct _node_char_class_impl
{
    template <typename Reader>
    LEXY_FORCE_INLINE static constexpr std::false_type match(const Reader&)
    {
        return {};
    }
};
template <typename H, typename... T>
struct _node_char_class_impl<0, true, H, T...>
{
    template <typename Reader>
    LEXY_FORCE_INLINE static constexpr bool match(Reader reader)
    {
        return lexy::token_parser_for<H, Reader>(reader).try_parse(reader);
    }
};
template <std::size_t Idx, typename H, typename... T>
struct _node_char_class_impl<Idx, true, H, T...> : _node_char_class_impl<Idx - 1, true, T...>
{};
template <std::size_t CharClassIdx, typename... CharClasses>
using _node_char_class
    = _node_char_class_impl<CharClassIdx, (CharClassIdx < sizeof...(CharClasses)), CharClasses...>;

template <const auto& Trie, std::size_t CurNode>
struct lit_trie_matcher;
template <typename Encoding, template <typename> typename CaseFolding, std::size_t N,
          typename... CharClasses, const lit_trie<Encoding, CaseFolding, N, CharClasses...>& Trie,
          std::size_t CurNode>
struct lit_trie_matcher<Trie, CurNode>
{
    template <std::size_t TransIdx, typename Reader, typename IntT>
    LEXY_FORCE_INLINE static constexpr bool _try_transition(std::size_t& result, Reader& reader,
                                                            IntT cur)
    {
        static_assert(Trie.transition_from[TransIdx] == CurNode);

        using encoding            = typename Reader::encoding;
        constexpr auto trans_char = Trie.transition_char[TransIdx];
        if (cur != encoding::to_int_type(trans_char))
            return false;

        reader.bump();
        result = lit_trie_matcher<Trie, Trie.transition_to[TransIdx]>::try_match(reader);
        return true;
    }

    static constexpr auto transitions = Trie.node_transitions(CurNode);

    template <typename Indices = make_index_sequence<transitions.length>>
    struct _impl;
    template <std::size_t... Idx>
    struct _impl<index_sequence<Idx...>>
    {
        template <typename Reader>
        LEXY_FORCE_INLINE static constexpr std::size_t try_match(Reader& reader)
        {
            constexpr auto cur_value = Trie.node_value[CurNode];

            if constexpr (sizeof...(Idx) > 0)
            {
                auto cur      = reader.current();
                auto cur_char = reader.peek();

                auto next_value = Trie.node_no_match;
                (void)(_try_transition<transitions.index[Idx]>(next_value, reader, cur_char)
                       || ...);
                if (next_value != Trie.node_no_match)
                    // We prefer a longer match.
                    return next_value;

                // We haven't found a longer match, return our match.
                reader.reset(cur);
            }

            // But first, we might need to check that we don't match that nodes char class.
            constexpr auto char_class = Trie.node_char_class[CurNode];
            if constexpr (cur_value == Trie.node_no_match || char_class >= sizeof...(CharClasses))
            {
                return cur_value;
            }
            else
            {
                if (_node_char_class<char_class, CharClasses...>::match(reader))
                    return Trie.node_no_match;
                else
                    return cur_value;
            }
        }
    };

    template <typename Reader>
    LEXY_FORCE_INLINE static constexpr std::size_t try_match(Reader& _reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        if constexpr (std::is_same_v<CaseFolding<Reader>, Reader>)
        {
            return _impl<>::try_match(_reader);
        }
        else
        {
            CaseFolding<Reader> reader{_reader};
            auto                result = _impl<>::try_match(reader);
            _reader.reset(reader.current());
            return result;
        }
    }
};
} // namespace lexy::_detail

//=== lit ===//
namespace lexyd
{
template <typename CharT, CharT... C>
struct _lit
: token_base<_lit<CharT, C...>,
             std::conditional_t<sizeof...(C) == 0, unconditional_branch_base, branch_base>>,
  _lit_base
{
    static constexpr auto lit_max_char_count = sizeof...(C);
    static constexpr auto lit_char_classes   = lexy::_detail::char_class_list{};
    using lit_case_folding                   = void;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        typename Encoding::char_type result = 0;
        (void)((result = lexy::_detail::transcode_char<decltype(result)>(C), true) || ...);
        return result;
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos, std::size_t)
    {
        return ((pos = trie.insert(pos, C)), ...);
    }

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr auto try_parse(Reader reader)
        {
            auto result = lexy::_detail::match_literal<0, CharT, C...>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using char_type    = typename Reader::encoding::char_type;
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<char_type>;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, end.position());
            auto err = lexy::error<Reader, lexy::expected_literal>(begin, str, index, sizeof...(C));
            context.on(_ev::error{}, err);
        }
    };
};

template <auto C>
constexpr auto lit_c = _lit<LEXY_DECAY_DECLTYPE(C), C>{};

template <unsigned char... C>
constexpr auto lit_b = _lit<unsigned char, C...>{};

#if LEXY_HAS_NTTP
/// Matches the literal string.
template <lexy::_detail::string_literal Str>
constexpr auto lit = lexy::_detail::to_type_string<_lit, Str>{};
#endif

#define LEXY_LIT(Str)                                                                              \
    LEXY_NTTP_STRING(::lexyd::_lit, Str) {}
} // namespace lexyd

namespace lexy
{
template <typename CharT, CharT... C>
inline constexpr auto token_kind_of<lexy::dsl::_lit<CharT, C...>> = lexy::literal_token_kind;
} // namespace lexy

//=== lit_cp ===//
namespace lexyd
{
template <char32_t... Cp>
struct _lcp : token_base<_lcp<Cp...>>, _lit_base
{
    template <typename Encoding>
    struct _string_t
    {
        typename Encoding::char_type data[4 * sizeof...(Cp)];
        std::size_t                  length = 0;

        constexpr _string_t() : data{}
        {
            ((length += lexy::_detail::encode_code_point<Encoding>(Cp, data + length, 4)), ...);
        }
    };
    template <typename Encoding>
    static constexpr _string_t<Encoding> _string = _string_t<Encoding>{};

    static constexpr auto lit_max_char_count = 4 * sizeof...(Cp);
    static constexpr auto lit_char_classes   = lexy::_detail::char_class_list{};
    using lit_case_folding                   = void;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        return _string<Encoding>.data[0];
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos, std::size_t)
    {
        using encoding = typename Trie::encoding;

        for (auto i = 0u; i != _string<encoding>.length; ++i)
            pos = trie.insert(pos, _string<encoding>.data[i]);

        return pos;
    }

    template <typename Reader,
              typename Indices
              = lexy::_detail::make_index_sequence<_string<typename Reader::encoding>.length>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;

            auto result = lexy::_detail::match_literal<0, typename encoding::char_type,
                                                       _string<encoding>.data[Idx]...>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using encoding = typename Reader::encoding;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, end.position());
            auto err   = lexy::error<Reader, lexy::expected_literal>(begin, _string<encoding>.data,
                                                                     index, _string<encoding>.length);
            context.on(_ev::error{}, err);
        }
    };
};

template <char32_t... CodePoint>
constexpr auto lit_cp = _lcp<CodePoint...>{};
} // namespace lexyd

namespace lexy
{
template <char32_t... Cp>
constexpr auto token_kind_of<lexy::dsl::_lcp<Cp...>> = lexy::literal_token_kind;
} // namespace lexy

//=== lit_set ===//
namespace lexy
{
template <typename T, template <typename> typename CaseFolding, typename... Strings>
class _symbol_table;

struct expected_literal_set
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected literal set";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Literal, template <typename> typename CaseFolding>
struct _cfl;

template <template <typename> typename CaseFolding, typename CharT, CharT... C>
constexpr auto _make_lit_rule(lexy::_detail::type_string<CharT, C...>)
{
    if constexpr (std::is_same_v<CaseFolding<lexy::_pr8>, lexy::_pr8>)
        return _lit<CharT, C...>{};
    else
        return _cfl<_lit<CharT, C...>, CaseFolding>{};
}

template <typename... Literals>
struct _lset : token_base<_lset<Literals...>>, _lset_base
{
    using as_lset = _lset;

    template <typename Encoding>
    static LEXY_CONSTEVAL auto _build_trie()
    {
        auto result = lexy::_detail::make_empty_trie<Encoding, Literals...>();

        [[maybe_unused]] auto char_class = std::size_t(0);
        ((result.node_value[Literals::lit_insert(result, 0, char_class)] = 0,
          // Keep the index correct.
          char_class += Literals::lit_char_classes.size),
         ...);

        return result;
    }
    template <typename Encoding>
    static constexpr lexy::_detail::lit_trie_for<Encoding, Literals...> _t
        = _build_trie<Encoding>();

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;
            using matcher  = lexy::_detail::lit_trie_matcher<_t<encoding>, 0>;

            auto result = matcher::try_match(reader);
            end         = reader.current();
            return result != _t<encoding>.node_no_match;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_literal_set>(reader.position());
            context.on(_ev::error{}, err);
        }
    };

    //=== dsl ===//
    template <typename Lit>
    constexpr auto operator/(Lit) const
    {
        if constexpr (lexy::is_literal_rule<Lit>)
        {
            return _lset<Literals..., Lit>{};
        }
        else if constexpr (sizeof...(Literals) == 0)
        {
            // We're empty, so do nothing and keep it type-erased.
            static_assert(lexy::is_literal_set_rule<Lit>);
            return Lit{};
        }
        else
        {
            // We're non empty, undo type erasure to append.
            static_assert(lexy::is_literal_set_rule<Lit>);
            return *this / typename Lit::as_lset{};
        }
    }
    template <typename... Lit>
    constexpr auto operator/(_lset<Lit...>) const
    {
        return _lset<Literals..., Lit...>{};
    }
};

/// Matches one of the specified literals.
template <typename... Literals>
constexpr auto literal_set(Literals...)
{
    static_assert((lexy::is_literal_rule<Literals> && ...));
    return _lset<Literals...>{};
}

/// Matches one of the symbols in the symbol table.
template <typename T, template <typename> typename CaseFolding, typename... Strings>
constexpr auto literal_set(const lexy::_symbol_table<T, CaseFolding, Strings...>)
{
    return _lset<decltype(_make_lit_rule<CaseFolding>(Strings{}))...>{};
}
} // namespace lexyd

#define LEXY_LITERAL_SET(...)                                                                      \
    [] {                                                                                           \
        using impl = decltype(::lexyd::literal_set(__VA_ARGS__));                                  \
        struct s : impl                                                                            \
        {                                                                                          \
            using impl::operator/;                                                                 \
        };                                                                                         \
        return s{};                                                                                \
    }()

namespace lexy
{
template <typename... Literals>
constexpr auto token_kind_of<lexy::dsl::_lset<Literals...>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED



namespace lexy
{
struct expected_newline
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected newline";
    }
};
} // namespace lexy

namespace lexyd
{
struct _nl : LEXY_DECAY_DECLTYPE(
                 literal_set(LEXY_LIT("\n"), LEXY_LIT("\r\n")).error<lexy::expected_newline>){};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

namespace lexyd
{
struct _eol : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);

        constexpr bool try_parse(const void*, Reader reader)
        {
            return reader.peek() == Reader::encoding::eof()
                   || lexy::try_match_token(newline, reader);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            if (reader.peek() == Reader::encoding::eof())
            {
                auto pos = reader.position();
                context.on(_ev::token{}, lexy::eof_token_kind, pos, pos);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // Note that we're re-doing the parsing for newline,
                // this looks at most at two characters, so it doesn't really matter.
                return lexy::parser_for<_nl, NextParser>::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            return bp<Reader>{}.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches the end of line (EOF or newline).
constexpr auto eol = _eol{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED




//=== input_location_anchor ===//
namespace lexy
{
/// Anchor for the location search.
template <typename Input>
struct input_location_anchor
{
    using marker = typename lexy::input_reader<Input>::marker;

    constexpr explicit input_location_anchor(const Input& input)
    : _line_begin(input.reader().current()), _line_nr(1)
    {}

    // implementation detail
    constexpr explicit input_location_anchor(marker line_begin, unsigned line_nr)
    : _line_begin(line_begin), _line_nr(line_nr)
    {}

    marker   _line_begin;
    unsigned _line_nr;
};
} // namespace lexy

//=== counting strategies ===//
namespace lexy
{
/// Counts code units for columns, newlines for lines.
class code_unit_location_counting
{
public:
    template <typename Reader>
    constexpr bool try_match_newline(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        return lexy::try_match_token(lexy::dsl::newline, reader);
    }

    template <typename Reader>
    constexpr void match_column(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        reader.bump();
    }
};

/// Counts code points for columns, newlines for lines.
class code_point_location_counting
{
public:
    template <typename Reader>
    constexpr bool try_match_newline(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        return lexy::try_match_token(lexy::dsl::newline, reader);
    }

    template <typename Reader>
    constexpr void match_column(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        if (!lexy::try_match_token(lexy::dsl::code_point, reader))
            reader.bump();
    }
};

/// Counts bytes for columns, lines end after LineWidth bytes.
template <std::size_t LineWidth = 16>
class byte_location_counting
{
public:
    template <typename Reader>
    constexpr bool try_match_newline(Reader& reader)
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        LEXY_PRECONDITION(_cur_index <= LineWidth - 1);
        if (_cur_index == LineWidth - 1)
        {
            // Consider the last byte to be the "newline".
            // We need to consume something if possible;
            // the logic in the function breaks otherwise.
            if (reader.peek() != Reader::encoding::eof())
                reader.bump();
            _cur_index = 0;
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename Reader>
    constexpr void match_column(Reader& reader)
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);

        reader.bump();
        ++_cur_index;
    }

private:
    std::size_t _cur_index = 0;
};

template <typename Input>
auto _compute_default_location_counting()
{
    using encoding = typename lexy::input_reader<Input>::encoding;
    if constexpr (lexy::is_byte_encoding<encoding>)
        return byte_location_counting{};
    else if constexpr (lexy::is_char_encoding<encoding>)
        return code_unit_location_counting{};
    else
        static_assert(_detail::error<Input>,
                      "input encoding does not have a default location counting policy");
}

template <typename Input>
using _default_location_counting = decltype(_compute_default_location_counting<Input>());
} // namespace lexy

//=== input_location ===//
namespace lexy
{
/// A location in the input.
template <typename Input, typename Counting = _default_location_counting<Input>>
class input_location
{
    using iterator = typename lexy::input_reader<Input>::iterator;
    using marker   = typename lexy::input_reader<Input>::marker;

public:
    constexpr explicit input_location(const Input& input)
    : _line_begin(input.reader().current()), _column_begin(_line_begin.position()), _line_nr(1),
      _column_nr(1)
    {}

    /// The closest previous anchor.
    constexpr input_location_anchor<Input> anchor() const
    {
        return input_location_anchor<Input>(_line_begin, _line_nr);
    }

    constexpr unsigned line_nr() const
    {
        return _line_nr;
    }
    constexpr unsigned column_nr() const
    {
        return _column_nr;
    }

    /// The corresponding position, rounded down to the previous column start.
    constexpr iterator position() const
    {
        return _column_begin;
    }

    friend constexpr bool operator==(const input_location& lhs, const input_location& rhs)
    {
        return lhs._line_nr == rhs._line_nr && lhs._column_nr == rhs._column_nr;
    }
    friend constexpr bool operator!=(const input_location& lhs, const input_location& rhs)
    {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(const input_location& lhs, const input_location& rhs)
    {
        if (lhs._line_nr != rhs._line_nr)
            return lhs._line_nr < rhs._line_nr;
        return lhs._column_nr < rhs._column_nr;
    }
    friend constexpr bool operator<=(const input_location& lhs, const input_location& rhs)
    {
        return !(rhs < lhs);
    }
    friend constexpr bool operator>(const input_location& lhs, const input_location& rhs)
    {
        return rhs < lhs;
    }
    friend constexpr bool operator>=(const input_location& lhs, const input_location& rhs)
    {
        return !(rhs > lhs);
    }

private:
    constexpr input_location(marker line_begin, unsigned line_nr, iterator column_begin,
                             unsigned column_nr)
    : _line_begin(line_begin), _column_begin(column_begin), _line_nr(line_nr), _column_nr(column_nr)
    {}

    marker   _line_begin;
    iterator _column_begin;
    unsigned _line_nr, _column_nr;

    template <typename C, typename I>
    friend constexpr auto get_input_location(
        const I& input, typename lexy::input_reader<I>::iterator position,
        input_location_anchor<I> anchor) -> input_location<I, C>;
};

/// The location for a position in the input; search starts at the anchor.
template <typename Counting, typename Input>
constexpr auto get_input_location(
    const Input& input, typename lexy::input_reader<Input>::iterator position,
    input_location_anchor<Input> anchor) -> input_location<Input, Counting>
{
    auto reader = input.reader();
    reader.reset(anchor._line_begin);

    auto line_begin   = anchor._line_begin;
    auto line_nr      = anchor._line_nr;
    auto column_begin = line_begin;
    auto column_nr    = 1u;

    Counting counting;
    while (true)
    {
        if (reader.position() == position)
        {
            // We've already found the position; it's at the beginning of a colum nor newline.
            // No need to do the expensive checks.
            //
            // This also allows `lexy_ext::shell` to work properly, if position is at EOF,
            // the reader.peek() call will ask for more input.
            break;
        }
        else if (reader.peek() == lexy::input_reader<Input>::encoding::eof())
        {
            LEXY_ASSERT(false, "invalid position + anchor combination");
        }
        else if (counting.try_match_newline(reader))
        {
            // [column_begin, newline_end) covers the newline.
            auto newline_end = reader.current();
            if (lexy::_detail::min_range_end(column_begin.position(), newline_end.position(),
                                             position)
                != newline_end.position())
                break;

            // Advance to the next line.
            ++line_nr;
            line_begin   = newline_end;
            column_nr    = 1;
            column_begin = line_begin;
        }
        else
        {
            counting.match_column(reader);

            // [column_begin, column_end) covers the column.
            auto column_end = reader.current();
            if (lexy::_detail::min_range_end(column_begin.position(), column_end.position(),
                                             position)
                != column_end.position())
                break;

            // Advance to the next column.
            ++column_nr;
            column_begin = column_end;
        }
    }

    return {line_begin, line_nr, column_begin.position(), column_nr};
}

template <typename Counting, typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position)
{
    return get_input_location<Counting>(input, position, input_location_anchor(input));
}
template <typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position,
                                  input_location_anchor<Input>                 anchor)
{
    return get_input_location<_default_location_counting<Input>>(input, position, anchor);
}
template <typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position)
{
    return get_input_location<_default_location_counting<Input>>(input, position,
                                                                 input_location_anchor(input));
}
} // namespace lexy

//=== input_line_annotation ===//
namespace lexy::_detail
{
template <typename Counting, typename Input>
constexpr auto get_input_line(const Input&                               input,
                              typename lexy::input_reader<Input>::marker line_begin)
{
    auto reader = input.reader();
    reader.reset(line_begin);

    auto line_end = reader.position();
    for (Counting counting;
         reader.peek() != decltype(reader)::encoding::eof() && !counting.try_match_newline(reader);
         line_end = reader.position())
    {
        counting.match_column(reader);
    }
    auto newline_end = reader.position();

    struct result_t
    {
        lexy::lexeme_for<Input> line;
        lexy::lexeme_for<Input> newline;
    };
    return result_t{{line_begin.position(), line_end}, {line_end, newline_end}};
}

// Advances the iterator to the beginning of the next code point.
template <typename Encoding, typename Iterator>
constexpr Iterator find_cp_boundary(Iterator cur, Iterator end)
{
    auto is_cp_continuation = [](auto c) {
        if constexpr (std::is_same_v<Encoding,
                                     lexy::utf8_encoding> //
                      || std::is_same_v<Encoding, lexy::utf8_char_encoding>)
            return (c & 0b1100'0000) == (0b10 << 6);
        else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
            return 0xDC00 <= c && c <= 0xDFFF;
        else
        {
            // This encoding doesn't have continuation code units.
            (void)c;
            return std::false_type{};
        }
    };

    while (cur != end && is_cp_continuation(*cur))
        ++cur;
    return cur;
}
} // namespace lexy::_detail

namespace lexy
{
template <typename Input>
struct input_line_annotation
{
    /// Everything of the line before the range.
    lexy::lexeme_for<Input> before;
    /// The annotated part.
    lexy::lexeme_for<Input> annotated;
    /// Everything of the line after the annotated range.
    lexy::lexeme_for<Input> after;

    /// true if the the range was spanning multiple line and needed to be truncated.
    bool truncated_multiline;
    /// true if annotated includes the newline (this implies after.empty())
    bool annotated_newline;
    /// true if end needed to be moved to a code point boundary.
    bool rounded_end;
};

template <typename Input>
constexpr void _get_input_line_annotation(input_line_annotation<Input>&                result,
                                          lexy::lexeme_for<Input>                      line,
                                          lexy::lexeme_for<Input>                      newline,
                                          typename lexy::input_reader<Input>::iterator begin,
                                          typename lexy::input_reader<Input>::iterator end)
{
    // At this point there are two cases:
    // Either line.begin() <= begin < end <= newline.end()),
    // or line.begin() <= begin == end == newline.end().

    // We then round end to the code point boundary.
    // Note that we don't round begin.
    {
        auto old_end = end;

        using encoding = typename lexy::input_reader<Input>::encoding;
        end            = _detail::find_cp_boundary<encoding>(end, newline.end());

        result.rounded_end = end != old_end;
    }

    // Now we can compute the annotation.
    if (lexy::_detail::min_range_end(line.begin(), line.end(), end) == end)
    {
        // We have end <= line.end(),
        // so line.end() is the end of after.
        result.before    = {line.begin(), begin};
        result.annotated = {begin, end};
        result.after     = {end, line.end()};
    }
    else
    {
        // We have end > line.end(),
        // so newline.end() is the end of annotated.
        result.before            = {line.begin(), begin};
        result.annotated         = {begin, newline.end()};
        result.after             = {newline.end(), newline.end()};
        result.annotated_newline = true;
    }
}

template <typename Input, typename Counting>
constexpr auto get_input_line_annotation(
    const Input& input, const input_location<Input, Counting>& begin_location,
    typename lexy::input_reader<Input>::iterator end) -> input_line_annotation<Input>
{
    input_line_annotation<Input> result{};

    auto [line, newline]
        = _detail::get_input_line<Counting>(input, begin_location.anchor()._line_begin);

    // We first normalize the range.
    auto begin = begin_location.position();
    if (begin == end)
    {
        if (end == newline.begin())
        {
            // Empty range at the newline; make it cover the entire newline.
            end = newline.end();
        }
        else if (end != newline.end())
        {
            // Empty range before end of newline; extend by one code unit.
            ++end;
        }
        else
        {
            // begin == end == newline.end()
        }
    }
    else if (lexy::_detail::min_range_end(begin, end, newline.end()) != end)
    {
        // Truncate a multiline range to a single line.
        // Note that we can't have both an empty range and a multiline range.
        end                        = newline.end();
        result.truncated_multiline = true;
    }

    _get_input_line_annotation(result, line, newline, begin, end);
    return result;
}

/// Computes the annotation for e.g. an error message covering [location, location + size).
template <typename Input, typename Counting>
constexpr auto get_input_line_annotation(const Input&                           input,
                                         const input_location<Input, Counting>& location,
                                         std::size_t                            size)
{
    input_line_annotation<Input> result{};
    auto [line, newline] = _detail::get_input_line<Counting>(input, location.anchor()._line_begin);

    // We don't want an empty annotation.
    auto range_size = size == 0 ? 1 : size;

    auto begin = location.position();
    auto end   = _detail::next_clamped(location.position(), range_size, newline.end());
    if (_detail::range_size(location.position(), end) < size)
    {
        // We didn't have enough of the current line to match the size request.
        // As such, we needed to truncate it.
        result.truncated_multiline = true;
    }

    _get_input_line_annotation(result, line, newline, begin, end);
    return result;
}
} // namespace lexy

#endif // LEXY_INPUT_LOCATION_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_VISUALIZE_HPP_INCLUDED
#define LEXY_VISUALIZE_HPP_INCLUDED

#include <cstdio>


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
        return _range_reader<Encoding>(_begin, _end);
    }

private:
    Iterator                   _begin;
    LEXY_EMPTY_MEMBER Sentinel _end;
};

template <typename Iterator, typename Sentinel>
range_input(Iterator begin, Sentinel end)
    -> range_input<deduce_encoding<LEXY_DECAY_DECLTYPE(*begin)>, Iterator, Sentinel>;

template <typename Encoding, typename Iterator, typename Sentinel>
constexpr bool input_is_view<range_input<Encoding, Iterator, Sentinel>> = true;
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
constexpr auto make_literal_lexeme(const typename Encoding::char_type* str, std::size_t length)
{
    struct reader
    {
        using encoding = Encoding;
        using iterator = const typename Encoding::char_type*;
    };

    return lexy::lexeme<reader>(str, str + length);
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

template <typename OutIt, typename Fn>
constexpr OutIt write_special_char(OutIt out, visualization_options opts, Fn inner)
{
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
}
} // namespace lexy::_detail

namespace lexy
{
template <typename OutputIt>
OutputIt visualize_to(OutputIt out, lexy::code_point cp, visualization_options opts = {})
{
    if (!cp.is_valid())
    {
        out = _detail::write_special_char(out, opts, [opts](OutputIt out) {
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_str(out, "U+????");
            else
                return _detail::write_str(out, "u????");
        });
        return out;
    }
    else if (cp.is_control())
    {
        auto c = static_cast<int>(cp.value());
        switch (c)
        {
        case '\0':
            out = _detail::write_special_char(out, opts, [opts](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_str(out, "NUL");
                else
                    return _detail::write_str(out, "0");
            });
            break;
        case '\r':
            out = _detail::write_special_char(out, opts, [opts](OutputIt out) {
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
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "LF");
                });
            }
            else
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "n");
                });
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
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "HT");
                });
            }
            else
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "t");
                });
            }
            break;

        default:
            out = _detail::write_special_char(out, opts, [opts, c](OutputIt out) {
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
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "SP");
                });
            }
            else
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
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
            out = _detail::write_special_char(out, opts, [](OutputIt out) {
                return _detail::write_str(out, "\\");
            });
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
        out = _detail::write_special_char(out, opts, [opts, cp](OutputIt out) {
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

    [[maybe_unused]] auto write_escaped_byte = [opts](OutputIt out, unsigned char byte) {
        return _detail::write_special_char(out, opts, [opts, byte](OutputIt out) {
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_format(out, "0x%02X", byte);
            else
                return _detail::write_format(out, "x%02X", byte);
        });
    };

    using encoding = typename Reader::encoding;
    if constexpr (lexy::is_unicode_encoding<encoding>)
    {
        // Parse the individual code points, and write them out.
        lexy::range_input<encoding, typename Reader::iterator> input(lexeme.begin(), lexeme.end());
        auto                                                   reader = input.reader();

        auto count = 0u;
        while (true)
        {
            if (auto result = lexy::_detail::parse_code_point(reader);
                result.error == lexy::_detail::cp_error::eof)
            {
                // No more code points in the lexeme, finish.
                break;
            }
            else if (result.error == lexy::_detail::cp_error::success)
            {
                // Consume and visualize.
                reader.reset(result.end);
                out = visualize_to(out, lexy::code_point(result.cp), opts);
            }
            else
            {
                // Recover from the failed code point.
                auto begin = reader.position();
                lexy::_detail::recover_code_point(reader, result);
                auto end = reader.position();

                // Visualize each skipped code unit as byte.
                for (auto cur = begin; cur != end; ++cur)
                {
                    if constexpr (std::is_same_v<encoding,
                                                 lexy::utf8_encoding> //
                                  || std::is_same_v<encoding, lexy::utf8_char_encoding>)
                    {
                        out = write_escaped_byte(out, static_cast<unsigned char>(*cur & 0xFF));
                    }
                    else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
                    {
                        auto first  = static_cast<unsigned char>((*cur >> 8) & 0xFF);
                        auto second = static_cast<unsigned char>(*cur & 0xFF);

                        if (first != 0)
                            out = write_escaped_byte(out, first);
                        out = write_escaped_byte(out, second);
                    }
                    else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
                    {
                        auto first  = static_cast<unsigned char>((*cur >> 24) & 0xFF);
                        auto second = static_cast<unsigned char>((*cur >> 16) & 0xFF);
                        auto third  = static_cast<unsigned char>((*cur >> 8) & 0xFF);
                        auto fourth = static_cast<unsigned char>(*cur & 0xFF);

                        if (first != 0)
                            out = write_escaped_byte(out, first);
                        if (first != 0 || second != 0)
                            out = write_escaped_byte(out, second);
                        if (first != 0 || second != 0 || third != 0)
                            out = write_escaped_byte(out, third);
                        out = write_escaped_byte(out, fourth);
                    }
                }
            }

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (lexy::is_text_encoding<encoding>)
    {
        auto count = 0u;
        for (char c : lexeme)
        {
            // If the character is in fact ASCII, visualize the code point.
            // Otherwise, visualize as byte.
            if (lexy::_detail::is_ascii(c))
                out = visualize_to(out, lexy::code_point(static_cast<char32_t>(c)), opts);
            else
                out = write_escaped_byte(out, static_cast<unsigned char>(c));

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (lexy::is_byte_encoding<encoding>)
    {
        auto count = 0u;
        for (auto iter = lexeme.begin(); iter != lexeme.end(); ++iter)
        {
            // Write each byte.
            out = _detail::write_special_char(out, opts, [c = *iter](OutputIt out) {
                return _detail::write_format(out, "%02X", c);
            });

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (lexy::is_node_encoding<encoding>)
    {
        // Visualize as an iterator range of characters.
        lexy::range_input<typename encoding::char_encoding, typename Reader::iterator>
            input(lexeme.begin(), lexeme.end());
        return visualize_to(out, lexy::lexeme_for<decltype(input)>(input.begin(), input.end()));
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

    explicit constexpr cfile_output_iterator(std::FILE* file) : _file(file) {}

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

struct stderr_output_iterator
{
    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    stderr_output_iterator& operator=(char c)
    {
        std::fputc(c, stderr);
        return *this;
    }
};
struct stdout_output_iterator
{
    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    stdout_output_iterator& operator=(char c)
    {
        std::fputc(c, stdout);
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


//=== debug event ===//
namespace lexy::parse_events
{
/// Debug event was triggered.
/// Arguments: pos, str
struct debug
{};
} // namespace lexy::parse_events

namespace lexyd
{
template <typename CharT, CharT... C>
struct _debug : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<>;
            context.on(_ev::debug{}, reader.position(), str);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

#if LEXY_HAS_NTTP
template <lexy::_detail::string_literal Str>
constexpr auto debug = lexy::_detail::to_type_string<_debug, Str>{};
#endif

#define LEXY_DEBUG(Str)                                                                            \
    LEXY_NTTP_STRING(::lexyd::_debug, Str) {}
} // namespace lexyd

//=== trace ====//
namespace lexy::_detail
{
template <typename OutputIt, typename TokenKind>
class trace_writer
{
public:
    explicit trace_writer(OutputIt out, visualization_options opts)
    : _out(out), _opts(opts), _cur_depth(0)
    {}

    template <typename Location>
    void write_production_start(const Location& loc, const char* name)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            write_prefix(loc, prefix::event);

            _out = _detail::write_color<_detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, name);
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
    }

    template <typename Location, typename Reader>
    void write_token(const Location& loc, lexy::token_kind<TokenKind> kind,
                     lexy::lexeme<Reader> lexeme)
    {
        if (_cur_depth > _opts.max_tree_depth || (kind.ignore_if_empty() && lexeme.empty()))
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, kind.name());
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (!lexeme.empty())
        {
            _out = _detail::write_str(_out, ": ");
            _out = visualize_to(_out, lexeme, _opts | visualize_space);
        }
    }

    template <typename Location, typename Reader>
    void write_backtrack(const Location& loc, lexy::lexeme<Reader> lexeme)
    {
        if (_cur_depth > _opts.max_tree_depth || lexeme.empty())
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "backtracked");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_str(_out, ": ");

        _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
        _out = visualize_to(_out, lexeme, _opts.reset(visualize_use_color) | visualize_space);
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);
    }

    template <typename Location, typename Reader, typename Tag>
    void write_error(const Location& loc, const lexy::error<Reader, Tag>& error)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::red, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "error");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::red>(_out, _opts);
        _out = _detail::write_str(_out, ": ");

        if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                  error.length());

            _out = _detail::write_str(_out, "expected '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                  error.length());

            _out = _detail::write_str(_out, "expected keyword '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
        {
            _out = _detail::write_str(_out, "expected ");
            _out = _detail::write_str(_out, error.name());
        }
        else
        {
            _out = _detail::write_str(_out, error.message());
        }

        _out = _detail::write_color<_detail::color::reset>(_out, _opts);
    }

    template <typename Location>
    void write_recovery_start(const Location& loc)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            write_prefix(loc, prefix::event);

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
        }
        ++_cur_depth;
    }

    template <typename Location>
    void write_operation(const Location& loc, const char* name)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "operation");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_str(_out, name);
    }

    template <typename Location>
    void write_debug(const Location& loc, const char* str)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::blue, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "debug");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::blue>(_out, _opts);
        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_str(_out, str);
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);
    }

    template <typename Location>
    void write_finish(const Location& loc)
    {
        if (_cur_depth <= _opts.max_tree_depth)
            write_prefix(loc, prefix::finish);
        --_cur_depth;
    }
    template <typename Location>
    void write_cancel(const Location& loc)
    {
        if (_cur_depth <= _opts.max_tree_depth)
            write_prefix(loc, prefix::cancel);
        --_cur_depth;
    }

    OutputIt finish() &&
    {
        *_out++ = '\n';
        return _out;
    }

private:
    enum class prefix
    {
        event,
        cancel,
        finish,
    };

    template <typename Location>
    void write_prefix(const Location& loc, prefix p)
    {
        const auto use_unicode = _opts.is_set(visualize_use_unicode);

        if (_cur_depth > 0)
            *_out++ = '\n';

        _out = _detail::write_color<_detail::color::faint>(_out, _opts);
        _out = _detail::write_format(_out, "%2u:%3u", loc.line_nr(), loc.column_nr());
        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (_cur_depth > 0)
        {
            for (auto i = 0u; i != _cur_depth - 1; ++i)
                _out = _detail::write_str(_out, use_unicode ? u8"│  " : u8"  ");

            switch (p)
            {
            case prefix::event:
                _out = _detail::write_str(_out, use_unicode ? u8"├──" : u8"- ");
                break;
            case prefix::cancel:
                _out = _detail::write_str(_out, use_unicode ? u8"└" : u8"-");
                _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
                _out = _detail::write_str(_out, use_unicode ? u8"╳" : u8"x");
                _out = _detail::write_color<_detail::color::reset>(_out, _opts);
                break;
            case prefix::finish:
                _out = _detail::write_str(_out, use_unicode ? u8"┴" : u8"- finish");
                break;
            }
        }
    }

    OutputIt              _out;
    visualization_options _opts;

    std::size_t _cur_depth;
};
} // namespace lexy::_detail

namespace lexy
{
template <typename OutputIt, typename Input, typename TokenKind = void>
class _th
{
public:
    explicit _th(OutputIt out, const Input& input, visualization_options opts = {}) noexcept
    : _writer(out, opts), _input(&input), _anchor(input)
    {
        LEXY_PRECONDITION(opts.max_tree_depth <= visualization_options::max_tree_depth_limit);
    }

    class event_handler
    {
        using iterator = typename lexy::input_reader<Input>::iterator;

    public:
        constexpr event_handler(production_info info) : _info(info) {}

        void on(_th&, parse_events::grammar_start, iterator) {}
        void on(_th&, parse_events::grammar_finish, lexy::input_reader<Input>&) {}
        void on(_th&, parse_events::grammar_cancel, lexy::input_reader<Input>&) {}

        void on(_th& handler, parse_events::production_start, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_production_start(loc, _info.name);

            // All events for the production are after the initial event.
            _previous_anchor.emplace(handler._anchor);
            handler._anchor = loc.anchor();
        }
        void on(_th& handler, parse_events::production_finish, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_finish(loc);
        }
        void on(_th& handler, parse_events::production_cancel, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_cancel(loc);

            // We've backtracked, so we need to restore the anchor.
            handler._anchor = *_previous_anchor;
        }

        int on(_th& handler, parse_events::operation_chain_start, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_production_start(loc, "operation chain");
            return 0; // need to return something
        }
        template <typename Operation>
        void on(_th& handler, parse_events::operation_chain_op, Operation, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_operation(loc, lexy::production_name<Operation>());
        }
        void on(_th& handler, parse_events::operation_chain_finish, int, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_finish(loc);
        }

        template <typename TK>
        void on(_th& handler, parse_events::token, TK kind, iterator begin, iterator end)
        {
            auto loc = handler.get_location(begin);
            handler._writer.write_token(loc, token_kind<TokenKind>(kind),
                                        lexeme_for<Input>(begin, end));
        }
        void on(_th& handler, parse_events::backtracked, iterator begin, iterator end)
        {
            auto loc = handler.get_location(begin);
            handler._writer.write_backtrack(loc, lexeme_for<Input>(begin, end));
        }

        template <typename Error>
        void on(_th& handler, parse_events::error, const Error& error)
        {
            auto loc = handler.get_location(error.position());
            handler._writer.write_error(loc, error);
        }

        void on(_th& handler, parse_events::recovery_start, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_recovery_start(loc);
        }
        void on(_th& handler, parse_events::recovery_finish, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_finish(loc);
        }
        void on(_th& handler, parse_events::recovery_cancel, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_cancel(loc);
        }

        void on(_th& handler, parse_events::debug, iterator pos, const char* str)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_debug(loc, str);
        }

    private:
        production_info _info;
        // The beginning of the previous production.
        // If the current production gets canceled, it needs to be restored.
        _detail::lazy_init<input_location_anchor<Input>> _previous_anchor;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename>
    constexpr OutputIt get_result(bool) &&
    {
        return LEXY_MOV(_writer).finish();
    }

private:
    input_location<Input> get_location(typename lexy::input_reader<Input>::iterator pos)
    {
        return get_input_location(*_input, pos, _anchor);
    }

    _detail::trace_writer<OutputIt, TokenKind> _writer;

    const Input*                 _input;
    input_location_anchor<Input> _anchor;
};

template <typename State, typename Input, typename OutputIt, typename TokenKind = void>
struct trace_action
{
    OutputIt              _out;
    visualization_options _opts;
    State*                _state = nullptr;

    using handler = _th<OutputIt, Input>;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = OutputIt;

    constexpr explicit trace_action(OutputIt out, visualization_options opts = {})
    : _out(out), _opts(opts)
    {}
    template <typename U = State>
    constexpr explicit trace_action(U& state, OutputIt out, visualization_options opts = {})
    : _out(out), _opts(opts), _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        auto reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(_out, input, _opts), _state,
                                                        reader);
    }
};

template <typename Production, typename TokenKind = void, typename OutputIt, typename Input>
OutputIt trace_to(OutputIt out, const Input& input, visualization_options opts = {})
{
    return trace_action<void, Input, OutputIt, TokenKind>(out, opts)(Production{}, input);
}
template <typename Production, typename TokenKind = void, typename OutputIt, typename Input,
          typename State>
OutputIt trace_to(OutputIt out, const Input& input, State& state, visualization_options opts = {})
{
    return trace_action<State, Input, OutputIt, TokenKind>(state, out, opts)(Production{}, input);
}
template <typename Production, typename TokenKind = void, typename OutputIt, typename Input,
          typename State>
OutputIt trace_to(OutputIt out, const Input& input, const State& state,
                  visualization_options opts = {})
{
    return trace_action<const State, Input, OutputIt, TokenKind>(state, out, opts)(Production{},
                                                                                   input);
}

template <typename Production, typename TokenKind = void, typename Input>
void trace(std::FILE* file, const Input& input, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, opts);
}
template <typename Production, typename TokenKind = void, typename Input, typename State>
void trace(std::FILE* file, const Input& input, State& state, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, state, opts);
}
template <typename Production, typename TokenKind = void, typename Input, typename State>
void trace(std::FILE* file, const Input& input, const State& state, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, state, opts);
}
} // namespace lexy

#endif // LEXY_ACTION_TRACE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_HPP_INCLUDED
#define LEXY_DSL_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ASCII_HPP_INCLUDED
#define LEXY_DSL_ASCII_HPP_INCLUDED





// SWAR tricks inspired by https://garbagecollected.org/2017/01/31/four-column-ascii/.

namespace lexyd::ascii
{
//=== control ===//
struct _control : char_class_base<_control>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.control";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x00, 0x1F);
        result.insert(0x7F);
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type         = typename Encoding::char_type;
        constexpr auto mask     = lexy::_detail::swar_fill_compl(char_type(0b11111));
        constexpr auto expected = lexy::_detail::swar_fill(char_type(0b00'00000));

        // We're only checking for 0x00-0x1F, and allow a false negative for 0x7F.
        return (c & mask) == expected;
    }
};
inline constexpr auto control = _control{};

//=== whitespace ===//
struct _blank : char_class_base<_blank>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.blank";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(' ');
        result.insert('\t');
        return result;
    }
};
inline constexpr auto blank = _blank{};

struct _newline : char_class_base<_newline>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.newline";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('\n');
        result.insert('\r');
        return result;
    }
};
inline constexpr auto newline = _newline{};

struct _other_space : char_class_base<_other_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.other-space";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('\f');
        result.insert('\v');
        return result;
    }
};
inline constexpr auto other_space = _other_space{};

struct _space : char_class_base<_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.space";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(_blank::char_class_ascii());
        result.insert(_newline::char_class_ascii());
        result.insert(_other_space::char_class_ascii());
        return result;
    }
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : char_class_base<_lower>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.lower";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type = typename Encoding::char_type;

        // All interesting characters are in column 4.
        constexpr auto mask     = lexy::_detail::swar_fill_compl(char_type(0b11111));
        constexpr auto expected = lexy::_detail::swar_fill(char_type(0b11'00000));

        // But we need to eliminate ~ at the beginning and {|}~\x7F at the end.
        constexpr auto offset_low  = lexy::_detail::swar_fill(char_type(1));
        constexpr auto offset_high = lexy::_detail::swar_fill(char_type(5));

        return ((c - offset_low) & mask) == expected && ((c + offset_high) & mask) == expected;
    }
};
inline constexpr auto lower = _lower{};

struct _upper : char_class_base<_upper>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.upper";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('A', 'Z');
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type = typename Encoding::char_type;

        // All interesting characters are in column 3.
        constexpr auto mask     = lexy::_detail::swar_fill_compl(char_type(0b11111));
        constexpr auto expected = lexy::_detail::swar_fill(char_type(0b10'00000));

        // But we need to eliminate @ at the beginning and [\]^_ at the end.
        constexpr auto offset_low  = lexy::_detail::swar_fill(char_type(1));
        constexpr auto offset_high = lexy::_detail::swar_fill(char_type(5));

        return ((c - offset_low) & mask) == expected && ((c + offset_high) & mask) == expected;
    }
};
inline constexpr auto upper = _upper{};

struct _alpha : char_class_base<_alpha>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.alpha";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        result.insert('A', 'Z');
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        // We're assuming lower characters are more common, so do the efficient check only for them.
        return _lower::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto alpha = _alpha{};

struct _alphau : char_class_base<_alphau>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.alpha-underscore";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        result.insert('A', 'Z');
        result.insert('_');
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        // We're assuming alpha characters are more common, so do the efficient check only for them.
        return _alpha::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto alpha_underscore = _alphau{};

//=== digit ===//
struct _digit : char_class_base<_digit>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.digit";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type = typename Encoding::char_type;

        // All interesting characters are in the second half of column 1.
        constexpr auto mask     = lexy::_detail::swar_fill_compl(char_type(0b01111));
        constexpr auto expected = lexy::_detail::swar_fill(char_type(0b01'10000));

        // But we need to eliminate :;<=>? at the end.
        constexpr auto offset_high = lexy::_detail::swar_fill(char_type(6));

        return (c & mask) == expected && ((c + offset_high) & mask) == expected;
    }
};
inline constexpr auto digit = _digit{};

struct _alnum : char_class_base<_alnum>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.alpha-digit";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(_alpha::char_class_ascii());
        result.insert(_digit::char_class_ascii());
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        // We're assuming alpha characters are more common, so do the efficient check only for them.
        return _alpha::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = _alnum{};

struct _word : char_class_base<_word>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.word";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(_alphau::char_class_ascii());
        result.insert(_digit::char_class_ascii());
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        // We're assuming alphau characters are more common, so do the efficient check only for
        // them.
        return _alphau::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto word                   = _word{};
inline constexpr auto alpha_digit_underscore = _word{};

//=== punct ===//
struct _punct : char_class_base<_punct>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.punct";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('!');
        result.insert('"');
        result.insert('#');
        result.insert('$');
        result.insert('%');
        result.insert('&');
        result.insert('\'');
        result.insert('(');
        result.insert(')');
        result.insert('*');
        result.insert('+');
        result.insert(',');
        result.insert('-');
        result.insert('.');
        result.insert('/');
        result.insert(':');
        result.insert(';');
        result.insert('<');
        result.insert('=');
        result.insert('>');
        result.insert('?');
        result.insert('@');
        result.insert('[');
        result.insert('\\');
        result.insert(']');
        result.insert('^');
        result.insert('_');
        result.insert('`');
        result.insert('{');
        result.insert('|');
        result.insert('}');
        result.insert('~');
        return result;
    }
};
inline constexpr auto punct = _punct{};

//=== categories ===//
struct _graph : char_class_base<_graph>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.graph";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x21, 0x7E);
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type = typename Encoding::char_type;

        // First check that we have only ASCII, but shifted by one, so we also exclude 0x7F.
        constexpr auto ascii_mask     = lexy::_detail::swar_fill_compl(char_type(0b11'11111));
        constexpr auto ascii_offset   = lexy::_detail::swar_fill(char_type(1));
        constexpr auto ascii_expected = lexy::_detail::swar_fill(char_type(0));
        if (((c + ascii_offset) & ascii_mask) != ascii_expected)
            return false;

        // The above check also included 0xFF for single byte encodings where it overflowed,
        // so do a separate check in those cases.
        if constexpr (sizeof(char_type) == 1)
        {
            if ((c & ascii_mask) != ascii_expected)
                return false;
        }

        // Then we must not have a character in column 0, or space.
        // If we subtract one we turn 0x21-0x01 into column 0 and 0x00 to a value definitely not in
        // column 0, so need to check both.
        constexpr auto mask       = lexy::_detail::swar_fill_compl(char_type(0b11111));
        constexpr auto offset_low = lexy::_detail::swar_fill(char_type(1));
        return !lexy::_detail::swar_has_zero<char_type>(c & mask)
               && !lexy::_detail::swar_has_zero<char_type>((c - offset_low) & mask);
    }
};
inline constexpr auto graph = _graph{};

struct _print : char_class_base<_print>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.print";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x20, 0x7E);
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type = typename Encoding::char_type;

        // First check that we have only ASCII, but shifted by one, so we also exclude 0x7F.
        constexpr auto ascii_mask     = lexy::_detail::swar_fill_compl(char_type(0b11'11111));
        constexpr auto ascii_offset   = lexy::_detail::swar_fill(char_type(1));
        constexpr auto ascii_expected = lexy::_detail::swar_fill(char_type(0));
        if (((c + ascii_offset) & ascii_mask) != ascii_expected)
            return false;

        // The above check also included 0xFF for single byte encodings where it overflowed,
        // so do a separate check in those cases.
        if constexpr (sizeof(char_type) == 1)
        {
            if ((c & ascii_mask) != ascii_expected)
                return false;
        }

        // Then we must not have a character in column 0.
        constexpr auto mask = lexy::_detail::swar_fill_compl(char_type(0b11111));
        return !lexy::_detail::swar_has_zero<char_type>(c & mask);
    }
};
inline constexpr auto print = _print{};

struct _char : char_class_base<_char>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x00, 0x7F);
        return result;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        using char_type = typename Encoding::char_type;

        constexpr auto mask     = lexy::_detail::swar_fill_compl(char_type(0b11'11111));
        constexpr auto expected = lexy::_detail::swar_fill(char_type(0));

        return (c & mask) == expected;
    }
};
inline constexpr auto character = _char{};
} // namespace lexyd::ascii

namespace lexyd::ascii
{
template <char... C>
struct _alt : char_class_base<_alt<C...>>
{
    static_assert(sizeof...(C) > 0);

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return lexy::_detail::type_string<char, C...>::template c_str<char>;
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        (result.insert(C), ...);
        return result;
    }
};

template <typename CharT, CharT... C>
struct _one_of
{
    static_assert((std::is_same_v<CharT, char> && ... && lexy::_detail::is_ascii(C)),
                  "only ASCII characters are supported");

    using rule = _alt<C...>;
};

#if LEXY_HAS_NTTP
/// Matches one of the ASCII characters.
template <lexy::_detail::string_literal Str>
constexpr auto one_of = typename lexy::_detail::to_type_string<_one_of, Str>::rule{};
#endif

#define LEXY_ASCII_ONE_OF(Str)                                                                     \
    LEXY_NTTP_STRING(::lexyd::ascii::_one_of, Str)::rule {}
} // namespace lexyd::ascii

#endif // LEXY_DSL_ASCII_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BITS_HPP_INCLUDED
#define LEXY_DSL_BITS_HPP_INCLUDED




//=== bit rules ===//
namespace lexyd::bit
{
struct _bit_pattern
{
    unsigned mask;
    unsigned value;
};

struct _bit_rule
{};

struct _b0 : _bit_rule
{
    static constexpr auto size = 1u;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= 1;
        p.value <<= 1;

        p.mask |= 1;
    }
};

/// Matches a 0 bit.
inline constexpr auto _0 = _b0{};

struct _b1 : _bit_rule
{
    static constexpr auto size = 1u;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= 1;
        p.value <<= 1;

        p.mask |= 1;
        p.value |= 1;
    }
};

/// Matches a 1 bit.
inline constexpr auto _1 = _b1{};

template <unsigned Value>
struct _n : _bit_rule
{
    static_assert(Value <= 0xF);

    static constexpr auto size = 4u;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= 4;
        p.value <<= 4;

        p.mask |= 0b1111;
        p.value |= Value;
    }
};

/// Matches a specific nibble, i.e. four bits.
template <unsigned Value>
constexpr auto nibble = _n<Value>{};

template <unsigned N>
struct _b : _bit_rule
{
    static_assert(N > 0);

    static constexpr auto size = N;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= N;
        p.value <<= N;
    }
};

/// Matches any bit.
inline constexpr auto _ = _b<1>{};

/// Matches N arbitrary bits.
template <unsigned N>
constexpr auto any = _b<N>{};
} // namespace lexyd::bit

//=== bits ===//
namespace lexyd
{
template <unsigned Mask, unsigned Value>
struct _bits : token_base<_bits<Mask, Value>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            static_assert(lexy::is_byte_encoding<typename Reader::encoding>);

            auto byte = reader.peek();
            if (byte == Reader::encoding::eof()
                || ((static_cast<unsigned char>(byte) & Mask) != Value))
                return false;

            reader.bump();
            end = reader.current();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(), "bits");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches the specific bit pattern.
template <typename... Bits>
constexpr auto bits(Bits...)
{
    static_assert((std::is_base_of_v<bit::_bit_rule, Bits> && ...), "bits() requires bit rules");
    static_assert((0 + ... + Bits::size) == 8, "must specify 8 bit at a time");

    constexpr auto pattern = [] {
        bit::_bit_pattern result{0, 0};
        (Bits::apply(result), ...);
        return result;
    }();

    return _bits<pattern.mask, pattern.value>{};
}
} // namespace lexyd

#endif // LEXY_DSL_BITS_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED





namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom : _lit<unsigned char>
{};
template <lexy::encoding_endianness DontCare>
struct _bom<lexy::utf8_encoding, DontCare> //
: _lit<unsigned char, 0xEF, 0xBB, 0xBF>
{};
template <lexy::encoding_endianness DontCare>
struct _bom<lexy::utf8_char_encoding, DontCare> //
: _lit<unsigned char, 0xEF, 0xBB, 0xBF>
{};
template <>
struct _bom<lexy::utf16_encoding, lexy::encoding_endianness::little>
: _lit<unsigned char, 0xFF, 0xFE>
{};
template <>
struct _bom<lexy::utf16_encoding, lexy::encoding_endianness::big> //
: _lit<unsigned char, 0xFE, 0xFF>
{};
template <>
struct _bom<lexy::utf32_encoding, lexy::encoding_endianness::little>
: _lit<unsigned char, 0xFF, 0xFE, 0x00, 0x00>
{};
template <>
struct _bom<lexy::utf32_encoding, lexy::encoding_endianness::big>
: _lit<unsigned char, 0x00, 0x00, 0xFE, 0xFF>
{};

/// The BOM for that particular encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = _bom<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BRACKETS_HPP_INCLUDED
#define LEXY_DSL_BRACKETS_HPP_INCLUDED



// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_TERMINATOR_HPP_INCLUDED
#define LEXY_DSL_TERMINATOR_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BRANCH_HPP_INCLUDED
#define LEXY_DSL_BRANCH_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
    using p = NextParser;
};
template <typename R1>
struct _seq_impl<R1>
{
    template <typename NextParser>
    using p = lexy::parser_for<R1, NextParser>;
};
template <typename R1, typename R2>
struct _seq_impl<R1, R2>
{
    template <typename NextParser>
    using p = lexy::parser_for<R1, lexy::parser_for<R2, NextParser>>;
};
template <typename R1, typename R2, typename R3>
struct _seq_impl<R1, R2, R3>
{
    template <typename NextParser>
    using p = lexy::parser_for<R1, lexy::parser_for<R2, lexy::parser_for<R3, NextParser>>>;
};
template <typename R1, typename R2, typename R3, typename R4>
struct _seq_impl<R1, R2, R3, R4>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1, lexy::parser_for<R2, lexy::parser_for<R3, lexy::parser_for<R4, NextParser>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5>
struct _seq_impl<R1, R2, R3, R4, R5>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1, lexy::parser_for<
                R2, lexy::parser_for<R3, lexy::parser_for<R4, lexy::parser_for<R5, NextParser>>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5, typename R6>
struct _seq_impl<R1, R2, R3, R4, R5, R6>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1,
        lexy::parser_for<
            R2,
            lexy::parser_for<
                R3, lexy::parser_for<R4, lexy::parser_for<R5, lexy::parser_for<R6, NextParser>>>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5, typename R6, typename R7>
struct _seq_impl<R1, R2, R3, R4, R5, R6, R7>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1,
        lexy::parser_for<
            R2, lexy::parser_for<
                    R3, lexy::parser_for<
                            R4, lexy::parser_for<
                                    R5, lexy::parser_for<R6, lexy::parser_for<R7, NextParser>>>>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5, typename R6, typename R7,
          typename... T>
struct _seq_impl<R1, R2, R3, R4, R5, R6, R7, T...>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1,
        lexy::parser_for<
            R2,
            lexy::parser_for<
                R3,
                lexy::parser_for<
                    R4, lexy::parser_for<
                            R5, lexy::parser_for<
                                    R6, lexy::parser_for<R7, lexy::parser_for<_seq_impl<T...>,
                                                                              NextParser>>>>>>>>;
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);

    template <typename NextParser>
    using p = lexy::parser_for<_seq_impl<R...>, NextParser>;
};

template <typename R, typename S, typename = std::enable_if_t<lexy::is_rule<R> && lexy::is_rule<S>>>
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
struct _br : _copy_base<Condition>
{
    static_assert(sizeof...(R) >= 0);

    template <typename NextParser>
    using _pc = lexy::parser_for<_seq_impl<R...>, NextParser>;

    // We parse Condition and then seq<R...>.
    // Condition's try_parse() checks the branch condition, which is what we want.
    template <typename Reader>
    using bp = lexy::continuation_branch_parser<Condition, Reader, _pc>;

    template <typename NextParser>
    using p = lexy::parser_for<_seq_impl<Condition, R...>, NextParser>;
};

//=== operator>> ===//
/// Parses `Then` only after `Condition` has matched.
template <typename Condition, typename Then>
constexpr auto operator>>(Condition, Then)
{
    LEXY_REQUIRE_BRANCH_RULE(Condition, "Left-hand-side of >>");
    return _br<Condition, Then>{};
}
template <typename Condition, typename... R>
constexpr auto operator>>(Condition, _seq<R...>)
{
    LEXY_REQUIRE_BRANCH_RULE(Condition, "Left-hand-side of >>");
    return _br<Condition, R...>{};
}
template <typename Condition, typename C, typename... R>
constexpr auto operator>>(Condition, _br<C, R...>)
{
    LEXY_REQUIRE_BRANCH_RULE(Condition, "Left-hand-side of >>");
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

template <typename Condition, typename Then>
constexpr auto _maybe_branch(Condition condition, Then then)
{
    if constexpr (lexy::is_branch_rule<Condition>)
        return condition >> then;
    else
        return condition + then;
}
} // namespace lexyd

namespace lexyd
{
struct _else : unconditional_branch_base
{
    template <typename NextParser>
    using p = NextParser;

    template <typename Reader>
    using bp = lexy::unconditional_branch_parser<_else, Reader>;
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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED





namespace lexyd
{
template <typename Rule>
struct _err_production
{
    static constexpr auto name                = "<error>";
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule                = Rule{};
};

template <typename Tag, typename Rule>
struct _err : unconditional_branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&...)
        {
            auto begin = reader.position();
            auto end   = reader.position();
            if constexpr (!std::is_same_v<Rule, void>)
            {
                auto backtrack = reader.current();

                // We match a dummy production that only consists of the rule.
                lexy::do_action<
                    _err_production<Rule>,
                    lexy::match_action<void, Reader>::template result_type>(lexy::_mh(),
                                                                            context.control_block
                                                                                ->parse_state,
                                                                            reader);
                end = reader.position();
                reader.reset(LEXY_MOV(backtrack));
            }

            auto err = lexy::error<Reader, Tag>(begin, end);
            context.on(_ev::error{}, err);
            return false;
        }
    };
    template <typename Reader>
    using bp = lexy::unconditional_branch_parser<_err, Reader>;

    /// Adds a rule whose match will be part of the error location.
    template <typename R>
    constexpr auto operator()(R) const
    {
        return _err<Tag, R>{};
    }
};

/// Matches nothing, produces an error with the given tag.
template <typename Tag>
constexpr auto error = _err<Tag, void>{};
} // namespace lexyd

namespace lexyd
{
template <typename Branch, typename Error>
struct _must : branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Try and parse the branch.
            lexy::branch_parser_for<Branch, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            branch.cancel(context);

            // The branch wasn't taken, so we fail with the specific error by parsing Error.
            auto result = lexy::parser_for<Error, lexy::pattern_parser<>>::parse(context, reader);
            LEXY_ASSERT(!result, "error must not recover");

            return false;
        }
    };

    // As a branch we parse it exactly the same.
    template <typename Reader>
    using bp = lexy::branch_parser_for<Branch, Reader>;
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
    LEXY_REQUIRE_BRANCH_RULE(Branch, "must()");
    static_assert(!lexy::is_unconditional_branch_rule<Branch>);
    return _must_dsl<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_RECOVER_HPP_INCLUDED
#define LEXY_DSL_RECOVER_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CHOICE_HPP_INCLUDED
#define LEXY_DSL_CHOICE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_TUPLE_HPP_INCLUDED
#define LEXY_DETAIL_TUPLE_HPP_INCLUDED




namespace lexy::_detail
{
template <std::size_t Idx, typename T>
struct _tuple_holder
{
#if !defined(__GNUC__) || defined(__clang__)
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105795
    LEXY_EMPTY_MEMBER
#endif
    T value;
};

template <std::size_t Idx, typename... T>
struct _nth_type;
template <std::size_t Idx, typename H, typename... T>
struct _nth_type<Idx, H, T...>
{
    using type = typename _nth_type<Idx - 1, T...>::type;
};
template <typename H, typename... T>
struct _nth_type<0, H, T...>
{
    using type = H;
};

template <typename T>
struct _tuple_get_type
{
    using type = T&;
};
template <typename T>
struct _tuple_get_type<T&&>
{
    using type = T&&;
};

template <typename Indices, typename... T>
class _tuple;
template <std::size_t... Idx, typename... T>
class _tuple<index_sequence<Idx...>, T...> : public _tuple_holder<Idx, T>...
{
public:
    constexpr _tuple() = default;

    template <typename... Args>
    constexpr _tuple(Args&&... args) : _tuple_holder<Idx, T>{LEXY_FWD(args)}...
    {}
};

template <typename... T>
struct tuple : _tuple<index_sequence_for<T...>, T...>
{
    constexpr tuple() = default;

    template <typename... Args>
    constexpr explicit tuple(Args&&... args)
    : _tuple<index_sequence_for<T...>, T...>(LEXY_FWD(args)...)
    {}

    template <std::size_t N>
    using element_type = typename _nth_type<N, T...>::type;

    template <std::size_t N>
    constexpr decltype(auto) get() noexcept
    {
        // NOLINTNEXTLINE: this is fine.
        auto&& holder = static_cast<_tuple_holder<N, element_type<N>>&>(*this);
        // NOLINTNEXTLINE
        return static_cast<typename _tuple_get_type<element_type<N>>::type>(holder.value);
    }
    template <std::size_t N>
    constexpr decltype(auto) get() const noexcept
    {
        // NOLINTNEXTLINE: this is fine.
        auto&& holder = static_cast<const _tuple_holder<N, element_type<N>>&>(*this);
        // NOLINTNEXTLINE
        return static_cast<typename _tuple_get_type<const element_type<N>>::type>(holder.value);
    }

    static constexpr auto index_sequence()
    {
        return index_sequence_for<T...>{};
    }
};
template <>
struct tuple<>
{
    constexpr tuple() = default;

    static constexpr auto index_sequence()
    {
        return index_sequence_for<>{};
    }
};

template <typename... Args>
constexpr auto make_tuple(Args&&... args)
{
    return tuple<std::decay_t<Args>...>(LEXY_FWD(args)...);
}

template <typename... Args>
constexpr auto forward_as_tuple(Args&&... args)
{
    return tuple<Args&&...>(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TUPLE_HPP_INCLUDED




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
template <typename... R>
struct _chc
// Only make it a branch rule if it doesn't have an unconditional branch.
// A choice rule with an unconditional branch is itself an unconditional branch, which is most
// likely a bug.
: std::conditional_t<(lexy::is_unconditional_branch_rule<R> || ...), rule_base, branch_base>
{
    static constexpr auto _any_unconditional = (lexy::is_unconditional_branch_rule<R> || ...);

    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<sizeof...(R)>>
    struct bp;
    template <typename Reader, std::size_t... Idx>
    struct bp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        template <typename Rule>
        using rp = lexy::branch_parser_for<Rule, Reader>;

        lexy::_detail::tuple<rp<R>...> r_parsers;
        std::size_t                    branch_idx;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
            -> std::conditional_t<_any_unconditional, std::true_type, bool>
        {
            auto try_r = [&](std::size_t idx, auto& parser) {
                if (!parser.try_parse(cb, reader))
                    return false;

                branch_idx = idx;
                return true;
            };

            // Need to try each possible branch.
            auto found_branch = (try_r(Idx, r_parsers.template get<Idx>()) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return {};
            }
            else
            {
                return found_branch;
            }
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // Need to cancel all branches.
            (r_parsers.template get<Idx>().cancel(context), ...);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Need to call finish on the selected branch, and cancel on all others before that.
            auto result = false;
            (void)((Idx == branch_idx
                        ? (result
                           = r_parsers.template get<Idx>()
                                 .template finish<NextParser>(context, reader, LEXY_FWD(args)...),
                           true)
                        : (r_parsers.template get<Idx>().cancel(context), false))
                   || ...);
            return result;
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto result = false;
            auto try_r  = [&](auto&& parser) {
                if (!parser.try_parse(context.control_block, reader))
                {
                    parser.cancel(context);
                    return false;
                }

                // LEXY_FWD(args) will break MSVC builds targeting C++17.
                result = parser.template finish<NextParser>(context, reader,
                                                             static_cast<Args&&>(args)...);
                return true;
            };

            // Try to parse each branch in order.
            auto found_branch = (try_r(lexy::branch_parser_for<R, Reader>{}) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return result;
            }
            else
            {
                if (found_branch)
                    return result;

                auto err = lexy::error<Reader, lexy::exhausted_choice>(reader.position());
                context.on(_ev::error{}, err);
                return false;
            }
        }
    };
};

template <typename R, typename S>
constexpr auto operator|(R, S)
{
    LEXY_REQUIRE_BRANCH_RULE(R, "choice");
    LEXY_REQUIRE_BRANCH_RULE(S, "choice");
    return _chc<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator|(_chc<R...>, S)
{
    LEXY_REQUIRE_BRANCH_RULE(S, "choice");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator|(R, _chc<S...>)
{
    LEXY_REQUIRE_BRANCH_RULE(R, "choice");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
template <typename Encoding, typename... Needle, typename... End>
LEXY_CONSTEVAL auto _build_look_trie(_lset<Needle...>, _lset<End...>)
{
    auto result     = lexy::_detail::make_empty_trie<Encoding, Needle..., End...>();
    auto char_class = std::size_t(0);

    // We insert all needles with value 0.
    ((result.node_value[Needle::lit_insert(result, 0, char_class)] = 0,
      char_class += Needle::lit_char_classes.size),
     ...);

    // And all ends with value 1.
    ((result.node_value[End::lit_insert(result, 0, char_class)] = 1,
      char_class += End::lit_char_classes.size),
     ...);

    return result;
}
template <typename Encoding, typename Needle, typename End>
static constexpr auto _look_trie
    = _build_look_trie<Encoding>(typename Needle::as_lset{}, typename End::as_lset{});

template <typename Needle, typename End, typename Tag>
struct _look : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);

        typename Reader::iterator begin;
        typename Reader::iterator end;

        constexpr bool try_parse(const void*, Reader reader)
        {
            begin = reader.position();

            auto result = [&] {
                using matcher = lexy::_detail::lit_trie_matcher<
                    _look_trie<typename Reader::encoding, Needle, End>, 0>;

                while (true)
                {
                    auto result = matcher::try_match(reader);
                    if (result == 0)
                        // We've found the needle.
                        return true;
                    else if (result == 1 || reader.peek() == Reader::encoding::eof())
                        // We've failed.
                        return false;
                    else
                        // Try again.
                        reader.bump();
                }

                return false; // unreachable
            }();

            end = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end);
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            context.on(_ev::backtracked{}, impl.begin, impl.end);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _look<Needle, End, Error> error = {};
};

/// Looks for the Needle before End.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
constexpr auto lookahead(Needle _needle, End _end)
{
    auto needle = literal_set() / _needle;
    auto end    = literal_set() / _end;
    return _look<decltype(needle), decltype(end), void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED


namespace lexyd
{
// Indicates that this rule already generates the recovery events.
struct _recovery_base : rule_base
{};

// Parses the rule but generates the appropriate recovery_start/finish/cancel events.
template <typename Rule>
struct _recovery_wrapper : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                               bool& recovery_finished, Args&&... args)
            {
                recovery_finished = true;
                context.on(_ev::recovery_finish{}, reader.position());
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::recovery_start{}, reader.position());
            auto recovery_finished = false;

            // As part of the recovery, we parse the rule and whitespace.
            using parser = lexy::parser_for<Rule, lexy::whitespace_parser<Context, _continuation>>;
            auto result  = parser::parse(context, reader, recovery_finished, LEXY_FWD(args)...);

            if (!recovery_finished)
                context.on(_ev::recovery_cancel{}, reader.position());
            return result;
        }
    };
};

struct _noop_recovery : rule_base
{
    template <typename NextParser>
    using p = NextParser;
};
} // namespace lexyd

namespace lexyd
{
template <typename Token, typename Limit>
struct _find : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Note that we can't use lookahead() directly as it's end position includes Needle/End,
            // here we want to exclude it, however.
            constexpr const auto& trie
                = _look_trie<typename Reader::encoding, Token, decltype(get_limit())>;
            using matcher = lexy::_detail::lit_trie_matcher<trie, 0>;

            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);
            while (true)
            {
                auto end    = reader.current(); // *before* we've consumed Token/Limit
                auto result = matcher::try_match(reader);
                if (result == 0)
                {
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end.position());
                    context.on(_ev::recovery_finish{}, end.position());
                    reader.reset(end); // reset to before the token
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                }
                else if (result == 1 || reader.peek() == Reader::encoding::eof())
                {
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end.position());
                    context.on(_ev::recovery_cancel{}, end.position());
                    reader.reset(end); // reset to before the limit
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            // unreachable
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting literal tokens is found first.
    template <typename... Literals>
    constexpr auto limit(Literals... literals) const
    {
        static_assert(sizeof...(Literals) > 0);

        auto l = (get_limit() / ... / literals);
        return _find<Token, decltype(l)>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return literal_set();
        else
            return Limit{};
    }
};

/// Recovers once it finds one of the given literal tokens (without consuming them).
template <typename... Literals>
constexpr auto find(Literals... literals)
{
    static_assert(sizeof...(Literals) > 0);

    auto needle = (literal_set() / ... / literals);
    return _find<decltype(needle), void>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Limit, typename... R>
struct _reco : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);

            // Try to match one of the recovery rules.
            lexy::branch_parser_for<decltype((R{} | ...)), Reader> recovery{};
            while (!recovery.try_parse(context.control_block, reader))
            {
                recovery.cancel(context);

                if (lexy::token_parser_for<decltype(get_limit()), Reader> limit(reader);
                    limit.try_parse(reader) || reader.peek() == Reader::encoding::eof())
                {
                    // We've failed to recover as we've reached the limit.
                    auto end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                    context.on(_ev::recovery_cancel{}, end);
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            auto end = reader.position();
            context.on(_ev::token{}, lexy::error_token_kind, begin, end);
            context.on(_ev::recovery_finish{}, end);

            // Finish with the rule that matched.
            return recovery.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting literal tokens is found first.
    template <typename... Literals>
    constexpr auto limit(Literals... literals) const
    {
        static_assert(sizeof...(Literals) > 0);

        auto l = (get_limit() / ... / literals);
        return _reco<decltype(l), R...>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return literal_set();
        else
            return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
constexpr auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    LEXY_REQUIRE_BRANCH_RULE(Branches..., "recover");
    return _reco<void, Branches...>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Terminator, typename Rule, typename Recover>
struct _tryt : rule_base
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           bool& continuation_reached, Args&&... args)
        {
            continuation_reached = true;

            // We need to parse the terminator on success as well, if we have one.
            if constexpr (std::is_void_v<Terminator>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return lexy::parser_for<Terminator, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool recover(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<Recover>)
            {
                using recovery_rule = _recovery_wrapper<_noop_recovery>;
                return lexy::parser_for<recovery_rule, NextParser>::parse(context, reader,
                                                                          LEXY_FWD(args)...);
            }
            else if constexpr (std::is_base_of_v<_recovery_base, Recover>)
            {
                using recovery_rule = Recover;
                return lexy::parser_for<recovery_rule, NextParser>::parse(context, reader,
                                                                          LEXY_FWD(args)...);
            }
            else
            {
                using recovery_rule = _recovery_wrapper<Recover>;
                return lexy::parser_for<recovery_rule, NextParser>::parse(context, reader,
                                                                          LEXY_FWD(args)...);
            }
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using parser = lexy::parser_for<Rule, _pc<NextParser>>;

            // Parse the rule and check whether it reached the continuation.
            auto continuation_reached = false;
            auto result = parser::parse(context, reader, continuation_reached, LEXY_FWD(args)...);
            if (continuation_reached)
                // Whatever happened, it is not our problem as we've reached the continuation.
                return result;

            // We haven't reached the continuation, so need to recover.
            LEXY_ASSERT(!result, "we've failed without reaching the continuation?!");
            return _pc<NextParser>::recover(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Rule, typename Recover>
struct _tryr : _copy_base<Rule>
{
    using impl = _tryt<void, Rule, Recover>;

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule and check whether it reached the continuation.
            using continuation        = typename impl::template _pc<NextParser>;
            auto continuation_reached = false;
            auto result = rule.template finish<continuation>(context, reader, continuation_reached,
                                                             LEXY_FWD(args)...);
            if (continuation_reached)
                // Whatever happened, it is not our problem as we've reached the continuation.
                return result;

            // We haven't reached the continuation, so need to recover.
            LEXY_ASSERT(!result, "we've failed without reaching the continuation?!");
            return continuation::recover(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p : lexy::parser_for<impl, NextParser>
    {};
};

/// Parses Rule, if that fails, continues immediately.
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


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED





// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LOOP_HPP_INCLUDED
#define LEXY_DSL_LOOP_HPP_INCLUDED




namespace lexyd
{
struct _break : unconditional_branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename LoopControl, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context&, Reader&, LoopControl& cntrl, Args&&...)
        {
            cntrl.loop_break = true;
            return true;
        }
    };

    template <typename Reader>
    using bp = lexy::unconditional_branch_parser<_break, Reader>;
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            struct loop_control_t
            {
                bool loop_break = false;
            } control;

            while (!control.loop_break)
            {
                using parser = lexy::parser_for<Rule, lexy::pattern_parser<loop_control_t>>;
                if (!parser::parse(context, reader, control))
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Branch, Reader> branch{};
            while (branch.try_parse(context.control_block, reader))
            {
                if (!branch.template finish<lexy::pattern_parser<>>(context, reader))
                    return false;
            }
            branch.cancel(context);

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches the branch rule as often as possible.
template <typename Rule>
constexpr auto while_(Rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "while()");
    return _whl<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the rule at least once, then as often as possible.
template <typename Rule>
constexpr auto while_one(Rule rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "while_one()");
    return rule >> while_(rule);
}
} // namespace lexyd

namespace lexyd
{
/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
constexpr auto do_while(Then then, Condition condition)
{
    return _maybe_branch(then, while_(condition >> then));
}
} // namespace lexyd

#endif // LEXY_DSL_LOOP_HPP_INCLUDED



namespace lexyd
{
template <typename Rule>
struct _wsr;
}

//=== implementation ===//
namespace lexy::_detail
{
// Parse this production to skip whitespace.
template <typename WhitespaceRule>
struct ws_production
{
    static constexpr auto name                = "<whitespace>";
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule = lexy::dsl::loop(WhitespaceRule{} | lexy::dsl::break_);
};
// If the whitespace rule itself uses `dsl::whitespace`, strip it to avoid infinite recursion.
template <typename Rule>
struct ws_production<lexyd::_wsr<Rule>> : ws_production<Rule>
{};

// A special handler for parsing whitespace.
// It only forwards error events and ignores all others.
template <typename Handler>
class ws_handler
{
public:
    constexpr explicit ws_handler(Handler& handler, typename Handler::event_handler& evh)
    : _handler(&handler), _event_handler(&evh)
    {}
    template <typename Context>
    constexpr explicit ws_handler(Context& context)
    : ws_handler(context.control_block->parse_handler, context.handler)
    {}

    class event_handler
    {
    public:
        template <typename Rule>
        constexpr event_handler(ws_production<Rule>)
        {}
        template <typename Production>
        constexpr event_handler(Production)
        {
            static_assert(_detail::error<Production>,
                          "whitespace rule must not contain `dsl::p` or `dsl::recurse`;"
                          "use `dsl::inline_` instead");
        }

        template <typename Error>
        constexpr void on(ws_handler& handler, parse_events::error ev, const Error& error)
        {
            handler._event_handler->on(*handler._handler, ev, error);
        }

        template <typename Event, typename... Args>
        constexpr int on(ws_handler&, Event, const Args&...)
        {
            return 0; // an operation_start event returns something
        }
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename>
    constexpr bool get_result(bool rule_parse_result) &&
    {
        return rule_parse_result;
    }

    template <typename Event, typename... Args>
    constexpr auto real_on(Event ev, Args&&... args)
    {
        return _event_handler->on(*_handler, ev, LEXY_FWD(args)...);
    }

private:
    Handler*                         _handler;
    typename Handler::event_handler* _event_handler;
};
template <typename Context>
ws_handler(Context& context) -> ws_handler<typename Context::handler_type>;

template <typename>
using ws_result = bool;

template <typename WhitespaceRule>
constexpr bool space_is_definitely_whitespace()
{
    if constexpr (lexy::is_char_class_rule<WhitespaceRule>)
        return WhitespaceRule::char_class_ascii().contains[int(' ')];
    else
        return false;
}

template <typename WhitespaceRule, typename Handler, typename Reader>
constexpr auto skip_whitespace(ws_handler<Handler>&& handler, Reader& reader)
{
    auto begin = reader.position();

    if constexpr (lexy::is_token_rule<WhitespaceRule>)
    {
        // Parsing a token repeatedly cannot fail, so we can optimize it.

        if constexpr (_detail::is_swar_reader<Reader> //
                      && space_is_definitely_whitespace<WhitespaceRule>())
        {
            while (true)
            {
                // Skip as many spaces as possible.
                using char_type = typename Reader::encoding::char_type;
                while (reader.peek_swar() == _detail::swar_fill(char_type(' ')))
                    reader.bump_swar();

                // We no longer have a space, skip the entire whitespace rule once.
                if (!lexy::try_match_token(WhitespaceRule{}, reader))
                    // If that fails, we definitely have no more whitespace.
                    break;
            }
        }
        else
        {
            // Without SWAR, we just repeatedly skip the whitespace rule.
            while (lexy::try_match_token(WhitespaceRule{}, reader))
            {
            }
        }

        handler.real_on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin,
                        reader.position());
        return std::true_type{};
    }
    else if constexpr (!std::is_void_v<WhitespaceRule>)
    {
        using production = ws_production<WhitespaceRule>;

        // Parse the production using a special handler that only forwards errors.
        auto result = lexy::do_action<production, ws_result>(LEXY_MOV(handler),
                                                             lexy::no_parse_state, reader);

        handler.real_on(lexy::parse_events::token{},
                        result ? lexy::whitespace_token_kind : lexy::error_token_kind, begin,
                        reader.position());
        return result;
    }
    else
    {
        (void)handler;
        (void)reader;
        (void)begin;
        return std::true_type{};
    }
}

// Inherit from it in a continuation to disable automatic whitespace skipping.
struct disable_whitespace_skipping
{};

template <typename NextParser>
struct automatic_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if (!std::is_base_of_v<disable_whitespace_skipping, NextParser> //
            && context.control_block->enable_whitespace_skipping)
        {
            using whitespace = lexy::production_whitespace<typename Context::production,
                                                           typename Context::whitespace_production>;
            if (!skip_whitespace<whitespace>(ws_handler(context), reader))
                return false;
        }

        return NextParser::parse(context, reader, LEXY_FWD(args)...);
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto result
                = lexy::_detail::skip_whitespace<Rule>(lexy::_detail::ws_handler(context), reader);
            if (!result)
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

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
};

template <typename Rule>
constexpr auto whitespace(Rule)
{
    return _wsr<Rule>{};
}
} // namespace lexyd

//=== no_whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsn : _copy_base<Rule>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Enable automatic whitespace skipping again.
            context.control_block->enable_whitespace_skipping = true;
            // And skip whitespace once.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            // Note that this can't skip whitespace as there is no way to access the whitespace
            // rule. We thus don't need to disable anything.
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule with whitespace skipping disabled.
            context.control_block->enable_whitespace_skipping = false;
            return rule.template finish<_pc<NextParser>>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using whitespace = lexy::production_whitespace<typename Context::production,
                                                           typename Context::whitespace_production>;
            if constexpr (std::is_void_v<whitespace>)
            {
                // No whitespace, just parse the rule normally.
                return lexy::parser_for<Rule, NextParser>::parse(context, reader,
                                                                 LEXY_FWD(args)...);
            }
            else
            {
                // Parse the rule with whitespace skipping disabled.
                context.control_block->enable_whitespace_skipping = false;
                using parser = lexy::parser_for<Rule, _pc<NextParser>>;
                return parser::parse(context, reader, LEXY_FWD(args)...);
            }
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
template <typename Terminator, typename Rule>
struct _optt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _lstt;
template <typename Terminator, typename R, typename Sep, typename Recover>
struct _olstt;

template <typename Terminator, typename RecoveryLimit = void>
struct _term
{
    /// Adds the literal tokens to the recovery limit.
    template <typename... Literals>
    constexpr auto limit(Literals... literals) const
    {
        static_assert(sizeof...(Literals) > 0);

        auto l = (recovery_rule().get_limit() / ... / literals);
        return _term<Terminator, decltype(l)>{};
    }

    //=== rules ===//
    /// Matches rule followed by the terminator.
    template <typename Rule>
    constexpr auto operator()(Rule rule) const
    {
        return _maybe_branch(rule, terminator());
    }

    /// Matches rule followed by the terminator, recovering on error.
    template <typename Rule>
    constexpr auto try_(Rule) const
    {
        return _tryt<Terminator, Rule, decltype(recovery_rule())>{};
    }

    /// Matches opt(rule) followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto opt(Rule) const
    {
        return _optt<Terminator, _tryt<Terminator, Rule, decltype(recovery_rule())>>{};
    }

    /// Matches `list(rule, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto list(Rule) const
    {
        return _lstt<Terminator, Rule, void, decltype(recovery_rule())>{};
    }
    template <typename Rule, typename Sep>
    constexpr auto list(Rule, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _lstt<Terminator, Rule, Sep, decltype(recovery_rule())>{};
    }

    /// Matches `opt_list(rule, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto opt_list(Rule) const
    {
        return _optt<Terminator, _lstt<Terminator, Rule, void, decltype(recovery_rule())>>{};
    }
    template <typename Rule, typename Sep>
    constexpr auto opt_list(Rule, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _optt<Terminator, _lstt<Terminator, Rule, Sep, decltype(recovery_rule())>>{};
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
        if constexpr (std::is_void_v<RecoveryLimit>)
            return recover(terminator());
        else
            return recover(terminator()).limit(RecoveryLimit{});
    }
};

/// Creates a terminator using the given branch.
template <typename Branch>
constexpr auto terminator(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "terminator");
    return _term<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TERMINATOR_HPP_INCLUDED


namespace lexyd
{
template <typename Open, typename Close, typename RecoveryLimit = void>
struct _brackets
{
    /// Adds the literal tokens to the recovery limit.
    template <typename... Literals>
    constexpr auto limit(Literals... literals) const
    {
        static_assert(sizeof...(Literals) > 0);

        auto l = (recovery_rule().get_limit() / ... / literals);
        return _brackets<Open, Close, decltype(l)>{};
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
        return _term<Close, RecoveryLimit>{};
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
    LEXY_REQUIRE_BRANCH_RULE(Open, "brackets()");
    LEXY_REQUIRE_BRANCH_RULE(Close, "brackets()");
    return _brackets<Open, Close>{};
}

constexpr auto round_bracketed  = brackets(lit_c<'('>, lit_c<')'>);
constexpr auto square_bracketed = brackets(lit_c<'['>, lit_c<']'>);
constexpr auto curly_bracketed  = brackets(lit_c<'{'>, lit_c<'}'>);
constexpr auto angle_bracketed  = brackets(lit_c<'<'>, lit_c<'>'>);

constexpr auto parenthesized = round_bracketed;
} // namespace lexyd

#endif // LEXY_DSL_BRACKETS_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BYTE_HPP_INCLUDED
#define LEXY_DSL_BYTE_HPP_INCLUDED

#include <cstdint>





//=== byte ===//
namespace lexyd
{
template <std::size_t N, typename Predicate>
struct _b : token_base<_b<N, Predicate>>
{
    static_assert(N > 0);

    static constexpr bool _match(lexy::byte_encoding::int_type cur)
    {
        if (cur == lexy::byte_encoding::eof())
            return false;

        if constexpr (!std::is_void_v<Predicate>)
        {
            constexpr auto predicate = Predicate{};
            return predicate(static_cast<lexy::byte_encoding::char_type>(cur));
        }
        else
        {
            return true;
        }
    }

    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<N>>
    struct tp;

    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Bump N times.
            auto result
                = ((_match(reader.peek()) ? (reader.bump(), true) : ((void)Idx, false)) && ...);
            end = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            constexpr auto name
                = std::is_void_v<Predicate> ? "byte" : lexy::_detail::type_name<Predicate>();
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(), name);
            context.on(_ev::error{}, err);
        }
    };

    //=== dsl ===//
    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _b<N, P>{};
    }

    template <unsigned char Low, unsigned char High>
    constexpr auto range() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "byte.range";
            }

            constexpr bool operator()(unsigned char byte) const
            {
                return Low <= byte && byte <= High;
            }
        };

        return if_<predicate>();
    }

    template <unsigned char... Bytes>
    constexpr auto set() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "byte.set";
            }

            constexpr bool operator()(unsigned char byte) const
            {
                return ((byte == Bytes) || ...);
            }
        };

        return if_<predicate>();
    }

    constexpr auto ascii() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "byte.ASCII";
            }

            constexpr bool operator()(unsigned char byte) const
            {
                return byte <= 0x7F;
            }
        };

        return if_<predicate>();
    }
};

/// Matches an arbitrary byte.
constexpr auto byte = _b<1, void>{};

/// Matches N arbitrary bytes.
template <std::size_t N>
constexpr auto bytes = _b<N, void>{};
} // namespace lexyd

namespace lexy
{
template <std::size_t N>
constexpr auto token_kind_of<lexy::dsl::_b<N, void>> = lexy::any_token_kind;
} // namespace lexy

//=== padding bytes ===//
namespace lexyd
{
template <std::size_t N, unsigned char Padding = 0>
struct _pb : branch_base
{
    template <typename Context, typename Reader, typename Iterator>
    static constexpr void _validate(Context& context, const Reader&, Iterator begin, Iterator end)
    {
        constexpr unsigned char str[] = {Padding, 0};
        for (auto iter = begin; iter != end; ++iter)
        {
            if (*iter != Padding)
            {
                auto err = lexy::error<Reader, lexy::expected_literal>(iter, str, 0, 1);
                context.on(_ev::error{}, err);
            }
        }
    }

    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<_b<N, void>, Reader> parser(reader);
            auto                                        result = parser.try_parse(reader);
            end                                                = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, lexy::any_token_kind, begin, end.position());
            reader.reset(end);

            _validate(context, reader, begin, end.position());
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
            auto begin = reader.position();
            if (!_b<N, void>::token_parse(context, reader))
                return false;
            auto end = reader.position();

            _validate(context, reader, begin, end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };
};

/// Matches N bytes set to the padding value.
/// It is a recoverable error if the byte doesn't have that value.
template <std::size_t N, unsigned char Padding = 0>
constexpr auto padding_bytes = _pb<N, Padding>{};
} // namespace lexyd

//=== bint ===//
namespace lexy::_detail
{
enum bint_endianness
{
    bint_little,
    bint_big,

#if LEXY_IS_LITTLE_ENDIAN
    bint_native = bint_little,
#else
    bint_native = bint_big,
#endif
};

template <std::size_t N>
auto _bint()
{
    if constexpr (N == 1)
        return std::uint_least8_t(0);
    else if constexpr (N == 2)
        return std::uint_least16_t(0);
    else if constexpr (N == 4)
        return std::uint_least32_t(0);
    else if constexpr (N == 8)
        return std::uint_least64_t(0);
    else
    {
        static_assert(N == 1, "invalid byte integer size");
        return 0;
    }
}

template <std::size_t N>
using bint = decltype(_bint<N>());
} // namespace lexy::_detail

namespace lexy
{
struct mismatched_byte_count
{
    static LEXY_CONSTEVAL auto name()
    {
        return "mismatched byte count";
    }
};
} // namespace lexy

namespace lexyd
{
template <std::size_t N, int Endianness, typename Rule = void>
struct _bint : branch_base
{
    using _rule = lexy::_detail::type_or<Rule, _b<N, void>>;

    template <typename NextParser, typename Indices = lexy::_detail::make_index_sequence<N>>
    struct _pc;

    template <typename NextParser, std::size_t... Idx>
    struct _pc<NextParser, lexy::_detail::index_sequence<Idx...>>
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           typename Reader::iterator begin,
                                           typename Reader::iterator end, Args&&... args)
        {
            if (lexy::_detail::range_size(begin, end) != N)
            {
                auto err = lexy::error<Reader, lexy::mismatched_byte_count>(begin, end);
                context.on(_ev::error{}, err);
                return false;
            }

            lexy::_detail::bint<N> result = 0;
            if constexpr (N == 1)
            {
                // For a single byte, endianness doesn't matter.
                result = static_cast<unsigned char>(*begin);
            }
            else if constexpr (Endianness == lexy::_detail::bint_big)
            {
                // In big endian, the first byte is shifted the most,
                // so read in order.
                ((result = static_cast<decltype(result)>(result << 8),
                  result = static_cast<decltype(result)>(result | *begin++), (void)Idx),
                 ...);
            }
            else
            {
                // In little endian, we need to reverse the order,
                // so copy into a buffer to do that.
                unsigned char buffer[N] = {((void)Idx, *begin++)...};
                ((result = static_cast<decltype(result)>(result << 8),
                  result = static_cast<decltype(result)>(result | buffer[N - Idx - 1])),
                 ...);
            }

            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)..., result);
        }
    };

    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<_rule, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, _rule{}, begin, end.position());
            reader.reset(end);

            return _pc<NextParser>::parse(context, reader, begin, end.position(),
                                          LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
            auto begin = reader.position();
            if (!_rule::token_parse(context, reader))
                return false;
            auto end = reader.position();
            return _pc<NextParser>::parse(context, reader, begin, end, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Matches a specific token rule instead of arbitrary bytes.
    template <typename Token>
    constexpr auto operator()(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        static_assert(std::is_void_v<Rule>);
        return _bint<N, Endianness, Token>{};
    }
};

/// Matches one byte and converts it into an 8-bit integer.
inline constexpr auto bint8 = _bint<1, lexy::_detail::bint_native>{};

/// Matches two bytes and converts it into an 16-bit integer.
inline constexpr auto bint16        = _bint<2, lexy::_detail::bint_native>{};
inline constexpr auto little_bint16 = _bint<2, lexy::_detail::bint_little>{};
inline constexpr auto big_bint16    = _bint<2, lexy::_detail::bint_big>{};

/// Matches four bytes and converts it into an 32-bit integer.
inline constexpr auto bint32        = _bint<4, lexy::_detail::bint_native>{};
inline constexpr auto little_bint32 = _bint<4, lexy::_detail::bint_little>{};
inline constexpr auto big_bint32    = _bint<4, lexy::_detail::bint_big>{};

/// Matches eight bytes and converts it into an 64-bit integer.
inline constexpr auto bint64        = _bint<8, lexy::_detail::bint_native>{};
inline constexpr auto little_bint64 = _bint<8, lexy::_detail::bint_little>{};
inline constexpr auto big_bint64    = _bint<8, lexy::_detail::bint_big>{};
} // namespace lexyd

#endif // LEXY_DSL_BYTE_HPP_INCLUDED
// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED





namespace lexyd
{
template <typename Token>
struct _cap : _copy_base<Token>
{
    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, Token{}, begin, end.position());
            reader.reset(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end.position()));
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            if (!Token::token_parse(context, reader))
                return false;
            auto end = reader.position();

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };
};

template <typename Rule>
struct _capr : _copy_base<Rule>
{
    template <typename NextParser, typename... PrevArgs>
    struct _pc : lexy::_detail::disable_whitespace_skipping
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           PrevArgs&&... prev_args, typename Reader::iterator begin,
                                           Args&&... args)
        {
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(prev_args)...,
                                       lexy::lexeme(reader, begin), LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using continuation = _pc<NextParser, Args...>;
            return rule.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                      reader.position());
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using parser = lexy::parser_for<Rule, _pc<NextParser, Args...>>;
            return parser::parse(context, reader, LEXY_FWD(args)..., reader.position());
        }
    };
};

template <typename Production>
struct _prd;

/// Captures whatever the token matches as a lexeme; does not include trailing whitespace.
template <typename Token>
constexpr auto capture(Token)
{
    static_assert(lexy::is_token_rule<Token>);
    return _cap<Token>{};
}

/// Captures whatever the token production matches as lexeme; does not include trailing whitespace.
template <typename Production>
constexpr auto capture(_prd<Production>)
{
    static_assert(lexy::is_token_production<Production>);
    return _capr<_prd<Production>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CASE_FOLDING_HPP_INCLUDED
#define LEXY_DSL_CASE_FOLDING_HPP_INCLUDED






//=== generic rule impl ===//
namespace lexyd
{
template <template <typename> typename CaseFolding>
struct _cfl_folding
{
    template <typename Reader>
    using reader = CaseFolding<Reader>;
};

template <typename Literal, template <typename> typename CaseFolding>
struct _cfl : token_base<_cfl<Literal, CaseFolding>>, _lit_base
{
    static constexpr auto lit_max_char_count = Literal::lit_max_char_count;

    static constexpr auto lit_char_classes = Literal::lit_char_classes;

    using lit_case_folding = _cfl_folding<CaseFolding>;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        return Literal::template lit_first_char<Encoding>();
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos,
                                                 std::size_t char_class)
    {
        return Literal::lit_insert(trie, pos, char_class);
    }

    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Literal, CaseFolding<Reader>> impl;
        typename Reader::marker                              end;

        constexpr explicit tp(const Reader& reader)
        : impl(CaseFolding<Reader>{reader}), end(reader.current())
        {}

        constexpr bool try_parse(Reader _reader)
        {
            CaseFolding<Reader> reader{_reader};
            auto                result = impl.try_parse(reader);
            end                        = impl.end;
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            impl.report_error(context, CaseFolding<Reader>{reader});
        }
    };
};
} // namespace lexyd

namespace lexy
{
template <typename Literal, template <typename> typename CaseFolding>
constexpr auto token_kind_of<lexy::dsl::_cfl<Literal, CaseFolding>> = lexy::literal_token_kind;
} // namespace lexy

//=== ASCII ===//
namespace lexy
{
template <typename Reader>
struct _acfr // ascii case folding reader
{
    Reader _impl;

    using encoding = typename Reader::encoding;
    using iterator = typename Reader::iterator;
    using marker   = typename Reader::marker;

    constexpr auto peek() const -> typename encoding::int_type
    {
        auto c = _impl.peek();
        if (encoding::to_int_type('A') <= c && c <= encoding::to_int_type('Z'))
            return typename encoding::int_type(c + encoding::to_int_type('a' - 'A'));
        else
            return c;
    }

    constexpr void bump()
    {
        _impl.bump();
    }

    constexpr iterator position() const
    {
        return _impl.position();
    }

    constexpr marker current() const noexcept
    {
        return _impl.current();
    }
    constexpr void reset(marker m) noexcept
    {
        _impl.reset(m);
    }
};
} // namespace lexy

namespace lexyd::ascii
{
struct _cf_dsl
{
    template <typename Encoding>
    static constexpr auto is_inplace = true;

    template <typename Reader>
    using case_folding = lexy::_acfr<Reader>;

    template <typename Literal>
    constexpr auto operator()(Literal) const
    {
        static_assert(lexy::is_literal_rule<Literal>);
        static_assert(std::is_void_v<typename Literal::lit_case_folding>, "cannot case fold twice");
        return _cfl<Literal, case_folding>{};
    }
};

/// Matches Literal with case insensitive ASCII characters.
inline constexpr auto case_folding = _cf_dsl{};
} // namespace lexyd::ascii

//=== Unicode ===//
namespace lexy
{
template <typename Reader>
struct _sucfr32 // simple unicode case folding reader, UTF-32
{
    Reader _impl;

    constexpr explicit _sucfr32(Reader impl) : _impl(impl) {}

    using encoding = typename Reader::encoding;
    using iterator = typename Reader::iterator;
    using marker   = typename Reader::marker;

    constexpr auto peek() const -> typename encoding::int_type
    {
        auto c = _impl.peek();
        return lexy::simple_case_fold(lexy::code_point(c)).value();
    }

    constexpr void bump()
    {
        _impl.bump();
    }

    constexpr iterator position() const
    {
        return _impl.position();
    }

    constexpr marker current() const noexcept
    {
        return _impl.current();
    }
    constexpr void reset(marker m) noexcept
    {
        _impl.reset(m);
    }
};

template <typename Reader>
struct _sucfrm // simple unicode case folding reader, UTF-8 and UTF-16
{
    using encoding = typename Reader::encoding;
    using iterator = typename Reader::iterator;
    using marker   = typename Reader::marker;

    Reader                       _impl;
    typename Reader::marker      _cur_pos;
    typename encoding::char_type _buffer[4];
    unsigned char                _buffer_size;
    unsigned char                _buffer_cur;

    constexpr explicit _sucfrm(Reader impl)
    : _impl(impl), _cur_pos(_impl.current()), _buffer{}, _buffer_size(0), _buffer_cur(0)
    {
        _fill();
    }

    constexpr void _fill()
    {
        _cur_pos = _impl.current();

        // We need to read the next code point at this point.
        auto result = lexy::_detail::parse_code_point(_impl);
        if (result.error == lexy::_detail::cp_error::success)
        {
            // Fill the buffer with the folded code point.
            auto folded  = lexy::simple_case_fold(lexy::code_point(result.cp));
            _buffer_size = static_cast<unsigned char>(
                lexy::_detail::encode_code_point<encoding>(folded.value(), _buffer, 4));
            _buffer_cur = 0;
            _impl.reset(result.end);
        }
        else
        {
            // Fill the buffer with the partial code point.
            _buffer_cur = _buffer_size = 0;
            while (_impl.position() != result.end.position())
            {
                _buffer[_buffer_size] = static_cast<typename encoding::char_type>(_impl.peek());
                ++_buffer_size;
                _impl.bump();
            }
        }
    }

    constexpr auto peek() const -> typename encoding::int_type
    {
        if (_buffer_cur == _buffer_size)
            return encoding::eof();

        auto cur = _buffer[_buffer_cur];
        return encoding::to_int_type(cur);
    }

    constexpr void bump()
    {
        ++_buffer_cur;
        if (_buffer_cur == _buffer_size)
            _fill();
    }

    constexpr iterator position() const
    {
        return current().position();
    }

    constexpr marker current() const noexcept
    {
        // We only report a marker at a code point boundary.
        // This has two consequences:
        // 1. If we don't match a rule, the error token does not include any common start code
        //    units. That's actually nice, and makes it unnecessary to handle that situation in the
        //    error reporting. The only relevant difference is in the error token.
        // 2. If the user wants to match partial code unit sequences, the behavior can become buggy.
        //    However, that's not really something we should worry about.
        return _cur_pos;
    }
    constexpr void reset(marker m) noexcept
    {
        _impl.reset(m);
    }
};

template <typename Reader>
using _sucfr_for
    = std::conditional_t<std::is_same_v<typename Reader::encoding, lexy::utf32_encoding>,
                         _sucfr32<Reader>, _sucfrm<Reader>>;

template <typename Reader>
struct _sucfr : _sucfr_for<Reader>
{
    using _sucfr_for<Reader>::_sucfr_for;
};
} // namespace lexy

namespace lexyd::unicode
{
struct _scf_dsl
{
    template <typename Encoding>
    static constexpr auto is_inplace = std::is_same_v<Encoding, lexy::utf32_encoding>;

    template <typename Reader>
    using case_folding = lexy::_sucfr<Reader>;

    template <typename Literal>
    constexpr auto operator()(Literal) const
    {
        static_assert(lexy::is_literal_rule<Literal>);
        static_assert(std::is_void_v<typename Literal::lit_case_folding>, "cannot case fold twice");
        return _cfl<Literal, case_folding>{};
    }
};

/// Matches Literal with case insensitive Unicode characters (simple case folding).
inline constexpr auto simple_case_folding = _scf_dsl{};
} // namespace lexyd::unicode

#endif // LEXY_DSL_CASE_FOLDING_HPP_INCLUDED




// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
struct _comb_control
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

// Final rule for one item in the combination.
template <std::size_t Idx>
struct _comb_it : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename Sink, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context&, Reader&, _comb_control<Sink>& ctrl,
                                           Args&&... args)
        {
            ctrl.idx = Idx;
            if constexpr (sizeof...(Args) > 0)
            {
                if (!ctrl.handled[Idx])
                    // Only call the sink if it is not a duplicate.
                    ctrl.sink(LEXY_FWD(args)...);
            }
            return true;
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
            return ((R{} >> _comb_it<Idx>{}) | ...);
        else
            return ((R{} >> _comb_it<Idx>{}) | ... | ElseRule{});
    }
    using _comb_choice = decltype(_comb_choice_(lexy::_detail::index_sequence_for<R...>{}));

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto N = sizeof...(R);

            auto                          sink       = context.value_callback().sink();
            bool                          handled[N] = {};
            _comb_control<decltype(sink)> control{sink, handled};

            // Parse all iterations of the choice.
            for (auto count = 0; count < int(N); ++count)
            {
                auto begin = reader.position();

                using parser
                    = lexy::parser_for<_comb_choice, lexy::pattern_parser<decltype(control)>>;
                if (!parser::parse(context, reader, control))
                    return false;
                else if (control.loop_break)
                    break; // Partial combination and we're done.

                if (handled[control.idx])
                {
                    using tag = lexy::_detail::type_or<DuplicateError, lexy::combination_duplicate>;
                    auto err  = lexy::error<Reader, tag>(begin, reader.position());
                    context.on(_ev::error{}, err);
                    // We can trivially recover, but need to do another iteration.
                    --count;
                }
                else
                {
                    handled[control.idx] = true;
                }
            }

            // Obtain the final result and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
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
    LEXY_REQUIRE_BRANCH_RULE(R..., "combination()");
    static_assert((!lexy::is_unconditional_branch_rule<R> && ...),
                  "combination() does not support unconditional branches");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
constexpr auto partial_combination(R...)
{
    LEXY_REQUIRE_BRANCH_RULE(R..., "partial_combination()");
    static_assert((!lexy::is_unconditional_branch_rule<R> && ...),
                  "partial_combination() does not support unconditional branches");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
template <typename Id>
using _ctx_counter = lexy::_detail::parse_context_var<Id, int>;

template <typename Id, int InitialValue>
struct _ctx_ccreate : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_counter<Id> var(InitialValue);
            var.link(context);
            auto result = NextParser::parse(context, reader, LEXY_FWD(args)...);
            var.unlink(context);
            return result;
        }
    };
};

template <typename Id, int Delta>
struct _ctx_cadd : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_counter<Id>::get(context.control_block) += Delta;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Rule, int Sign>
struct _ctx_cpush : _copy_base<Rule>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           typename Reader::iterator begin, Args&&... args)
        {
            auto end    = reader.position();
            auto length = lexy::_detail::range_size(begin, end);

            _ctx_counter<Id>::get(context.control_block) += int(length) * Sign;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            return rule.template finish<_pc<NextParser>>(context, reader, reader.position(),
                                                         LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using parser = lexy::parser_for<Rule, _pc<NextParser>>;
            return parser::parse(context, reader, reader.position(), LEXY_FWD(args)...);
        }
    };
};

template <typename Id, int Value>
struct _ctx_cis : branch_base
{
    template <typename Reader>
    struct bp
    {
        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader&)
        {
            return _ctx_counter<Id>::get(cb) == Value;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    using p = NextParser;
};

template <typename Id>
struct _ctx_cvalue : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                     _ctx_counter<Id>::get(context.control_block));
        }
    };
};

template <typename... Ids>
struct _ctx_ceq;
template <typename H, typename... T>
struct _ctx_ceq<H, T...> : branch_base
{
    template <typename Reader>
    struct bp
    {
        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader&)
        {
            auto value = _ctx_counter<H>::get(cb);
            return ((value == _ctx_counter<T>::get(cb)) && ...);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto value = _ctx_counter<H>::get(context.control_block);
            if (((value != _ctx_counter<T>::get(context.control_block)) || ...))
            {
                auto err = lexy::error<Reader, lexy::unequal_counts>(reader.position());
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
struct _ctx_counter_dsl
{
    template <int InitialValue = 0>
    constexpr auto create() const
    {
        return _ctx_ccreate<Id, InitialValue>{};
    }

    constexpr auto inc() const
    {
        return _ctx_cadd<Id, +1>{};
    }
    constexpr auto dec() const
    {
        return _ctx_cadd<Id, -1>{};
    }

    template <typename Rule>
    constexpr auto push(Rule) const
    {
        return _ctx_cpush<Id, Rule, +1>{};
    }
    template <typename Rule>
    constexpr auto pop(Rule) const
    {
        return _ctx_cpush<Id, Rule, -1>{};
    }

    template <int Value>
    constexpr auto is() const
    {
        return _ctx_cis<Id, Value>{};
    }
    constexpr auto is_zero() const
    {
        return is<0>();
    }

    constexpr auto value() const
    {
        return _ctx_cvalue<Id>{};
    }
};

/// Declares an integer counter that is added to the parsing context.
template <typename Id>
constexpr auto context_counter = _ctx_counter_dsl<Id>{};

/// Takes a branch only if all counters are equal.
template <typename... Ids>
constexpr auto equal_counts(_ctx_counter_dsl<Ids>...)
{
    static_assert(sizeof...(Ids) > 1);
    return _ctx_ceq<Ids...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_COUNTER_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED




namespace lexyd
{
template <typename Id>
using _ctx_flag = lexy::_detail::parse_context_var<Id, bool>;

template <typename Id, bool InitialValue>
struct _ctx_fcreate : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_flag<Id> var(InitialValue);
            var.link(context);
            auto result = NextParser::parse(context, reader, LEXY_FWD(args)...);
            var.unlink(context);
            return result;
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fset : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_flag<Id>::get(context.control_block) = Value;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id>
struct _ctx_ftoggle : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto& flag = _ctx_flag<Id>::get(context.control_block);
            flag       = !flag;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fis : branch_base
{
    template <typename Reader>
    struct bp
    {
        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader&)
        {
            return _ctx_flag<Id>::get(cb) == Value;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    using p = NextParser;
};

template <typename Id>
struct _ctx_fvalue : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                     _ctx_flag<Id>::get(context.control_block));
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id>
struct _ctx_flag_dsl
{
    template <bool InitialValue = false>
    constexpr auto create() const
    {
        return _ctx_fcreate<Id, InitialValue>{};
    }

    constexpr auto set() const
    {
        return _ctx_fset<Id, true>{};
    }
    constexpr auto reset() const
    {
        return _ctx_fset<Id, false>{};
    }

    constexpr auto toggle() const
    {
        return _ctx_ftoggle<Id>{};
    }

    constexpr auto is_set() const
    {
        return _ctx_fis<Id, true>{};
    }
    constexpr auto is_reset() const
    {
        return _ctx_fis<Id, false>{};
    }

    constexpr auto value() const
    {
        return _ctx_fvalue<Id>{};
    }
};

/// Declares a flag.
template <typename Id>
constexpr auto context_flag = _ctx_flag_dsl<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_FLAG_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED



// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
template <typename Id, typename CharT, CharT... C>
struct _kw;
template <typename Literal, template <typename> typename CaseFolding>
struct _cfl;

template <typename Leading, typename Trailing>
struct _idp : token_base<_idp<Leading, Trailing>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);

            // Need to match Leading character.
            if (!lexy::try_match_token(Leading{}, reader))
                return false;

            // Match zero or more trailing characters.
            while (true)
            {
                if constexpr (lexy::_detail::is_swar_reader<Reader>)
                {
                    // If we have a swar reader, consume as much as possible at once.
                    while (Trailing{}.template char_class_match_swar<typename Reader::encoding>(
                        reader.peek_swar()))
                        reader.bump_swar();
                }

                if (!lexy::try_match_token(Trailing{}, reader))
                    break;
            }

            end = reader.current();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            Leading::template char_class_report_error<Reader>(context, reader.position());
        }
    };
};

template <typename Set>
struct _idrp // reserve predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        return lexy::try_match_token(Set{}, reader)
               && reader.peek() == decltype(reader)::encoding::eof();
    }
};
template <typename Set>
struct _idpp // reserve prefix predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        return lexy::try_match_token(Set{}, reader);
    }
};
template <typename Set>
struct _idcp // reserve contains predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        while (true)
        {
            if (lexy::try_match_token(Set{}, reader))
                return true;
            else if (reader.peek() == decltype(reader)::encoding::eof())
                return false;
            else
                reader.bump();
        }

        // unreachable
    }
};
template <typename Set>
struct _idsp // reserve suffix predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        while (true)
        {
            if (lexy::try_match_token(Set{}, reader)
                && reader.peek() == decltype(reader)::encoding::eof())
                return true;
            else if (reader.peek() == decltype(reader)::encoding::eof())
                return false;
            else
                reader.bump();
        }

        // unreachable
    }
};

template <typename Leading, typename Trailing, typename... ReservedPredicate>
struct _id : branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the pattern; this does not consume whitespace, so the range is accurate.
            auto begin = reader.position();
            if (!pattern().token_parse(context, reader))
                return false;
            auto end = reader.position();

            // Check for a reserved identifier.
            [[maybe_unused]] auto input = lexy::partial_input(reader, begin, end);
            if ((ReservedPredicate::is_reserved(input) || ...))
            {
                // It is reserved, report an error but trivially recover.
                auto err = lexy::error<Reader, lexy::reserved_identifier>(begin, end);
                context.on(_ev::error{}, err);
            }

            // Skip whitespace and continue with the value.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr bool try_parse(const void*, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<decltype(pattern()), Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // We only succeed if it's not a reserved identifier.
            [[maybe_unused]] auto input
                = lexy::partial_input(reader, reader.position(), end.position());
            return !(ReservedPredicate::is_reserved(input) || ...);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end.position());
            reader.reset(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end.position()));
        }
    };

    template <typename R>
    constexpr auto _make_reserve(R r) const
    {
        static_assert(lexy::is_literal_rule<R> || lexy::is_literal_set_rule<R>);
        return r;
    }
    template <typename Id, typename CharT, CharT... C>
    constexpr auto _make_reserve(_kw<Id, CharT, C...>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // No need to remember that it was a keyword originally.
        return _lit<CharT, C...>{};
    }
    template <typename Id, typename CharT, CharT... C, template <typename> typename CaseFolding>
    constexpr auto _make_reserve(_cfl<_kw<Id, CharT, C...>, CaseFolding>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // No need to remember that it was a keyword originally.
        return _cfl<_lit<CharT, C...>, CaseFolding>{};
    }

    //=== dsl ===//
    /// Adds a set of reserved identifiers.
    template <typename... R>
    constexpr auto reserve(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idrp<decltype(set)>>{};
    }

    /// Reserves everything starting with the given rule.
    template <typename... R>
    constexpr auto reserve_prefix(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idpp<decltype(set)>>{};
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    constexpr auto reserve_containing(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idcp<decltype(set)>>{};
    }

    /// Reserves everything that ends with the given rule.
    template <typename... R>
    constexpr auto reserve_suffix(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idsp<decltype(set)>>{};
    }

    /// Matches every identifier, ignoring reserved ones.
    static constexpr auto pattern()
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

/// Creates an identifier that consists of one or more of the given characters.
template <typename CharClass>
constexpr auto identifier(CharClass)
{
    static_assert(lexy::is_char_class_rule<CharClass>);
    return _id<CharClass, CharClass>{};
}

/// Creates an identifier that consists of one leading token followed by zero or more trailing
/// tokens.
template <typename LeadingClass, typename TrailingClass>
constexpr auto identifier(LeadingClass, TrailingClass)
{
    static_assert(lexy::is_char_class_rule<LeadingClass> //
                  && lexy::is_char_class_rule<TrailingClass>);
    return _id<LeadingClass, TrailingClass>{};
}
} // namespace lexyd

namespace lexy
{
template <typename Leading, typename Trailing>
constexpr auto token_kind_of<lexy::dsl::_idp<Leading, Trailing>> = lexy::identifier_token_kind;
} // namespace lexy

//=== keyword ===//
namespace lexyd
{
template <typename Id, typename CharT, CharT... C>
struct _kw : token_base<_kw<Id, CharT, C...>>, _lit_base
{
    static constexpr auto lit_max_char_count = sizeof...(C);

    // We must not end on a trailing character.
    static constexpr auto lit_char_classes
        = lexy::_detail::char_class_list<decltype(Id{}.trailing_pattern())>{};

    using lit_case_folding = void;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        typename Encoding::char_type result = 0;
        (void)((result = lexy::_detail::transcode_char<decltype(result)>(C), true) || ...);
        return result;
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos,
                                                 std::size_t char_class)
    {
        auto end                  = ((pos = trie.insert(pos, C)), ...);
        trie.node_char_class[end] = char_class;
        return end;
    }

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match the literal.
            if (!lexy::_detail::match_literal<0, CharT, C...>(reader))
                return false;
            end = reader.current();

            // To qualify as a keyword, and not just the prefix of an identifier,
            // we must not have a trailing identifier character.
            return !lexy::try_match_token(Id{}.trailing_pattern(), reader);
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            using char_type    = typename Reader::encoding::char_type;
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<char_type>;

            // Match the entire identifier.
            auto begin = reader.position();
            lexy::try_match_token(Id{}.pattern(), reader);
            auto end = reader.position();

            auto err = lexy::error<Reader, lexy::expected_keyword>(begin, end, str, sizeof...(C));
            context.on(_ev::error{}, err);
        }
    };
};

template <typename Id>
struct _keyword;
template <typename L, typename T, typename... R>
struct _keyword<_id<L, T, R...>>
{
    template <typename CharT, CharT... C>
    using get = _kw<_id<L, T>, CharT, C...>;
};

#if LEXY_HAS_NTTP
template <lexy::_detail::string_literal Str, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return lexy::_detail::to_type_string<_keyword<_id<L, T>>::template get, Str>{};
}
#else
template <auto C, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return _kw<_id<L, T>, LEXY_DECAY_DECLTYPE(C), C>{};
}
#endif

#define LEXY_KEYWORD(Str, Id)                                                                      \
    LEXY_NTTP_STRING(::lexyd::_keyword<LEXY_DECAY_DECLTYPE(Id)>::template get, Str) {}
} // namespace lexyd

namespace lexy
{
template <typename Id, typename CharT, CharT... C>
constexpr auto token_kind_of<lexy::dsl::_kw<Id, CharT, C...>> = lexy::literal_token_kind;
} // namespace lexy

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
template <typename Id, typename Reader>
using _ctx_id = lexy::_detail::parse_context_var<Id, lexy::lexeme<Reader>>;

template <typename Id>
struct _ctx_icreate : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_id<Id, Reader> var({});
            var.link(context);
            auto result = NextParser::parse(context, reader, LEXY_FWD(args)...);
            var.unlink(context);
            return result;
        }
    };
};

template <typename Id, typename Identifier>
struct _ctx_icap : branch_base
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // The last argument will be a lexeme.
            _ctx_id<Id, Reader>::get(context.control_block) = (args, ...);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
    template <typename Reader>
    using bp = lexy::continuation_branch_parser<Identifier, Reader, _pc>;
    template <typename NextParser>
    using p = lexy::parser_for<Identifier, _pc<NextParser>>;
};

template <typename Id, typename Identifier, typename Tag>
struct _ctx_irem : branch_base
{
    // We only need the pattern:
    // We don't want a value and don't need to check for reserved identifiers,
    // as it needs to match a previously parsed identifier, which wasn't reserved.
    using _pattern = decltype(Identifier{}.pattern());

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<_pattern, Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // The two lexemes need to be equal.
            auto lexeme = lexy::lexeme<Reader>(reader.position(), end.position());
            return lexy::_detail::equal_lexemes(_ctx_id<Id, Reader>::get(cb), lexeme);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish parsing the token.
            context.on(_ev::token{}, lexy::identifier_token_kind, reader.position(),
                       end.position());
            reader.reset(end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename... PrevArgs>
        struct _cont
        {
            template <typename Context, typename Reader>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, PrevArgs&&... args,
                                               lexy::lexeme<Reader> lexeme)
            {
                if (!lexy::_detail::equal_lexemes(_ctx_id<Id, Reader>::get(context.control_block),
                                                  lexeme))
                {
                    // The lexemes weren't equal.
                    using tag = lexy::_detail::type_or<Tag, lexy::different_identifier>;
                    auto err  = lexy::error<Reader, tag>(lexeme.begin(), lexeme.end());
                    context.on(_ev::error{}, err);

                    // But we can trivially recover.
                }

                // Continue parsing with the symbol value.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Capture the pattern and continue with special continuation.
            return lexy::parser_for<_cap<_pattern>, _cont<Args...>>::parse(context, reader,
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
struct _ctx_id_dsl
{
    constexpr auto create() const
    {
        return _ctx_icreate<Id>{};
    }

    constexpr auto capture() const
    {
        return _ctx_icap<Id, Identifier>{};
    }

    constexpr auto rematch() const
    {
        return _ctx_irem<Id, Identifier, void>{};
    }
};

/// Declares a context variable that stores one instance of the given identifier.
template <typename Id, typename Leading, typename Trailing, typename... Reserved>
constexpr auto context_identifier(_id<Leading, Trailing, Reserved...>)
{
    return _ctx_id_dsl<Id, _id<Leading, Trailing, Reserved...>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED






// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SYMBOL_HPP_INCLUDED
#define LEXY_DSL_SYMBOL_HPP_INCLUDED







namespace lexy
{
#define LEXY_SYMBOL(Str) LEXY_NTTP_STRING(::lexy::_detail::type_string, Str)

template <typename T, template <typename> typename CaseFolding, typename... Strings>
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

    template <typename CaseFoldingDSL>
    LEXY_CONSTEVAL auto case_folding(CaseFoldingDSL) const
    {
        return _symbol_table<T, CaseFoldingDSL::template case_folding,
                             Strings...>(_detail::make_index_sequence<size()>{}, *this);
    }

    template <typename SymbolString, typename... Args>
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        using next_table = _symbol_table<T, CaseFolding, Strings..., SymbolString>;
        if constexpr (empty())
            return next_table(_detail::make_index_sequence<0>{}, nullptr, LEXY_FWD(args)...);
        else
            return next_table(_detail::make_index_sequence<size()>{}, _data, LEXY_FWD(args)...);
    }

#if LEXY_HAS_NTTP
    template <_detail::string_literal SymbolString, typename... Args>
    LEXY_CONSTEVAL auto map(Args&&... args) const
    {
        return map<_detail::to_type_string<_detail::type_string, SymbolString>>(LEXY_FWD(args)...);
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
        return map<_detail::type_string<LEXY_DECAY_DECLTYPE(C), C>>(LEXY_FWD(args)...);
    }
#endif

    template <typename CharT, CharT... C, typename... Args>
    LEXY_CONSTEVAL auto map(lexyd::_lit<CharT, C...>, Args&&... args) const
    {
        return map<_detail::type_string<CharT, C...>>(LEXY_FWD(args)...);
    }

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
                constexpr const char_type* strings[] = {Strings::template c_str<char_type>...};
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
        using encoding = typename Reader::encoding;

        auto result = _detail::lit_trie_matcher<_trie<encoding>, 0>::try_match(reader);
        if (result == _trie<encoding>.node_no_match)
            return key_index();
        else
            return key_index(result);
    }

    template <typename Input>
    constexpr key_index parse(const Input& input) const
    {
        auto reader = input.reader();
        auto result = try_parse(reader);
        if (reader.peek() == decltype(reader)::encoding::eof())
            return result;
        else
            return key_index();
    }

    constexpr const T& operator[](key_index idx) const noexcept
    {
        LEXY_PRECONDITION(idx);
        return _data[idx._value];
    }

private:
    static constexpr auto _max_char_count = (0 + ... + Strings::size);

    template <typename Encoding>
    static LEXY_CONSTEVAL auto _build_trie()
    {
        lexy::_detail::lit_trie<Encoding, CaseFolding, _max_char_count> result;

        auto idx = 0u;
        ((result.node_value[result.insert(0, Strings{})] = idx++), ...);

        return result;
    }
    template <typename Encoding>
    static constexpr lexy::_detail::lit_trie<Encoding, CaseFolding, _max_char_count> _trie
        = _build_trie<Encoding>();

    template <std::size_t... Idx, typename... Args>
    constexpr explicit _symbol_table(lexy::_detail::index_sequence<Idx...>, const T* data,
                                     Args&&... args)
    // New data is appended at the end.
    : _data{data[Idx]..., T(LEXY_FWD(args)...)}
    {}
    template <std::size_t... Idx, template <typename> typename OtherCaseFolding>
    constexpr explicit _symbol_table(lexy::_detail::index_sequence<Idx...>,
                                     const _symbol_table<T, OtherCaseFolding, Strings...>& table)
    : _data{table._data[Idx]...}
    {}

    std::conditional_t<empty(), char, T> _data[empty() ? 1 : size()];

    template <typename, template <typename> typename, typename...>
    friend class _symbol_table;
};

template <typename T>
constexpr auto symbol_table = _symbol_table<T, _detail::lit_no_case_fold>{};
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
struct _sym : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        typename Reader::marker end;
        typename LEXY_DECAY_DECLTYPE(Table)::key_index symbol;

        constexpr auto value() const
        {
            return Table[symbol];
        }

        template <typename ControlBlock>
        constexpr bool try_parse(ControlBlock&, const Reader& reader)
        {
            // Try and parse the token.
            lexy::token_parser_for<Token, Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // Check whether this is a symbol.
            auto content = lexy::partial_input(reader, end.position());
            symbol       = Table.parse(content);

            // Only succeed if it is a symbol.
            return static_cast<bool>(symbol);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // We need to consume and report the token.
            context.on(_ev::token{}, Token{}, reader.position(), end.position());
            reader.reset(end);

            // And continue parsing with the symbol value after whitespace skipping.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[symbol]);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename... PrevArgs>
        struct _cont
        {
            template <typename Context, typename Reader>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, PrevArgs&&... args,
                                               lexy::lexeme<Reader> lexeme)
            {
                // Check whether the captured lexeme is a symbol.
                auto content = lexy::partial_input(reader, lexeme.begin(), lexeme.end());
                auto symbol  = Table.parse(content);
                if (!symbol)
                {
                    // Unknown symbol.
                    using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                    auto err  = lexy::error<Reader, tag>(lexeme.begin(), lexeme.end());
                    context.on(_ev::error{}, err);
                    return false;
                }

                // Continue parsing with the symbol value.
                return NextParser::parse(context, reader, LEXY_FWD(args)..., Table[symbol]);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            // Capture the token and continue with special continuation.
            return lexy::parser_for<_cap<Token>, _cont<Args...>>::parse(context, reader,
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
struct _sym<Table, _idp<L, T>, Tag> : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        typename LEXY_DECAY_DECLTYPE(Table)::key_index symbol;
        typename Reader::marker end;

        constexpr auto value() const
        {
            return Table[symbol];
        }

        constexpr bool try_parse(const void*, Reader reader)
        {
            // Try to parse a symbol.
            symbol = Table.try_parse(reader);
            if (!symbol)
                return false;
            end = reader.current();

            // We had a symbol, but it must not be the prefix of a valid identifier.
            return !lexy::try_match_token(T{}, reader);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // We need to consume and report the identifier pattern.
            context.on(_ev::token{}, _idp<L, T>{}, reader.position(), end.position());
            reader.reset(end);

            // And continue parsing with the symbol value after whitespace skipping.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[symbol]);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            auto begin = reader.position();

            // Try to parse a symbol that is not the prefix of an identifier.
            auto symbol_reader = reader;
            auto symbol        = Table.try_parse(symbol_reader);
            if (!symbol || lexy::try_match_token(T{}, symbol_reader))
            {
                // Unknown symbol or not an identifier.
                // Parse the identifier pattern normally, and see if that fails.
                using id_parser = lexy::parser_for<_idp<L, T>, lexy::pattern_parser<>>;
                if (!id_parser::parse(context, reader))
                    // It did fail, so it reported an error and we're done here.
                    return false;

                // We're having a valid identifier but unknown symbol.
                using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
                auto err  = lexy::error<Reader, tag>(begin, reader.position());
                context.on(_ev::error{}, err);

                return false;
            }
            else
            {
                // We need to consume and report the identifier pattern.
                auto end = symbol_reader.current();
                context.on(_ev::token{}, _idp<L, T>{}, begin, end.position());
                reader.reset(end);

                // And continue parsing with the symbol value after whitespace skipping.
                using continuation = lexy::whitespace_parser<Context, NextParser>;
                return continuation::parse(context, reader, LEXY_FWD(args)..., Table[symbol]);
            }
        }
    };

    //=== dsl ===//
    template <typename ErrorTag>
    static constexpr _sym<Table, _idp<L, T>, ErrorTag> error = {};
};

template <const auto& Table, typename Tag>
struct _sym<Table, void, Tag> : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        typename LEXY_DECAY_DECLTYPE(Table)::key_index symbol;
        typename Reader::marker end;

        constexpr auto value() const
        {
            return Table[symbol];
        }

        constexpr bool try_parse(const void*, Reader reader)
        {
            // Try to parse a symbol.
            symbol = Table.try_parse(reader);
            end    = reader.current();

            // Only succeed if it is a symbol.
            return static_cast<bool>(symbol);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // We need to consume and report the token.
            context.on(_ev::token{}, lexy::identifier_token_kind, reader.position(),
                       end.position());
            reader.reset(end);

            // And continue parsing with the symbol value after whitespace skipping.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)..., Table[symbol]);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            bp<Reader> impl{};
            if (impl.try_parse(context.control_block, reader))
                return impl.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            impl.cancel(context);

            // Unknown symbol.
            using tag = lexy::_detail::type_or<Tag, lexy::unknown_symbol>;
            auto err  = lexy::error<Reader, tag>(reader.position());
            context.on(_ev::error{}, err);

            return false;
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
template <typename CharClass, typename Reader>
struct _del_chars
{
    typename Reader::iterator begin;

    constexpr _del_chars(const Reader& reader) : begin(reader.position()) {}

    template <typename Context>
    constexpr void _recover(Context& context, typename Reader::iterator recover_begin,
                            typename Reader::iterator recover_end)
    {
        CharClass::template char_class_report_error<Reader>(context, recover_begin);

        // We recovery by discarding the ASCII character.
        // (We've checked for EOF before, so that's not the error.)
        context.on(_ev::recovery_start{}, recover_begin);
        context.on(_ev::token{}, lexy::error_token_kind, recover_begin, recover_end);
        context.on(_ev::recovery_finish{}, recover_end);

        // Restart the next character here.
        begin = recover_end;
    }

    template <typename Close, typename... Escs>
    constexpr void parse_swar(Reader& reader, Close, Escs...)
    {
        using encoding = typename Reader::encoding;

        // If we have a SWAR reader and the Close and Escape chars are literal rules,
        // we can munch as much content as possible in a fast loop.
        // We also need to efficiently check for the CharClass for it to make sense.
        if constexpr (lexy::_detail::is_swar_reader<Reader> //
                      && (lexy::is_literal_rule<Close> && ... && Escs::esc_is_literal)
                      && !std::is_same_v<
                          decltype(CharClass::template char_class_match_swar<encoding>({})),
                          std::false_type>)
        {
            using char_type = typename encoding::char_type;
            using lexy::_detail::swar_has_char;

            while (true)
            {
                auto cur = reader.peek_swar();

                // If we have an EOF or the initial character of the closing delimiter, we exit as
                // we have no more content.
                if (swar_has_char<char_type, encoding::eof()>(cur)
                    || swar_has_char<char_type, Close::template lit_first_char<encoding>()>(cur))
                    break;

                // The same is true if we have the escape character.
                if constexpr (sizeof...(Escs) > 0)
                {
                    if ((swar_has_char<char_type, Escs::template esc_first_char<encoding>()>(cur)
                         || ...))
                        break;
                }

                // We definitely don't have the end of the delimited content in the current SWAR,
                // check if they all follow the char class.
                if (!CharClass::template char_class_match_swar<encoding>(cur))
                    // They don't or we need to look closer, exit the loop.
                    break;

                reader.bump_swar();
            }
        }
    }

    // Precondition: the next code unit definitely belongs to the content, not the delimiter.
    template <typename Context, typename Sink>
    constexpr void parse_one(Context& context, Reader& reader, Sink& sink)
    {
        using encoding = typename Reader::encoding;

        // First try to match the ASCII characters.
        using matcher = lexy::_detail::ascii_set_matcher<_cas<CharClass>>;
        if (matcher::template match<encoding>(reader.peek()))
        {
            reader.bump();
        }
        else if constexpr (!std::is_same_v<decltype(CharClass::char_class_match_cp(char32_t())),
                                           std::false_type>)
        {
            if constexpr (lexy::is_unicode_encoding<encoding>)
            {
                static_assert(CharClass::char_class_unicode(),
                              "cannot use this character class with Unicode encoding");

                auto result = lexy::_detail::parse_code_point(reader);
                if (result.error == lexy::_detail::cp_error::success
                    && CharClass::char_class_match_cp(result.cp))
                {
                    reader.reset(result.end);
                }
                else
                {
                    finish(context, sink, reader.position());

                    auto recover_begin = reader.position();
                    if (recover_begin == result.end.position())
                        reader.bump();
                    else
                        reader.reset(result.end);
                    _recover(context, recover_begin, reader.position());
                }
            }
            else
            {
                static_assert(!CharClass::char_class_unicode(),
                              "cannot use this character class with non-Unicode char encoding");
                LEXY_ASSERT(reader.peek() != encoding::eof(),
                            "EOF should be checked before calling this");

                auto recover_begin = reader.position();
                auto cp            = static_cast<char32_t>(reader.peek());
                reader.bump();

                if (!CharClass::char_class_match_cp(cp))
                {
                    finish(context, sink, recover_begin);
                    _recover(context, recover_begin, reader.position());
                }
            }
        }
        // It doesn't match Unicode characters.
        else
        {
            // We can just discard the invalid ASCII character.
            LEXY_ASSERT(reader.peek() != encoding::eof(),
                        "EOF should be checked before calling this");
            auto recover_begin = reader.position();
            reader.bump();
            auto recover_end = reader.position();

            finish(context, sink, recover_begin);
            _recover(context, recover_begin, recover_end);
        }
    }

    template <typename Context, typename Sink>
    constexpr void finish(Context& context, Sink& sink, typename Reader::iterator end)
    {
        if (begin == end)
            return;

        context.on(_ev::token{}, typename CharClass::token_type{}, begin, end);
        sink(lexy::lexeme<Reader>(begin, end));
    }
};

template <typename Token, typename Error = lexy::missing_delimiter>
struct _del_limit
{
    using error = Error;

    template <typename Reader>
    static constexpr bool peek(Reader reader)
    {
        return lexy::try_match_token(Token{}, reader) || reader.peek() == Reader::encoding::eof();
    }
};
template <typename Error>
struct _del_limit<void, Error>
{
    using error = Error;

    template <typename Reader>
    static constexpr bool peek(Reader reader)
    {
        return reader.peek() == Reader::encoding::eof();
    }
};

template <typename Close, typename Char, typename Limit, typename... Escapes>
struct _del : rule_base
{
    using _limit = std::conditional_t<std::is_void_v<Limit> || lexy::is_token_rule<Limit>,
                                      _del_limit<Limit>, Limit>;

    template <typename CloseParser, typename Context, typename Reader, typename Sink>
    LEXY_PARSER_FUNC static bool _loop(CloseParser& close, Context& context, Reader& reader,
                                       Sink& sink)
    {
        auto                     del_begin = reader.position();
        _del_chars<Char, Reader> cur_chars(reader);
        while (true)
        {
            // Parse as many content chars as possible.
            // If it returns, we need to look closer at the next char.
            cur_chars.parse_swar(reader, Close{}, Escapes{}...);

            // Check for closing delimiter.
            if (close.try_parse(context.control_block, reader))
                break;
            close.cancel(context);

            // Check for missing delimiter.
            if (_limit::peek(reader))
            {
                // We're done, so finish the current characters.
                auto end = reader.position();
                cur_chars.finish(context, sink, end);

                auto err = lexy::error<Reader, typename _limit::error>(del_begin, end);
                context.on(_ev::error{}, err);
                return false;
            }

            // Check for escape sequences.
            if ((Escapes::esc_try_parse(context, reader, sink, cur_chars) || ...))
                // We had an escape sequence, so do nothing in this iteration.
                continue;

            // It is actually a content char, consume it.
            cur_chars.parse_one(context, reader, sink);
        }

        // Finish the currently active character sequence.
        cur_chars.finish(context, sink, reader.position());
        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            auto sink = context.value_callback().sink();

            // Parse characters until we have the closing delimiter.
            lexy::branch_parser_for<Close, Reader> close{};
            if (!_loop(close, context, reader, sink))
                return false;

            // We're done, finish the sink and then the closing delimiter.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return close.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return close.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                         LEXY_MOV(sink).finish());
            }
        }
    };
};

struct _escape_base
{};

template <typename Open, typename Close, typename Limit = void>
struct _delim_dsl
{
    /// Add char classes that will limit the delimited to detect a missing terminator.
    template <typename LimitCharClass>
    constexpr auto limit(LimitCharClass) const
    {
        static_assert(std::is_void_v<Limit> && lexy::is_char_class_rule<LimitCharClass>);

        return _delim_dsl<Open, Close, LimitCharClass>{};
    }
    /// Add char classes that will limit the delimited and specify the error.
    template <typename Error, typename LimitCharClass>
    constexpr auto limit(LimitCharClass) const
    {
        static_assert(std::is_void_v<Limit> && lexy::is_char_class_rule<LimitCharClass>);
        return _delim_dsl<Open, Close, _del_limit<LimitCharClass, Error>>{};
    }

    //=== rules ===//
    /// Sets the content.
    template <typename Char, typename... Escapes>
    constexpr auto operator()(Char, Escapes...) const
    {
        static_assert(lexy::is_char_class_rule<Char>);
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
    LEXY_REQUIRE_BRANCH_RULE(Open, "delimited()");
    LEXY_REQUIRE_BRANCH_RULE(Close, "delimited()");
    return _delim_dsl<Open, Close>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
constexpr auto delimited(Delim)
{
    LEXY_REQUIRE_BRANCH_RULE(Delim, "delimited()");
    return _delim_dsl<Delim, Delim>{};
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
struct _escape : _escape_base
{
    static constexpr bool esc_is_literal = lexy::is_literal_rule<Escape>;
    template <typename Encoding>
    static constexpr auto esc_first_char() -> typename Encoding::char_type
    {
        return Escape::template lit_first_char<Encoding>();
    }

    template <typename Context, typename Reader, typename Sink, typename Char>
    static constexpr bool esc_try_parse(Context& context, Reader& reader, Sink& sink,
                                        _del_chars<Char, Reader>& cur_chars)
    {
        auto begin = reader.position();

        // Check whether we're having the initial escape character.
        lexy::branch_parser_for<Escape, Reader> token{};
        if (!token.try_parse(context.control_block, reader))
            // No need to call `.cancel()`; it's a token.
            return false;

        // We do, so finish current character sequence and consume the escape token.
        cur_chars.finish(context, sink, begin);
        // It's a token, so this can't fail.
        token.template finish<lexy::pattern_parser<>>(context, reader);

        // Try to parse the correct branch.
        auto try_parse_branch = [&](auto branch) {
            lexy::branch_parser_for<decltype(branch), Reader> parser{};
            if (!parser.try_parse(context.control_block, reader))
            {
                parser.cancel(context);
                return false;
            }

            // This might fail, but we don't care:
            // it will definitely consume the escape token, and everything that is a valid prefix.
            // The remaining stuff is then just treated as part of the delimited.
            parser.template finish<lexy::sink_parser>(context, reader, sink);
            return true;
        };
        auto found = (try_parse_branch(Branches{}) || ...);

        if constexpr ((lexy::is_unconditional_branch_rule<Branches> || ...))
        {
            LEXY_ASSERT(found, "there is an unconditional branch");
        }
        else if (!found)
        {
            // We haven't found any branch of the escape sequence.
            auto err = lexy::error<Reader, lexy::invalid_escape_sequence>(begin, reader.position());
            context.on(_ev::error{}, err);
        }

        // Restart the current character sequence after the escape sequence.
        cur_chars.begin = reader.position();
        return true;
    }

    /// Adds a generic escape rule.
    template <typename Branch>
    constexpr auto rule(Branch) const
    {
        LEXY_REQUIRE_BRANCH_RULE(Branch, "escape()");
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the branch.
    template <typename Branch>
    constexpr auto capture(Branch branch) const
    {
        LEXY_REQUIRE_BRANCH_RULE(Branch, "escape()");
        return this->rule(lexy::dsl::capture(branch));
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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED







//=== bases ===//
// SWAR matching code adapted from:
// https://lemire.me/blog/2018/09/30/quickly-identifying-a-sequence-of-digits-in-a-string-of-characters/
namespace lexyd
{
template <int Radix>
struct _d;

template <>
struct _d<2> : char_class_base<_d<2>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.binary";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '1');
        return result;
    }

    static constexpr unsigned digit_radix = 2;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        constexpr auto mask     = lexy::_detail::swar_fill_compl(CharT(0xF));
        constexpr auto expected = lexy::_detail::swar_fill(CharT(0x30));
        constexpr auto offset   = lexy::_detail::swar_fill(CharT(0x0E));

        return (c & mask) == expected && ((c + offset) & mask) == expected;
    }
};
using binary = _d<2>;

template <>
struct _d<8> : char_class_base<_d<8>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.octal";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '7');
        return result;
    }

    static constexpr unsigned digit_radix = 8;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        constexpr auto mask     = lexy::_detail::swar_fill_compl(CharT(0xF));
        constexpr auto expected = lexy::_detail::swar_fill(CharT(0x30));
        constexpr auto offset   = lexy::_detail::swar_fill(CharT(0x08));

        return (c & mask) == expected && ((c + offset) & mask) == expected;
    }
};
using octal = _d<8>;

template <>
struct _d<10> : char_class_base<_d<10>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.decimal";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        return result;
    }

    static constexpr unsigned digit_radix = 10;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        constexpr auto mask     = lexy::_detail::swar_fill_compl(CharT(0xF));
        constexpr auto expected = lexy::_detail::swar_fill(CharT(0x30));
        constexpr auto offset   = lexy::_detail::swar_fill(CharT(0x06));

        return (c & mask) == expected && ((c + offset) & mask) == expected;
    }
};
using decimal = _d<10>;

struct hex_lower : char_class_base<hex_lower>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.hex-lower";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        result.insert('a', 'f');
        return result;
    }

    static constexpr unsigned digit_radix = 16;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        if (c >= 'a')
            return static_cast<unsigned>(c) - 'a' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        // False negative for hex digits, but that's okay.
        return _d<10>::swar_matches<CharT>(c);
    }
};

struct hex_upper : char_class_base<hex_upper>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.hex-upper";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        result.insert('A', 'F');
        return result;
    }

    static constexpr unsigned digit_radix = 16;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        if (c >= 'A')
            return static_cast<unsigned>(c) - 'A' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        // False negative for hex digits, but that's okay.
        return _d<10>::swar_matches<CharT>(c);
    }
};

template <>
struct _d<16> : char_class_base<_d<16>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.hex";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        result.insert('a', 'f');
        result.insert('A', 'F');
        return result;
    }

    static constexpr unsigned digit_radix = 16;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
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

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        // False negative for hex digits, but that's okay.
        return _d<10>::swar_matches<CharT>(c);
    }
};
using hex = _d<16>;
} // namespace lexyd

//=== digit ===//
namespace lexyd
{
struct _zero : char_class_base<_zero>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.zero";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0');
        return result;
    }
};

/// Matches the zero digit.
constexpr auto zero = _zero{};

/// Matches a single digit.
template <typename Base = decimal, int = Base::digit_radix>
constexpr auto digit = Base{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_zero> = lexy::digits_token_kind;

template <int Radix>
constexpr auto token_kind_of<lexy::dsl::_d<Radix>> = lexy::digits_token_kind;
template <>
inline constexpr auto token_kind_of<lexy::dsl::hex_lower> = lexy::digits_token_kind;
template <>
inline constexpr auto token_kind_of<lexy::dsl::hex_upper> = lexy::digits_token_kind;
} // namespace lexy

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
template <typename Base, typename Reader>
constexpr bool _match_digits(Reader& reader)
{
    // Need at least one digit.
    // Checking for a single digit is also cheaper than doing a SWAR comparison,
    // so we do that manually in either case.
    if (!lexy::try_match_token(digit<Base>, reader))
        return false;

    // Now we consume as many digits as possible.
    // First using SWAR...
    if constexpr (lexy::_detail::is_swar_reader<Reader>)
    {
        using char_type = typename Reader::encoding::char_type;
        while (Base::template swar_matches<char_type>(reader.peek_swar()))
            reader.bump_swar();
    }

    // ... then manually to get any trailing digits.
    while (lexy::try_match_token(digit<Base>, reader))
    {
    }

    return true;
}
template <typename Base, typename Sep, typename Reader>
constexpr bool _match_digits_sep(Reader& reader)
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
        else
        {
            // Attempt to consume as many digits as possible.
            if constexpr (lexy::_detail::is_swar_reader<Reader>)
            {
                using char_type = typename Reader::encoding::char_type;
                while (Base::template swar_matches<char_type>(reader.peek_swar()))
                    reader.bump_swar();
            }

            if (!lexy::try_match_token(digit<Base>, reader))
                // If we're not having a digit, we're done.
                break;
        }
    }

    return true;
}

template <typename Base, typename Sep>
struct _digits_st : token_base<_digits_st<Base, Sep>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;
        bool                    forbidden_leading_zero;

        constexpr explicit tp(const Reader& reader)
        : end(reader.current()), forbidden_leading_zero(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            using char_type = typename Reader::encoding::char_type;
            auto begin      = reader.current();
            auto result     = _match_digits_sep<Base, Sep>(reader);
            end             = reader.current();

            if (result && lexy::_detail::next(begin.position()) != end.position()
                && *begin.position() == lexy::_detail::transcode_char<char_type>('0'))
            {
                reader.reset(begin);
                reader.bump();
                end = reader.current();

                forbidden_leading_zero = true;
                return false;
            }

            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (forbidden_leading_zero)
            {
                auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(reader.position(),
                                                                             end.position());
                context.on(_ev::error{}, err);
            }
            else
            {
                auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                          Base::char_class_name());
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
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = _match_digits_sep<Base, Sep>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                      Base::char_class_name());
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
        typename Reader::marker end;
        bool                    forbidden_leading_zero;

        constexpr explicit tp(const Reader& reader)
        : end(reader.current()), forbidden_leading_zero(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            using char_type = typename Reader::encoding::char_type;
            auto begin      = reader.current();
            auto result     = _match_digits<Base>(reader);
            end             = reader.current();

            if (result && lexy::_detail::next(begin.position()) != end.position()
                && *begin.position() == lexy::_detail::transcode_char<char_type>('0'))
            {
                reader.reset(begin);
                reader.bump();
                end = reader.current();

                forbidden_leading_zero = true;
                return false;
            }

            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (forbidden_leading_zero)
            {
                auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(reader.position(),
                                                                             end.position());
                context.on(_ev::error{}, err);
            }
            else
            {
                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                          Base::char_class_name());
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
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = _match_digits<Base>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                      Base::char_class_name());
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

namespace lexy
{
template <typename Base>
constexpr auto token_kind_of<lexy::dsl::_digits<Base>> = lexy::digits_token_kind;
template <typename Base>
constexpr auto token_kind_of<lexy::dsl::_digits_t<Base>> = lexy::digits_token_kind;
template <typename Base, typename Sep>
constexpr auto token_kind_of<lexy::dsl::_digits_s<Base, Sep>> = lexy::digits_token_kind;
template <typename Base, typename Sep>
constexpr auto token_kind_of<lexy::dsl::_digits_st<Base, Sep>> = lexy::digits_token_kind;
} // namespace lexy

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
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the Base one time.
            if (!lexy::try_match_token(digit<Base>, reader))
            {
                end = reader.current();
                return false;
            }

            // Match each other digit after a separator.
            auto success = (((void)Idx, lexy::try_match_token(Sep{}, reader),
                             lexy::try_match_token(digit<Base>, reader))
                            && ...);
            end          = reader.current();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                      Base::char_class_name());
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
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the Base N times.
            auto success = (((void)Idx, lexy::try_match_token(digit<Base>, reader)) && ...);
            end          = reader.current();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                      Base::char_class_name());
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

namespace lexy
{
template <std::size_t N, typename Base>
constexpr auto token_kind_of<lexy::dsl::_ndigits<N, Base>> = lexy::digits_token_kind;
template <std::size_t N, typename Base, typename Sep>
constexpr auto token_kind_of<lexy::dsl::_ndigits_s<N, Base, Sep>> = lexy::digits_token_kind;
} // namespace lexy

#endif // LEXY_DSL_DIGIT_HPP_INCLUDED

// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_EFFECT_HPP_INCLUDED
#define LEXY_DSL_EFFECT_HPP_INCLUDED




namespace lexyd
{
template <typename EffRule, typename State>
using _detect_eff_fn = decltype(EffRule::_fn()(LEXY_DECLVAL(State&)));

template <LEXY_NTTP_PARAM Fn>
struct _eff : rule_base
{
    static constexpr auto _fn()
    {
        return Fn;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using control_block_type = LEXY_DECAY_DECLTYPE(*context.control_block);
            using state_type         = typename control_block_type::state_type;

            if constexpr (lexy::_detail::is_detected<_detect_eff_fn, _eff, state_type>)
            {
                using return_type = decltype(Fn(*context.control_block->parse_state));
                if constexpr (std::is_void_v<return_type>)
                {
                    Fn(*context.control_block->parse_state);
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                }
                else
                {
                    return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                             Fn(*context.control_block->parse_state));
                }
            }
            else
            {
                using return_type = decltype(Fn());
                if constexpr (std::is_void_v<return_type>)
                {
                    Fn();
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                }
                else
                {
                    return NextParser::parse(context, reader, LEXY_FWD(args)..., Fn());
                }
            }
        }
    };
};

/// Invokes Fn and produces its value as result.
template <LEXY_NTTP_PARAM Fn>
constexpr auto effect = _eff<Fn>{};
} // namespace lexyd

#endif // LEXY_DSL_EFFECT_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED




namespace lexy
{
struct expected_eof
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected EOF";
    }
};
} // namespace lexy

namespace lexyd
{
struct _eof : branch_base
{
    template <typename Reader>
    struct bp
    {
        constexpr bool try_parse(const void*, const Reader& reader)
        {
            return reader.peek() == Reader::encoding::eof();
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            auto pos = reader.position();
            context.on(_ev::token{}, lexy::eof_token_kind, pos, pos);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (reader.peek() != Reader::encoding::eof())
            {
                // Report that we've failed.
                auto err = lexy::error<Reader, lexy::expected_eof>(reader.position());
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }
            else
            {
                auto pos = reader.position();
                context.on(_ev::token{}, lexy::eof_token_kind, pos, pos);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

#endif // LEXY_DSL_EOF_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_EXPRESSION_HPP_INCLUDED
#define LEXY_DSL_EXPRESSION_HPP_INCLUDED




// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_OPERATOR_HPP_INCLUDED
#define LEXY_DSL_OPERATOR_HPP_INCLUDED






namespace lexyd
{
template <typename Condition, typename... R>
struct _br;
template <typename... R>
struct _seq_impl;
} // namespace lexyd

//=== tag type ===//
namespace lexy
{
template <typename Literal>
struct _op
{};
// GCC is buggy with auto parameters.
template <typename T, T Value>
struct _opv
{
    constexpr operator LEXY_DECAY_DECLTYPE(Value)() const
    {
        return Value;
    }
};

#if LEXY_HAS_NTTP
template <auto Operator>
#else
template <const auto& Operator>
#endif
using op = typename LEXY_DECAY_DECLTYPE(Operator)::op_tag_type;
} // namespace lexy

//=== op rule ===//
namespace lexy::_detail
{
template <typename... Literals>
struct op_lit_list
{
    static constexpr auto size = sizeof...(Literals);

    template <typename Encoding>
    static LEXY_CONSTEVAL auto _build_trie()
    {
        auto result = make_empty_trie<Encoding, Literals...>();

        auto value      = std::size_t(0);
        auto char_class = std::size_t(0);
        ((result.node_value[Literals::lit_insert(result, 0, char_class)] = value++,
          char_class += Literals::lit_char_classes.size),
         ...);

        return result;
    }
    template <typename Encoding>
    static constexpr lit_trie_for<Encoding, Literals...> trie = _build_trie<Encoding>();

    template <typename... T>
    constexpr auto operator+(op_lit_list<T...>) const
    {
        return op_lit_list<Literals..., T...>{};
    }
};

template <typename Reader>
struct parsed_operator
{
    typename Reader::marker cur;
    std::size_t             idx;
};

template <typename OpList, typename Reader>
constexpr auto parse_operator(Reader& reader)
{
    using encoding   = typename Reader::encoding;
    using op_matcher = lexy::_detail::lit_trie_matcher<OpList::template trie<encoding>, 0>;

    auto begin = reader.current();
    auto op    = op_matcher::try_match(reader);
    return parsed_operator<Reader>{begin, op};
}
} // namespace lexy::_detail

namespace lexyd
{
template <typename Tag, typename Reader>
using _detect_op_tag_ctor = decltype(Tag(LEXY_DECLVAL(Reader).position()));

template <typename Tag, typename Reader, typename Context>
using _detect_op_tag_ctor_with_state
    = decltype(Tag(*LEXY_DECLVAL(Context).control_block->parse_state,
                   LEXY_DECLVAL(Reader).position()));

template <typename TagType, typename Literal, typename... R>
struct _op : branch_base
{
    using op_tag_type = TagType;
    using op_literals = lexy::_detail::op_lit_list<Literal>;

    template <typename NextParser, typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool op_finish(Context& context, Reader& reader,
                                           lexy::_detail::parsed_operator<Reader> op,
                                           Args&&... args)
    {
        context.on(_ev::token{}, typename Literal::token_type{}, op.cur.position(),
                   reader.position());

        using continuation
            = lexy::whitespace_parser<Context, lexy::parser_for<_seq_impl<R...>, NextParser>>;
        if constexpr (std::is_void_v<TagType>)
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor_with_state, op_tag_type,
                                                      Reader, Context>)
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       op_tag_type(*context.control_block->parse_state, op.pos));
        else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor, op_tag_type, Reader>)
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       op_tag_type(op.cur.position()));
        else
            return continuation::parse(context, reader, LEXY_FWD(args)..., op_tag_type{});
    }

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Literal, Reader> impl;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return impl.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            impl.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            using continuation = lexy::parser_for<_seq_impl<R...>, NextParser>;

            if constexpr (std::is_void_v<TagType>)
                return impl.template finish<continuation>(context, reader, LEXY_FWD(args)...);
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor_with_state,
                                                          op_tag_type, Reader, Context>)
                return impl
                    .template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                   op_tag_type(*context.control_block->parse_state,
                                                               reader.position()));
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor, op_tag_type, Reader>)
                return impl.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                          op_tag_type(reader.position()));
            else
                return impl.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                          op_tag_type{});
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            [[maybe_unused]] auto pos = reader.position();

            using continuation
                = lexy::parser_for<Literal, lexy::parser_for<_seq_impl<R...>, NextParser>>;
            if constexpr (std::is_void_v<TagType>)
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor_with_state,
                                                          op_tag_type, Reader, Context>)
                return continuation::parse(context, reader, LEXY_FWD(args)...,
                                           op_tag_type(*context.control_block->parse_state, pos));
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor, op_tag_type, Reader>)
                return continuation::parse(context, reader, LEXY_FWD(args)..., op_tag_type(pos));
            else
                return continuation::parse(context, reader, LEXY_FWD(args)..., op_tag_type{});
        }
    };
};

template <typename Literal>
constexpr auto op(Literal)
{
    static_assert(lexy::is_literal_rule<Literal>);
    return _op<lexy::_op<Literal>, Literal>{};
}
template <typename Literal, typename... R>
constexpr auto op(_br<Literal, R...>)
{
    static_assert(lexy::is_literal_rule<Literal>,
                  "condition in the operator must be a literal rule");
    return _op<lexy::_op<Literal>, Literal, R...>{};
}

template <typename Tag, typename Literal>
constexpr auto op(Literal)
{
    static_assert(lexy::is_literal_rule<Literal>);
    return _op<Tag, Literal>{};
}
template <typename Tag, typename Literal, typename... R>
constexpr auto op(_br<Literal, R...>)
{
    static_assert(lexy::is_literal_rule<Literal>,
                  "condition in the operator must be a literal rule");
    return _op<Tag, Literal, R...>{};
}

template <auto Tag, typename Literal>
constexpr auto op(Literal)
{
    static_assert(lexy::is_literal_rule<Literal>);
    return _op<lexy::_opv<LEXY_DECAY_DECLTYPE(Tag), Tag>, Literal>{};
}
template <auto Tag, typename Literal, typename... R>
constexpr auto op(_br<Literal, R...>)
{
    static_assert(lexy::is_literal_rule<Literal>,
                  "condition in the operator must be a literal rule");
    return _op<lexy::_opv<LEXY_DECAY_DECLTYPE(Tag), Tag>, Literal, R...>{};
}
} // namespace lexyd

//=== op choice ===//
namespace lexyd
{
template <typename... Ops>
struct _opc : branch_base
{
    using op_literals = decltype((typename Ops::op_literals{} + ...));

    template <typename NextParser, typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool op_finish(Context& context, Reader& reader,
                                           lexy::_detail::parsed_operator<Reader> op,
                                           Args&&... args)
    {
        auto result = false;

        auto cur_idx = std::size_t(0);
        (void)((cur_idx == op.idx
                    ? (result = Ops::template op_finish<NextParser>(context, reader, op,
                                                                    LEXY_FWD(args)...),
                       true)
                    : (++cur_idx, false))
               || ...);

        return result;
    }

    template <typename Reader>
    struct bp
    {
        lexy::_detail::parsed_operator<Reader> op;
        typename Reader::marker                end;

        constexpr auto try_parse(const void*, Reader reader)
        {
            op  = lexy::_detail::parse_operator<op_literals>(reader);
            end = reader.current();
            return op.idx < op_literals::size;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            reader.reset(end);
            return op_finish<NextParser>(context, reader, op, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                auto err = lexy::error<Reader, lexy::expected_literal_set>(impl.op.cur.position());
                context.on(_ev::error{}, err);
                return false;
            }
            else
            {
                return impl.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
        }
    };
};

template <typename T1, typename L1, typename... R1, typename T2, typename L2, typename... R2>
constexpr auto operator/(_op<T1, L1, R1...> lhs, _op<T2, L2, R2...> rhs)
{
    return _opc<decltype(lhs), decltype(rhs)>{};
}
template <typename... Ops, typename T2, typename L2, typename... R2>
constexpr auto operator/(_opc<Ops...>, _op<T2, L2, R2...> rhs)
{
    return _opc<Ops..., decltype(rhs)>{};
}
template <typename T1, typename L1, typename... R1, typename... Ops>
constexpr auto operator/(_op<T1, L1, R1...> lhs, _opc<Ops...>)
{
    return _opc<decltype(lhs), Ops...>{};
}
template <typename... O1, typename... O2>
constexpr auto operator/(_opc<O1...>, _opc<O2...>)
{
    return _opc<O1..., O2...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_OPERATOR_HPP_INCLUDED

// Expression parsing algorithm uses an adapted version of Pratt parsing, as described here:
// https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
// In particular:
// * precedence specified implicitly by type type hierarchy
// * support for list and single precedence
// * support for operator groups that require additional parentheses
// * generate proper parse tree events

//=== dsl ===//
namespace lexyd
{
/// Operation that just parses the atomic rule.
struct atom : _operation_base
{
    static LEXY_CONSTEVAL auto name()
    {
        return "atom";
    }
};

/// Operation that selects between multiple ones.
template <typename... Operands>
struct groups : _operation_base
{};

struct infix_op_left : _operation_base // a ~ b ~ c == (a ~ b) ~ c
{};
struct infix_op_right : _operation_base // a ~ b ~ c == a ~ (b ~ c)
{};
struct infix_op_list : _operation_base // a ~ b ~ c kept as-is
{};
struct infix_op_single : _operation_base // a ~ b ~ c is an error
{};

struct postfix_op : _operation_base
{};

struct prefix_op : _operation_base
{};
} // namespace lexyd

//=== implementation ===//
namespace lexy::_detail
{
struct binding_power
{
    unsigned group;
    unsigned lhs, rhs;

    static constexpr auto left(unsigned group, unsigned level)
    {
        return binding_power{group, 2 * level, 2 * level + 1};
    }
    static constexpr auto right(unsigned group, unsigned level)
    {
        return binding_power{group, 2 * level + 1, 2 * level};
    }
    static constexpr auto prefix(unsigned group, unsigned level)
    {
        // prefix is sort of left-associative, so right side odd.
        return binding_power{group, 0, 2 * level + 1};
    }
    static constexpr auto postfix(unsigned group, unsigned level)
    {
        // postfix is sort of right-associative, so left side odd.
        return binding_power{group, 2 * level + 1, 0};
    }

    constexpr bool is_valid() const
    {
        return lhs > 0 || rhs > 0;
    }
    constexpr bool is_infix() const
    {
        return lhs > 0 && rhs > 0;
    }
    constexpr bool is_postfix() const
    {
        return lhs > 0 && rhs == 0;
    }
    constexpr bool is_prefix() const
    {
        return lhs == 0 && rhs > 0;
    }
};

template <typename Operation>
constexpr binding_power get_binding_power(unsigned cur_group, unsigned cur_level)
{
    if constexpr (std::is_base_of_v<lexyd::infix_op_left, Operation>
                  // We treat a list as left associative operator for simplicity here.
                  // It doesn't really matter, as it will only consider operators from the
                  // same operation anyway.
                  || std::is_base_of_v<lexyd::infix_op_list, Operation>
                  // For the purposes of error recovery, single is left associative.
                  || std::is_base_of_v<lexyd::infix_op_single, Operation>)
        return binding_power::left(cur_group, cur_level);
    else if constexpr (std::is_base_of_v<lexyd::infix_op_right, Operation>)
        return binding_power::right(cur_group, cur_level);
    else if constexpr (std::is_base_of_v<lexyd::prefix_op, Operation>)
        return binding_power::prefix(cur_group, cur_level);
    else if constexpr (std::is_base_of_v<lexyd::postfix_op, Operation>)
        return binding_power::postfix(cur_group, cur_level);
}

template <typename DestOperation>
struct _binding_power_of
{
    static constexpr auto transition(lexyd::atom, unsigned cur_group, unsigned)
    {
        // Not found, return an invalid operator, but return the current group.
        // This is the highest group encountered.
        return binding_power{cur_group, 0, 0};
    }
    template <typename CurOperation>
    static constexpr auto transition(CurOperation op, unsigned cur_group, unsigned cur_level)
    {
        // Normal operation: keep group the same, but increment level.
        return get(op, cur_group, cur_level + 1);
    }
    template <typename... Operations>
    static constexpr auto transition(lexyd::groups<Operations...>, unsigned cur_group,
                                     unsigned cur_level)
    {
        auto result = binding_power{cur_group, 0, 0};
        // Try to find the destination in each group.
        // Before we transition, we increment the group to create a new one,
        // afterwards we update group to the highest group encountered so far.
        // That way, we don't re-use group numbers.
        // Note that we don't increment the level, as that is handled by the overload above.
        (void)((result    = transition(Operations{}, cur_group + 1, cur_level),
                cur_group = result.group, result.is_valid())
               || ...);
        return result;
    }

    template <typename CurOperation>
    static constexpr auto get(CurOperation, unsigned cur_group, unsigned cur_level)
    {
        if constexpr (std::is_same_v<DestOperation, CurOperation>)
            return get_binding_power<CurOperation>(cur_group, cur_level);
        else
            return transition(typename CurOperation::operand{}, cur_group, cur_level);
    }
};

// Returns the binding power of an operator in an expression.
template <typename Expr, typename Operation>
constexpr auto binding_power_of(Operation)
{
    return _binding_power_of<Operation>::transition(typename Expr::operation{}, 0, 0);
}
} // namespace lexy::_detail

namespace lexy::_detail
{
template <typename Operation>
using op_of = LEXY_DECAY_DECLTYPE(Operation::op);

template <typename... Operations>
struct operation_list
{
    template <typename T>
    constexpr auto operator+(T) const
    {
        return operation_list<Operations..., T>{};
    }
    template <typename... T>
    constexpr auto operator+(operation_list<T...>) const
    {
        return operation_list<Operations..., T...>{};
    }

    static constexpr auto size = sizeof...(Operations);

    using ops = decltype((typename op_of<Operations>::op_literals{} + ... + op_lit_list{}));

    template <template <typename> typename Continuation, typename Context, typename Reader,
              typename... Args>
    static constexpr bool apply(Context& context, Reader& reader, parsed_operator<Reader> op,
                                Args&&... args)
    {
        auto result = false;

        auto cur_idx = std::size_t(0);
        (void)((cur_idx <= op.idx && op.idx < cur_idx + op_of<Operations>::op_literals::size
                    ? (result
                       = Continuation<Operations>::parse(context, reader,
                                                         parsed_operator<Reader>{op.cur,
                                                                                 op.idx - cur_idx},
                                                         LEXY_FWD(args)...),
                       true)
                    : (cur_idx += op_of<Operations>::op_literals::size, false))
               || ...);

        return result;
    }
};

template <bool Pre, unsigned MinBindingPower>
struct _operation_list_of
{
    template <unsigned CurLevel>
    static constexpr auto get(lexyd::atom)
    {
        return operation_list{};
    }
    template <unsigned CurLevel, typename... Operations>
    static constexpr auto get(lexyd::groups<Operations...>)
    {
        return (get<CurLevel>(Operations{}) + ...);
    }
    template <unsigned CurLevel, typename Operation>
    static constexpr auto get(Operation)
    {
        constexpr auto bp = get_binding_power<Operation>(0, CurLevel);

        auto           tail      = get<CurLevel + 1>(typename Operation::operand{});
        constexpr auto is_prefix = std::is_base_of_v<lexyd::prefix_op, Operation>;
        if constexpr (is_prefix == Pre
                      && ((is_prefix && bp.rhs >= MinBindingPower)
                          || (!is_prefix && bp.lhs >= MinBindingPower)))
            return tail + Operation{};
        else
            return tail;
    }
};

// prefix operations
template <typename Expr, unsigned MinBindingPower>
using pre_operation_list_of = decltype(_operation_list_of<true, MinBindingPower>::template get<1>(
    typename Expr::operation{}));

// infix and postfix operations
template <typename Expr, unsigned MinBindingPower>
using post_operation_list_of = decltype(_operation_list_of<false, MinBindingPower>::template get<1>(
    typename Expr::operation{}));
} // namespace lexy::_detail

//=== expression rule ===//
namespace lexyd
{
template <typename RootOperation>
struct _expr : rule_base
{
    struct _state
    {
        unsigned cur_group         = 0;
        unsigned cur_nesting_level = 0;
    };

    template <typename Operation>
    struct _continuation
    {
        struct _op_cont
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, _state& state,
                                               Args&&... op_args)
            {
                using namespace lexy::_detail;

                constexpr auto value_type_void = std::is_void_v<typename Context::value_type>;
                constexpr auto binding_power
                    = binding_power_of<typename Context::production>(Operation{});

                if constexpr (std::is_base_of_v<infix_op_list, Operation>)
                {
                    // We need to handle a list infix operator specially,
                    // and parse an arbitrary amount of lhs.
                    // For that, we use a loop and a sink.
                    auto sink = context.value_callback().sink();

                    // We need to pass the initial lhs to the sink.
                    if constexpr (!value_type_void)
                        sink(*LEXY_MOV(context.value));
                    context.value = {};

                    // As well as the operator we've already got.
                    sink(LEXY_FWD(op_args)...);

                    auto result = true;
                    while (true)
                    {
                        // Parse (another) value.
                        if (!_parse<binding_power.rhs>(context, reader, state))
                        {
                            result = false;
                            break;
                        }

                        if constexpr (!value_type_void)
                            sink(*LEXY_MOV(context.value));
                        context.value = {};

                        using op_rule = op_of<Operation>;
                        auto op       = parse_operator<typename op_rule::op_literals>(reader);
                        if (op.idx >= op_rule::op_literals::size)
                        {
                            // The list ends at this point.
                            reader.reset(op.cur);
                            break;
                        }

                        // Need to finish the operator properly, by passing it to the sink.
                        if (!op_rule::template op_finish<lexy::sink_parser>(context, reader, op,
                                                                            sink))
                        {
                            result = false;
                            break;
                        }
                    }

                    // We store the final value of the sink no matter the parse result.
                    if constexpr (value_type_void)
                    {
                        LEXY_MOV(sink).finish();
                        context.value.emplace();
                    }
                    else
                    {
                        context.value.emplace(LEXY_MOV(sink).finish());
                    }

                    // If we've failed at any point, propagate failure now.
                    if (!result)
                        return false;
                }
                else if constexpr (binding_power.is_prefix())
                {
                    if (!_parse<binding_power.rhs>(context, reader, state))
                        return false;

                    auto value    = LEXY_MOV(context.value);
                    context.value = {};

                    if constexpr (value_type_void)
                        context.value.emplace_result(context.value_callback(),
                                                     LEXY_FWD(op_args)...);
                    else
                        context.value.emplace_result(context.value_callback(), LEXY_FWD(op_args)...,
                                                     *LEXY_MOV(value));
                }
                else if constexpr (binding_power.is_infix())
                {
                    auto lhs      = LEXY_MOV(context.value);
                    context.value = {};

                    if (!_parse<binding_power.rhs>(context, reader, state))
                    {
                        // Put it back, so we can properly recover.
                        context.value = LEXY_MOV(lhs);
                        return false;
                    }

                    auto rhs      = LEXY_MOV(context.value);
                    context.value = {};

                    if constexpr (value_type_void)
                        context.value.emplace_result(context.value_callback(),
                                                     LEXY_FWD(op_args)...);
                    else
                        context.value.emplace_result(context.value_callback(), *LEXY_MOV(lhs),
                                                     LEXY_FWD(op_args)..., *LEXY_MOV(rhs));

                    if constexpr (std::is_base_of_v<infix_op_single, Operation>)
                    {
                        using op_rule = op_of<Operation>;
                        auto op       = parse_operator<typename op_rule::op_literals>(reader);
                        if (op.idx < op_rule::op_literals::size)
                        {
                            using tag = typename Context::production::operator_chain_error;
                            auto err
                                = lexy::error<Reader, tag>(op.cur.position(), reader.position());
                            context.on(_ev::error{}, err);
                        }
                        reader.reset(op.cur);
                    }
                }
                else if constexpr (binding_power.is_postfix())
                {
                    auto value    = LEXY_MOV(context.value);
                    context.value = {};

                    if constexpr (value_type_void)
                        context.value.emplace_result(context.value_callback(),
                                                     LEXY_FWD(op_args)...);
                    else
                        context.value.emplace_result(context.value_callback(), *LEXY_MOV(value),
                                                     LEXY_FWD(op_args)...);
                }

                context.on(_ev::operation_chain_op{}, Operation{}, reader.position());
                return true;
            }
        };

        template <typename Context, typename Reader>
        static constexpr bool parse(Context& context, Reader& reader,
                                    lexy::_detail::parsed_operator<Reader> op, _state& state)
        {
            using namespace lexy::_detail;
            using production = typename Context::production;

            // Check whether we might have nested to far.
            if (state.cur_nesting_level++ >= production::max_operator_nesting)
            {
                using tag = typename production::operator_nesting_error;
                auto err  = lexy::error<Reader, tag>(op.cur.position(), reader.position());
                context.on(_ev::error{}, err);

                // We do not recover, to prevent stack overflow.
                reader.reset(op.cur);
                return false;
            }

            // If the operator is part of a group, check whether it matches.
            constexpr auto binding_power = binding_power_of<production>(Operation{});
            if constexpr (binding_power.group != 0)
            {
                if (state.cur_group == 0)
                {
                    // We didn't have any operator group yet, set it.
                    state.cur_group = binding_power.group;
                }
                else if (state.cur_group != binding_power.group)
                {
                    // Operators can't be grouped.
                    using tag = typename production::operator_group_error;
                    auto err  = lexy::error<Reader, tag>(op.cur.position(), reader.position());
                    context.on(_ev::error{}, err);
                    // Trivially recover, but don't update group:
                    // let the first one stick.
                }
            }

            // Finish the operator and parse a RHS, if necessary.
            return op_of<Operation>::template op_finish<_op_cont>(context, reader, op, state);
        }
    };

    template <unsigned MinBindingPower, typename Context, typename Reader>
    static constexpr bool _parse_lhs(Context& context, Reader& reader, _state& state)
    {
        using namespace lexy::_detail;

        using op_list = pre_operation_list_of<typename Context::production, MinBindingPower>;
        using atom_parser
            = lexy::parser_for<LEXY_DECAY_DECLTYPE(Context::production::atom), final_parser>;

        if constexpr (op_list::size == 0)
        {
            // We don't have any prefix operators, so parse an atom directly.
            (void)state;
            return atom_parser::parse(context, reader);
        }
        else
        {
            auto op = lexy::_detail::parse_operator<typename op_list::ops>(reader);
            if (op.idx >= op_list::ops::size)
            {
                // We don't have a prefix operator, so it must be an atom.
                reader.reset(op.cur);
                return atom_parser::parse(context, reader);
            }

            auto start_event = context.on(_ev::operation_chain_start{}, op.cur.position());
            auto result      = op_list::template apply<_continuation>(context, reader, op, state);
            context.on(_ev::operation_chain_finish{}, LEXY_MOV(start_event), reader.position());
            return result;
        }
    }

    template <unsigned MinBindingPower, typename Context, typename Reader>
    static constexpr bool _parse(Context& context, Reader& reader, _state& state)
    {
        using namespace lexy::_detail;
        using op_list = post_operation_list_of<typename Context::production, MinBindingPower>;

        if constexpr (op_list::size == 0)
        {
            // We don't have any post operators, so we only parse the left-hand-side.
            return _parse_lhs<MinBindingPower>(context, reader, state);
        }
        else
        {
            auto start_event = context.on(_ev::operation_chain_start{}, reader.position());
            if (!_parse_lhs<MinBindingPower>(context, reader, state))
            {
                context.on(_ev::operation_chain_finish{}, LEXY_MOV(start_event), reader.position());
                return false;
            }

            auto result = true;
            while (true)
            {
                auto op = parse_operator<typename op_list::ops>(reader);
                if (op.idx >= op_list::ops::size)
                {
                    reader.reset(op.cur);
                    break;
                }

                result = op_list::template apply<_continuation>(context, reader, op, state);
                if (!result)
                    break;
            }

            context.on(_ev::operation_chain_finish{}, LEXY_MOV(start_event), reader.position());
            return result;
        }

        return false; // unreachable
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader)
        {
            static_assert(std::is_same_v<NextParser, lexy::_detail::final_parser>);

            using production             = typename Context::production;
            constexpr auto binding_power = lexy::_detail::binding_power_of<production>(
                lexy::_detail::type_or<RootOperation, typename production::operation>{});
            // The MinBindingPower is determined by the root operation.
            // The initial operand is always on the left, so we use the left binding power.
            // However, for a prefix operator it is zero, but then it's a right operand so we use
            // that.
            constexpr auto min_binding_power
                = binding_power.is_prefix() ? binding_power.rhs : binding_power.lhs;

            _state state;
            _parse<min_binding_power>(context, reader, state);

            // Regardless of parse errors, we can recover if we already had a value at some point.
            return !!context.value;
        }
    };
};
} // namespace lexyd

//=== expression_production ===//
namespace lexy
{
struct max_operator_nesting_exceeded
{
    static LEXY_CONSTEVAL auto name()
    {
        return "maximum operator nesting level exceeded";
    }
};

struct operator_chain_error
{
    static LEXY_CONSTEVAL auto name()
    {
        return "operator cannot be chained";
    }
};

struct operator_group_error
{
    static LEXY_CONSTEVAL auto name()
    {
        return "operator cannot be mixed with previous operators";
    }
};

struct expression_production
{
    using operator_nesting_error               = lexy::max_operator_nesting_exceeded;
    static constexpr auto max_operator_nesting = 256; // arbitrary power of two

    using operator_chain_error = lexy::operator_chain_error;
    using operator_group_error = lexy::operator_group_error;

    static constexpr auto rule = lexyd::_expr<void>{};
};

template <typename Expr, typename RootOperation>
struct subexpression_production : Expr
{
    static constexpr auto rule = lexyd::_expr<RootOperation>{};
};
} // namespace lexy

#endif // LEXY_DSL_EXPRESSION_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_FLAGS_HPP_INCLUDED
#define LEXY_DSL_FLAGS_HPP_INCLUDED




namespace lexy
{
struct duplicate_flag
{
    static LEXY_CONSTEVAL auto name()
    {
        return "duplicate flag";
    }
};
} // namespace lexy

namespace lexyd
{
template <const auto& Table, typename Token, typename Tag>
struct _sym;

template <typename FlagRule, auto Default, typename DuplicateError = void>
struct _flags : rule_base
{
    using _enum_type = LEXY_DECAY_DECLTYPE(Default);
    using _int_type  = std::underlying_type_t<_enum_type>;

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader>
        static constexpr bool _parse(_int_type& result, Context& context, Reader& reader)
        {
            result = _int_type(Default);

            while (true)
            {
                auto begin = reader.position();

                lexy::branch_parser_for<FlagRule, Reader> bp{};
                if (!bp.try_parse(context.control_block, reader))
                {
                    bp.cancel(context);
                    break;
                }

                if (!bp.template finish<lexy::pattern_parser<_enum_type>>(context, reader))
                    return false;

                auto flag = _int_type(bp.value());
                if ((result & flag) == flag)
                {
                    using tag = lexy::_detail::type_or<DuplicateError, lexy::duplicate_flag>;
                    auto err  = lexy::error<Reader, tag>(begin, reader.position());
                    context.on(_ev::error{}, err);
                    // We can trivially recover.
                }
                result |= flag;
            }

            return true;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _int_type result{};
            if (!_parse(result, context, reader))
                return false;
            return NextParser::parse(context, reader, LEXY_FWD(args)..., _enum_type(result));
        }
    };

    template <typename Tag>
    static constexpr _flags<FlagRule, Default, Tag> error = {};
};

template <auto Default, const auto& Table, typename Token, typename Tag>
constexpr auto flags(_sym<Table, Token, Tag> flag_rule)
{
    using table_type = LEXY_DECAY_DECLTYPE(Table);
    using enum_type  = LEXY_DECAY_DECLTYPE(Default);
    static_assert(std::is_same_v<enum_type, typename table_type::mapped_type>);
    static_assert(std::is_enum_v<enum_type>);

    return _flags<decltype(flag_rule), Default>{};
}
template <const auto& Table, typename Token, typename Tag>
constexpr auto flags(_sym<Table, Token, Tag> flag_rule)
{
    using table_type = LEXY_DECAY_DECLTYPE(Table);
    using enum_type  = typename table_type::mapped_type;
    static_assert(std::is_enum_v<enum_type>);

    return _flags<decltype(flag_rule), enum_type{}>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule, auto If, auto Else>
struct _flag : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Rule, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)..., If);
            else
            {
                branch.cancel(context);
                return NextParser::parse(context, reader, LEXY_FWD(args)..., Else);
            }
        }
    };
};

template <auto If, auto Else = LEXY_DECAY_DECLTYPE(If){}, typename Rule>
constexpr auto flag(Rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "flag()");
    return _flag<Rule, If, Else>{};
}

template <typename Rule>
constexpr auto flag(Rule rule)
{
    return flag<true, false>(rule);
}
} // namespace lexyd

#endif // LEXY_DSL_FLAGS_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_FOLLOW_HPP_INCLUDED
#define LEXY_DSL_FOLLOW_HPP_INCLUDED





namespace lexy
{
struct follow_restriction
{
    static LEXY_CONSTEVAL auto name()
    {
        return "follow restriction";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Literal, typename CharClass>
struct _nf : token_base<_nf<Literal, CharClass>>, _lit_base
{
    static constexpr auto lit_max_char_count = Literal::lit_max_char_count;

    static constexpr auto lit_char_classes = lexy::_detail::char_class_list<CharClass>{};

    using lit_case_folding = typename Literal::lit_case_folding;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        return Literal::template lit_first_char<Encoding>();
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos,
                                                 std::size_t char_class)
    {
        auto end                  = Literal::lit_insert(trie, pos, char_class);
        trie.node_char_class[end] = char_class;
        return end;
    }

    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Literal, Reader> impl;
        typename Reader::marker                 end;
        bool                                    literal_success;

        constexpr explicit tp(const Reader& reader)
        : impl(reader), end(reader.current()), literal_success(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            literal_success = false;

            // Need to match the literal.
            if (!impl.try_parse(reader))
                return false;
            end             = impl.end;
            literal_success = true;

            // To match, we must not match the char class now.
            reader.reset(end);
            if constexpr (std::is_void_v<lit_case_folding>)
            {
                return !lexy::try_match_token(CharClass{}, reader);
            }
            else
            {
                typename lit_case_folding::template reader<Reader> case_folded{reader};
                return !lexy::try_match_token(CharClass{}, case_folded);
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            if (!literal_success)
            {
                impl.report_error(context, reader);
            }
            else
            {
                auto err
                    = lexy::error<Reader, lexy::follow_restriction>(end.position(), end.position());
                context.on(_ev::error{}, err);
            }
        }
    };
};

/// Match a literal but only if not followed by the given char class.
template <typename Literal, typename CharClass>
constexpr auto not_followed_by(Literal, CharClass cc)
{
    static_assert(lexy::is_literal_rule<Literal> && Literal::lit_char_classes.size == 0);
    return _nf<Literal, decltype(_make_char_class(cc))>{};
}

/// Match a literal but only if followed by the given char class.
template <typename Literal, typename CharClass>
constexpr auto followed_by(Literal lit, CharClass cc)
{
    return not_followed_by(lit, -cc);
}
} // namespace lexyd

namespace lexy
{
template <typename Literal, typename CharClass>
constexpr auto token_kind_of<lexy::dsl::_nf<Literal, CharClass>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_FOLLOW_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_IF_HPP_INCLUDED
#define LEXY_DSL_IF_HPP_INCLUDED




namespace lexyd
{
template <typename Branch>
struct _if : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Branch, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                // We take the branch.
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            else
            {
                // We don't take the branch.
                branch.cancel(context);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };
};

/// If the branch condition matches, matches the branch then.
template <typename Branch>
constexpr auto if_(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "if()");
    if constexpr (lexy::is_unconditional_branch_rule<Branch>)
        // Branch is always taken, so don't wrap in if_().
        return Branch{};
    else
        return _if<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_IF_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
#ifdef __SIZEOF_INT128__
        else if constexpr (std::is_same_v<T, __int128_t>)
            return __int128_t(~__uint128_t{} >> 1);
        else if constexpr (std::is_same_v<T, __uint128_t>)
            return ~__uint128_t{};
#endif
        else
            static_assert(_detail::error<T>,
                          "specialize integer_traits for your custom integer types");
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

template <typename T, T Max>
struct bounded
{};
template <typename T, T Max>
struct integer_traits<bounded<T, Max>>
{
    using type                       = typename integer_traits<T>::type;
    static constexpr auto is_bounded = true;

    template <int Radix>
    static constexpr std::size_t max_digit_count = _digit_count(Radix, Max);

    template <int Radix>
    static constexpr void add_digit_unchecked(type& result, unsigned digit)
    {
        integer_traits<T>::template add_digit_unchecked<Radix>(result, digit);
    }
    template <int Radix>
    static constexpr bool add_digit_checked(type& result, unsigned digit)
    {
        return integer_traits<T>::template add_digit_checked<Radix>(result, digit) && result <= Max;
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

// Parses T in the Base without checking for overflow.
template <typename T, typename Base>
struct _unbounded_integer_parser
{
    using traits = lexy::integer_traits<T>;
    using base   = Base;

    static constexpr auto radix = Base::digit_radix;

    struct result_type
    {
        typename traits::type value;
        std::false_type       overflow;
    };

    template <typename Iterator>
    static constexpr result_type parse(Iterator cur, Iterator end)
    {
        typename traits::type value(0);

        // Just parse digits until we've run out of digits.
        while (cur != end)
        {
            auto digit = Base::digit_value(*cur++);
            if (digit >= Base::digit_radix)
                // Skip digit separator.
                continue;

            traits::template add_digit_unchecked<radix>(value, digit);
        }

        return {value, {}};
    }
};

// Parses T in the Base while checking for overflow.
template <typename T, typename Base, bool AssumeOnlyDigits>
struct _bounded_integer_parser
{
    using traits = lexy::integer_traits<T>;
    using base   = Base;

    static constexpr auto radix           = Base::digit_radix;
    static constexpr auto max_digit_count = traits::template max_digit_count<radix>;
    static_assert(max_digit_count > 1, "integer must be able to store all possible digit values");

    struct result_type
    {
        typename traits::type value;
        bool                  overflow;
    };

    template <typename Iterator>
    static constexpr result_type parse(Iterator cur, Iterator end)
    {
        // Find the first non-zero digit.
        // Note that we always need a loop, even if leading zeros are not allowed:
        // error recovery might get them anyway.
        auto first_digit = 0u;
        while (true)
        {
            if (cur == end)
                return {typename traits::type(0), false};

            first_digit = Base::digit_value(*cur++);
            if (first_digit != 0 && first_digit < radix)
                break;
        }

        // At this point, we've parsed exactly one non-zero digit, so we can assign.
        auto value = typename traits::type(first_digit);

        // Handle at most the number of remaining digits.
        // Due to the fixed loop count, it is most likely unrolled.
        for (std::size_t digit_count = 1; digit_count < max_digit_count; ++digit_count)
        {
            // Find the next digit.
            auto digit = 0u;
            do
            {
                if (cur == end)
                    return {value, false};

                digit = Base::digit_value(*cur++);
                // If we can assume it's a digit, we don't need the comparison.
            } while (AssumeOnlyDigits ? false : digit >= Base::digit_radix);

            // We need to handle the last loop iteration special.
            // (The compiler will not generate a branch here.)
            if (digit_count == max_digit_count - 1)
            {
                // The last digit might overflow, so check for it.
                if (!traits::template add_digit_checked<radix>(value, digit))
                    return {value, true};
            }
            else
            {
                // Add the digit without checking as it can't overflow.
                traits::template add_digit_unchecked<radix>(value, digit);
            }
        }

        // If we've reached this point, we've parsed the maximal number of digits allowed.
        // Now we can only overflow if there are still digits left.
        return {value, cur != end};
    }
};
template <typename T, typename Base, bool AssumeOnlyDigits>
using _integer_parser
    = std::conditional_t<_is_bounded<T>, _bounded_integer_parser<T, Base, AssumeOnlyDigits>,
                         _unbounded_integer_parser<T, Base>>;

template <typename T, typename Digits>
struct _integer_parser_digits;
template <typename T, typename Base>
struct _integer_parser_digits<T, _digits<Base>>
{
    using type = _integer_parser<T, Base, true>;
};
template <typename T, typename Base>
struct _integer_parser_digits<T, _digits_t<Base>>
{
    using type = _integer_parser<T, Base, true>;
};
template <typename T, typename Base, typename Sep>
struct _integer_parser_digits<T, _digits_s<Base, Sep>>
{
    using type = _integer_parser<T, Base, false>;
};
template <typename T, typename Base, typename Sep>
struct _integer_parser_digits<T, _digits_st<Base, Sep>>
{
    using type = _integer_parser<T, Base, false>;
};
template <typename T, std::size_t N, typename Base>
struct _integer_parser_digits<T, _ndigits<N, Base>>
{
    using value_type = std::conditional_t<_ndigits_can_overflow<T, N, Base::digit_radix>(), T,
                                          lexy::unbounded<T>>;
    using type       = _integer_parser<value_type, Base, true>;
};
template <typename T, std::size_t N, typename Base, typename Sep>
struct _integer_parser_digits<T, _ndigits_s<N, Base, Sep>>
{
    using value_type = std::conditional_t<_ndigits_can_overflow<T, N, Base::digit_radix>(), T,
                                          lexy::unbounded<T>>;
    using type       = _integer_parser<value_type, Base, false>;
};

template <typename T, typename Digits>
using _integer_parser_for = typename _integer_parser_digits<T, Digits>::type;

template <typename Token, typename IntParser, typename Tag>
struct _int : _copy_base<Token>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           typename Reader::iterator begin,
                                           typename Reader::iterator end, Args&&... args)
        {
            auto [value, overflow] = IntParser::parse(begin, end);
            if (overflow)
            {
                // Raise error but recover.
                using tag = lexy::_detail::type_or<Tag, lexy::integer_overflow>;
                context.on(_ev::error{}, lexy::error<Reader, tag>(begin, end));
            }

            // Need to skip whitespace now as well.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)..., value);
        }
    };

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, Token{}, begin, end.position());
            reader.reset(end);

            return _pc<NextParser>::parse(context, reader, begin, end.position(),
                                          LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            if (lexy::token_parser_for<Token, Reader> parser(reader); parser.try_parse(reader))
            {
                context.on(_ev::token{}, typename Token::token_type{}, begin,
                           parser.end.position());
                reader.reset(parser.end);
            }
            else
            {
                parser.report_error(context, reader);
                reader.reset(parser.end);

                // To recover we try and skip additional digits.
                while (lexy::try_match_token(digit<typename IntParser::base>, reader))
                {
                }

                auto recovery_end = reader.position();
                if (begin == recovery_end)
                {
                    // We didn't get any digits; couldn't recover.
                    // We don't report error recovery events as nothing was done;
                    // we don't need to create an error token as nothing was consumed.
                    return false;
                }
                else
                {
                    // We've succesfully recovered, mark everything as digits.
                    context.on(_ev::recovery_start{}, begin);
                    context.on(_ev::token{}, lexy::digits_token_kind, begin, recovery_end);
                    context.on(_ev::recovery_finish{}, recovery_end);
                }
            }
            auto end = reader.position();

            return _pc<NextParser>::parse(context, reader, begin, end, LEXY_FWD(args)...);
        }
    };
};

template <typename T, typename Base>
struct _int_dsl : _int<_digits<lexy::_detail::type_or<Base, decimal>>,
                       _integer_parser_for<T, _digits<lexy::_detail::type_or<Base, decimal>>>, void>
{
    template <typename Digits>
    constexpr auto operator()(Digits) const
    {
        static_assert(lexy::is_token_rule<Digits>);
        if constexpr (std::is_void_v<Base>)
        {
            // Digits is a known rule as the user didn't specify Base.
            return _int<Digits, _integer_parser_for<T, Digits>, void>{};
        }
        else
        {
            // User has specified a base, so the digits are arbitrary.
            using parser = _integer_parser<T, Base, false>;
            return _int<Digits, parser, void>{};
        }
    }
};

/// Parses the digits matched by the rule into an integer type.
template <typename T, typename Base = void>
constexpr auto integer = _int_dsl<T, Base>{};
} // namespace lexyd

//=== code_point_id ===//
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
/// Matches the integer value of a code point.
template <std::size_t N, typename Base = hex>
constexpr auto code_point_id = [] {
    using type = std::conditional_t<_ndigits_can_overflow<lexy::code_point, N, Base::digit_radix>(),
                                    lexy::code_point, lexy::unbounded<lexy::code_point>>;
    using parser = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, parser, lexy::invalid_code_point>{};
}();
} // namespace lexyd

//=== code_unit_id ===//
namespace lexy
{
struct invalid_code_unit
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid code unit";
    }
};
} // namespace lexy

namespace lexyd
{
/// Matches the integer value of a code unit.
template <typename Encoding, std::size_t N, typename Base = hex>
constexpr auto code_unit_id = [] {
    using char_type = typename Encoding::char_type;
    using type      = std::conditional_t<_ndigits_can_overflow<char_type, N, Base::digit_radix>(),
                                         char_type, lexy::unbounded<char_type>>;
    using parser    = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, parser, lexy::invalid_code_unit>{};
}();
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED



// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED





namespace lexy
{
// An optional type is something that has the following:
// * a default constructor: this means we can actually construct it from our `nullopt`
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Branch, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                // We take the branch.
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            else
            {
                // We don't take the branch and produce a nullopt.
                branch.cancel(context);
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
            }
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
constexpr auto opt(Rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "opt()");
    if constexpr (lexy::is_unconditional_branch_rule<Rule>)
        // Branch is always taken, so don't wrap in opt().
        return Rule{};
    else
        return _opt<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Rule>
struct _optt : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Try to parse the terminator.
            lexy::branch_parser_for<Term, Reader> term{};
            if (term.try_parse(context.control_block, reader))
                // We had the terminator, so produce nullopt.
                return term.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                        lexy::nullopt{});
            term.cancel(context);

            // We didn't have the terminator, so we parse the rule.
            using parser = lexy::parser_for<Rule, NextParser>;
            return parser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED





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
// Reports the trailing sep error.
template <typename Branch, typename Tag>
struct _nsep : rule_base
{
    template <typename NextParser>
    struct p
    {
        struct _pc
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                               typename Reader::iterator sep_begin, Args&&... args)
            {
                auto sep_end = reader.position();

                using tag = lexy::_detail::type_or<Tag, lexy::unexpected_trailing_separator>;
                auto err  = lexy::error<Reader, tag>(sep_begin, sep_end);
                context.on(_ev::error{}, err);

                // Trivially recover.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (lexy::branch_parser_for<Branch, Reader> parser{};
                !parser.try_parse(context.control_block, reader))
            {
                // Didn't have the separator, everything is okay.
                parser.cancel(context);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // Did have the separator, report error.
                return parser.template finish<_pc>(context, reader, reader.position(),
                                                   LEXY_FWD(args)...);
            }
        }
    };
};

template <typename Branch, typename Tag>
struct _sep : _sep_base
{
    using rule          = Branch;
    using trailing_rule = _nsep<Branch, Tag>;

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&                  context, Reader&,
                                                typename Reader::iterator sep_begin,
                                                typename Reader::iterator sep_end)
    {
        using tag = lexy::_detail::type_or<Tag, lexy::unexpected_trailing_separator>;
        auto err  = lexy::error<Reader, tag>(sep_begin, sep_end);
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
    LEXY_REQUIRE_BRANCH_RULE(Branch, "sep");
    return _sep<Branch, void>{};
}

template <typename Branch>
struct _tsep : _sep_base
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{}));

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&, Reader&, typename Reader::iterator,
                                                typename Reader::iterator)
    {}
};

/// Defines a separator for a list that can be trailing.
template <typename Branch>
constexpr auto trailing_sep(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "trailing_sep");
    return _tsep<Branch>{};
}

template <typename Branch>
struct _isep : _sep_base
{
    using rule          = Branch;
    using trailing_rule = _else;

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&, Reader&, typename Reader::iterator,
                                                typename Reader::iterator)
    {}
};

/// Defines a separator for a list that ignores the existence of trailing separators.
template <typename Branch>
constexpr auto ignore_trailing_sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _isep<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED


namespace lexyd
{
template <typename Item, typename Sep>
struct _lst : _copy_base<Item>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_PARSER_FUNC static bool _loop(Context& context, Reader& reader, Sink& sink)
    {
        while (true)
        {
            // Parse a separator if necessary.
            [[maybe_unused]] auto sep_begin = reader.position();
            if constexpr (!std::is_void_v<Sep>)
            {
                lexy::branch_parser_for<typename Sep::rule, Reader> sep{};
                if (!sep.try_parse(context.control_block, reader))
                {
                    // We didn't have a separator, list is definitely finished.
                    sep.cancel(context);
                    break;
                }

                if (!sep.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }
            [[maybe_unused]] auto sep_end = reader.position();

            // Parse the next item.
            if constexpr (lexy::is_branch_rule<Item>)
            {
                // It's a branch, so try parsing it to detect loop exit.
                lexy::branch_parser_for<Item, Reader> item{};
                if (!item.try_parse(context.control_block, reader))
                {
                    // We don't have a next item, exit the loop.
                    // If necessary, we report a trailing separator.
                    item.cancel(context);
                    if constexpr (!std::is_void_v<Sep>)
                        Sep::report_trailing_error(context, reader, sep_begin, sep_end);
                    break;
                }

                // We're having an item, finish it.
                if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }
            else
            {
                // Not a branch, so we need one item.
                if (!lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                    return false;
            }
        }

        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Construct the sink.
            auto sink = context.value_callback().sink();

            // Parse the first item.
            if (!lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                return false;

            // Parse the remaining items.
            if (!_loop(context, reader, sink))
                return false;

            // We're done with the list, finish the sink and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Item, Reader> item;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader& reader)
        {
            // We parse a list if we can parse its first item.
            return item.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            return item.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // At this point, we have a list so construct a sink.
            auto sink = context.value_callback().sink();

            // Finish the first item, passing all values to the sink.
            if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                return false;

            // Parse the remaining items.
            if (!_loop(context, reader, sink))
                return false;

            // We're done with the list, finish the sink and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};

/// Parses a list of items without a separator.
template <typename Item>
constexpr auto list(Item)
{
    LEXY_REQUIRE_BRANCH_RULE(Item, "list() without a separator");
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
    LEXY_REQUIRE_BRANCH_RULE(Item, "list() with a trailing separator");
    return _lst<Item, _tsep<Sep>>{};
}

template <typename Item, typename Sep>
constexpr auto list(Item, _isep<Sep>)
{
    static_assert(lexy::_detail::error<Item, Sep>,
                  "list() does not support `dsl::ignore_trailing_sep()`");
    return _lst<Item, void>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Item, typename Sep, typename Recover>
struct _lstt : rule_base
{
    // We're using an enum together with a switch to compensate a lack of goto in constexpr.
    // The simple state machine goes as follows on well-formed input:
    // terminator -> separator -> separator_trailing_check -> item -> terminator -> ... ->
    // done
    //
    // The interesting case is error recovery.
    // There we skip over characters until we either found the terminator, separator or
    // item. We then set the enum to jump to the appropriate state of the state machine.
    enum class _state
    {
        terminator,
        separator,
        separator_trailing_check,
        item,
        recovery,
    };

    template <typename TermParser, typename Context, typename Reader, typename Sink>
    LEXY_PARSER_FUNC static bool _loop(_state initial_state, TermParser& term, Context& context,
                                       Reader& reader, Sink& sink)
    {
        auto state = initial_state;

        [[maybe_unused]] auto sep_pos = reader.position();
        while (true)
        {
            switch (state)
            {
            case _state::terminator:
                if (term.try_parse(context.control_block, reader))
                    // We had the terminator, so the list is done.
                    return true;
                term.cancel(context);

                // Parse the following list separator next.
                state = _state::separator;
                break;

            case _state::separator:
                if constexpr (!std::is_void_v<Sep>)
                {
                    sep_pos = reader.position();
                    if (lexy::parser_for<typename Sep::rule, lexy::sink_parser>::parse(context,
                                                                                       reader,
                                                                                       sink))
                    {
                        // Check for a trailing separator next.
                        state = _state::separator_trailing_check;
                        break;
                    }
                    else if (sep_pos == reader.position())
                    {
                        // We don't have a separator at all.
                        // Assume it's missing and parse an item instead.

                        if constexpr (lexy::is_branch_rule<Item>)
                        {
                            lexy::branch_parser_for<Item, Reader> item{};
                            if (item.try_parse(context.control_block, reader)
                                && item.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue after an item has been parsed.
                                state = _state::terminator;
                                break;
                            }
                            else
                            {
                                // Not an item, recover.
                                item.cancel(context);
                                state = _state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            // We cannot try and parse an item.
                            // To avoid generating wrong errors, immediately recover.
                            state = _state::recovery;
                            break;
                        }
                    }
                    else
                    {
                        // We did have something that looked like a separator initially, but
                        // wasn't one on closer inspection. Enter generic recovery as we've
                        // already consumed input. (If we ignore the case where the item and
                        // separator share a common prefix, we know it wasn't the start of an
                        // item so can't just pretend that there is one).
                        state = _state::recovery;
                        break;
                    }
                }
                else
                {
                    // List doesn't have a separator; immediately parse item next.
                    state = _state::item;
                    break;
                }

            case _state::separator_trailing_check:
                if constexpr (!std::is_void_v<Sep>)
                {
                    // We need to check whether we're having a trailing separator by checking
                    // for a terminating one.
                    if (term.try_parse(context.control_block, reader))
                    {
                        // We had the terminator, so the list is done.
                        // Report a trailing separator error if necessary.
                        Sep::report_trailing_error(context, reader, sep_pos, reader.position());
                        return true;
                    }
                    else
                    {
                        // We didn't have a separator, parse item next.
                        state = _state::item;
                        break;
                    }
                }
                break;

            case _state::item:
                if (lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                {
                    // Loop back.
                    state = _state::terminator;
                    break;
                }
                else
                {
                    // Recover from missing item.
                    state = _state::recovery;
                    break;
                }

            case _state::recovery: {
                auto recovery_begin = reader.position();
                context.on(_ev::recovery_start{}, recovery_begin);
                while (true)
                {
                    // Recovery succeeds when we reach the next separator.
                    if constexpr (!std::is_void_v<Sep>)
                    {
                        sep_pos = reader.position();

                        lexy::branch_parser_for<typename Sep::rule, Reader> sep{};
                        if (sep.try_parse(context.control_block, reader))
                        {
                            auto recovery_end = reader.position();
                            context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                       recovery_end);
                            context.on(_ev::recovery_finish{}, recovery_end);

                            if (sep.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue the list with the trailing separator check.
                                state = _state::separator_trailing_check;
                                break;
                            }
                            else
                            {
                                // Need to recover from this.
                                state = _state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            sep.cancel(context);
                        }
                    }
                    // When we don't have a separator, but the item is a branch, we also succeed
                    // when we reach the next item.
                    //
                    // Note that we're doing this check only if we don't have a separator.
                    // If we do have one, the heuristic "end of the invalid item" is better than
                    // "beginning of the next one".
                    else if constexpr (lexy::is_branch_rule<Item>)
                    {
                        lexy::branch_parser_for<Item, Reader> item{};
                        if (item.try_parse(context.control_block, reader))
                        {
                            auto recovery_end = reader.position();
                            context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                       recovery_end);
                            context.on(_ev::recovery_finish{}, recovery_end);

                            if (item.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue the list with the next terminator check.
                                state = _state::terminator;
                                break;
                            }
                            else
                            {
                                // Need to recover from this.
                                state = _state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            item.cancel(context);
                        }
                    }

                    // At this point, we couldn't detect the next item.
                    // Recovery succeeds when we reach the terminator.
                    if (term.try_parse(context.control_block, reader))
                    {
                        // We're now done with the entire list.
                        auto recovery_end = reader.position();
                        context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                   recovery_end);
                        context.on(_ev::recovery_finish{}, recovery_end);
                        return true;
                    }
                    else
                    {
                        term.cancel(context);
                    }

                    // At this point, we couldn't detect the next item or a terminator.
                    // Recovery fails when we reach the limit.
                    using limit_rule = decltype(Recover{}.get_limit());
                    if (lexy::token_parser_for<limit_rule, Reader> limit(reader);
                        limit.try_parse(reader) || reader.peek() == Reader::encoding::eof())
                    {
                        // Recovery has failed, propagate error.
                        auto recovery_end = reader.position();
                        context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                   recovery_end);
                        context.on(_ev::recovery_cancel{}, recovery_end);
                        return false;
                    }

                    // Consume one code unit and try again.
                    reader.bump();
                }
                break;
            }
            }
        }

        return false; // unreachable
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Term, Reader> term{};
            auto                                  sink = context.value_callback().sink();

            // Parse initial item.
            using item_parser = lexy::parser_for<Item, lexy::sink_parser>;
            auto result       = item_parser::parse(context, reader, sink);

            // Parse the remaining items.
            if (!_loop(result ? _state::terminator : _state::recovery, term, context, reader, sink))
                return false;

            // At this point, we just need to finish parsing the terminator.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return term.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return term.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                        LEXY_MOV(sink).finish());
            }
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED




// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_MEMBER_HPP_INCLUDED
#define LEXY_DSL_MEMBER_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
struct _mem : _copy_base<Rule>
{
    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Add member tag here.
            return rule.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                    lexy::member<Fn>{});
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but add member tag.
            using parser = lexy::parser_for<Rule, NextParser>;
            return parser::parse(context, reader, LEXY_FWD(args)..., lexy::member<Fn>{});
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




// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PARSE_AS_HPP_INCLUDED
#define LEXY_DSL_PARSE_AS_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_OBJECT_HPP_INCLUDED
#define LEXY_CALLBACK_OBJECT_HPP_INCLUDED



namespace lexy::_detail
{
template <typename T, typename... Args>
using _detect_brace_construct = decltype(T{LEXY_DECLVAL(Args)...});
template <typename T, typename... Args>
constexpr auto is_brace_constructible = _detail::is_detected<_detect_brace_construct, T, Args...>;

template <typename T, typename... Args>
constexpr auto is_constructible
    = std::is_constructible_v<T, Args...> || is_brace_constructible<T, Args...>;
} // namespace lexy::_detail

namespace lexy
{
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
    constexpr auto operator()(Args&&... args) const
        -> std::enable_if_t<_detail::is_constructible<T, Args&&...>, T>
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
            return T(LEXY_FWD(args)...);
        else
            return T{LEXY_FWD(args)...};
    }
};
template <>
struct _construct<void>
{
    using return_type = void;

    constexpr void operator()() const {}
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
    constexpr auto operator()(Args&&... args) const
        -> std::enable_if_t<_detail::is_constructible<T, Args&&...>, PtrT>
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

#endif // LEXY_CALLBACK_OBJECT_HPP_INCLUDED



namespace lexyd
{
// Custom handler that forwards events but overrides the value callback.
template <typename Handler>
struct _pas_handler
{
    Handler& _handler;

    using event_handler = typename Handler::event_handler;

    // We are implicitly convertible to all handler types the original handler is convertible to.
    // This is because the handler is passed to event_handler::on.
    template <typename H, typename = decltype(static_cast<H&>(LEXY_DECLVAL(Handler&)))>
    constexpr operator H&() const
    {
        return static_cast<H&>(_handler);
    }

    // We use ::value to get a value.
    // We can't use it unconditionally, as the initial production that contains the parse_as might
    // not have one. So we silently fallback if that's the case - this might cause worse errors if
    // the value is missing.
    template <typename Production, typename State>
    using value_callback
        = std::conditional_t<lexy::production_has_value_callback<Production, State>,
                             lexy::production_value_callback<Production, State>,
                             lexy::_detail::void_value_callback>;
};

struct _pas_final_parser
{
    template <typename Context, typename Reader, typename T, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context&, Reader&, lexy::_detail::lazy_init<T>& value,
                                       Args&&... args)
    {
        value.emplace_result(lexy::construct<T>, LEXY_FWD(args)...);
        return true;
    }
};

template <typename Handler>
constexpr auto _make_pas_handler(Handler& handler)
{
    return _pas_handler<Handler>{handler};
}
// Prevent infinite nesting when parse_as itself is recursive.
template <typename Handler>
constexpr auto _make_pas_handler(_pas_handler<Handler>& handler)
{
    return handler;
}

template <typename T, typename Rule, bool Front = false>
struct _pas : _copy_base<Rule>
{
    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule_parser;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule_parser.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // No need to use the special context here; it doesn't produce any values.
            rule_parser.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            auto handler = _make_pas_handler(context.control_block->parse_handler);
            lexy::_detail::parse_context_control_block cb(LEXY_MOV(handler), context.control_block);
            using context_type
                = lexy::_pc<decltype(handler), typename Context::state_type,
                            typename Context::production, typename Context::whitespace_production>;
            context_type sub_context(&cb);
            sub_context.handler = LEXY_MOV(context).handler;

            lexy::_detail::lazy_init<T> value;
            auto                        result
                = rule_parser.template finish<_pas_final_parser>(sub_context, reader, value);

            context.control_block->copy_vars_from(&cb);
            context.handler = LEXY_MOV(sub_context).handler;

            if (!result)
                return false;
            else if constexpr (std::is_void_v<T>)
                // NOLINTNEXTLINE: clang-tidy wrongly thinks the branch is repeated.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (Front)
                return NextParser::parse(context, reader, *LEXY_MOV(value), LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., *LEXY_MOV(value));
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto handler = _make_pas_handler(context.control_block->parse_handler);
            lexy::_detail::parse_context_control_block cb(LEXY_MOV(handler), context.control_block);
            using context_type
                = lexy::_pc<decltype(handler), typename Context::state_type,
                            typename Context::production, typename Context::whitespace_production>;
            context_type sub_context(&cb);
            sub_context.handler = LEXY_MOV(context).handler;

            lexy::_detail::lazy_init<T> value;
            auto                        result
                = lexy::parser_for<Rule, _pas_final_parser>::parse(sub_context, reader, value);

            context.control_block->copy_vars_from(&cb);
            context.handler = LEXY_MOV(sub_context).handler;

            if (!result)
                return false;
            else if constexpr (std::is_void_v<T>)
                // NOLINTNEXTLINE: clang-tidy wrongly thinks the branch is repeated.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (Front)
                return NextParser::parse(context, reader, *LEXY_MOV(value), LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., *LEXY_MOV(value));
        }
    };
};

template <typename T, typename Rule>
constexpr auto parse_as(Rule)
{
    return _pas<T, Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PARSE_AS_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
template <typename Rule, typename Tag>
struct _peek : branch_base
{
    template <typename Reader>
    struct bp
    {
        typename Reader::iterator begin;
        typename Reader::marker   end;

        constexpr bool try_parse(const void*, Reader reader)
        {
            // We need to match the entire rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            begin       = reader.position();
            auto result = parser.try_parse(reader);
            end         = parser.end;

            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end.position());
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::peek_failure>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end.position());
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            context.on(_ev::backtracked{}, impl.begin, impl.end.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peek<Rule, Error> error = {};
};

template <typename Rule, typename Tag>
struct _peekn : branch_base
{
    template <typename Reader>
    struct bp
    {
        typename Reader::iterator begin;
        typename Reader::marker   end;

        constexpr bool try_parse(const void*, Reader reader)
        {
            // We must not match the rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            begin       = reader.position();
            auto result = !parser.try_parse(reader);
            end         = parser.end;

            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end.position());
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::unexpected>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end.position());
                context.on(_ev::error{}, err);

                // And recover by consuming the input.
                context.on(_ev::recovery_start{}, impl.begin);
                context.on(_ev::token{}, lexy::error_token_kind, impl.begin, impl.end.position());
                context.on(_ev::recovery_finish{}, impl.end.position());

                reader.reset(impl.end);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                context.on(_ev::backtracked{}, impl.begin, impl.end.position());
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    template <typename Error>
    static constexpr _peekn<Rule, Error> error = {};
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek(Rule)
{
    return _peek<Rule, void>{};
}

/// Checks if at this reader position, the rule would not match.
template <typename Rule>
constexpr auto peek_not(Rule)
{
    return _peekn<Rule, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_POSITION_HPP_INCLUDED
#define LEXY_DSL_POSITION_HPP_INCLUDED



namespace lexyd
{
struct _pos : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto pos = reader.position();
            context.on(_ev::token{}, lexy::position_token_kind, pos, pos);
            return NextParser::parse(context, reader, LEXY_FWD(args)..., pos);
        }
    };
};

template <typename Rule>
struct _posr : _copy_base<Rule>
{
    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto pos = reader.position();
            context.on(_ev::token{}, lexy::position_token_kind, pos, pos);
            return rule.template finish<NextParser>(context, reader, LEXY_FWD(args)..., pos);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto pos = reader.position();
            context.on(_ev::token{}, lexy::position_token_kind, pos, pos);
            return lexy::parser_for<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...,
                                                             pos);
        }
    };
};

struct _pos_dsl : _pos
{
    template <typename Rule>
    constexpr auto operator()(Rule) const
    {
        return _posr<Rule>{};
    }
};

/// Produces an iterator to the current reader position without parsing anything.
constexpr auto position = _pos_dsl{};
} // namespace lexyd

#endif // LEXY_DSL_POSITION_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
template <typename Production, typename Context, typename Reader>
/* not force inline */ constexpr bool _parse_production(Context& context, Reader& reader)
{
    using parser = lexy::parser_for<lexy::production_rule<Production>, lexy::_detail::final_parser>;
    return parser::parse(context, reader);
}
template <typename ProductionParser, typename Context, typename Reader>
/* not force inline */ constexpr bool _finish_production(ProductionParser& parser, Context& context,
                                                         Reader& reader)
{
    return parser.template finish<lexy::_detail::final_parser>(context, reader);
}

template <typename Production>
struct _prd
// If the production defines whitespace, it can't be a branch production.
: std::conditional_t<lexy::_production_defines_whitespace<Production>, rule_base,
                     _copy_base<lexy::production_rule<Production>>>
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Create a context for the production and parse the context there.
            auto sub_context = context.sub_context(Production{});
            sub_context.on(_ev::production_start{}, reader.position());

            // Skip initial whitespace if the rule changed.
            if constexpr (lexy::_production_defines_whitespace<Production>)
            {
                if (!lexy::whitespace_parser<decltype(sub_context),
                                             lexy::pattern_parser<>>::parse(sub_context, reader))
                {
                    sub_context.on(_ev::production_cancel{}, reader.position());
                    return false;
                }
            }

            if (_parse_production<Production>(sub_context, reader))
            {
                sub_context.on(_ev::production_finish{}, reader.position());

                using continuation = lexy::_detail::context_finish_parser<NextParser>;
                return continuation::parse(context, reader, sub_context, LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                sub_context.on(_ev::production_cancel{}, reader.position());
                return false;
            }
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::production_branch_parser<Production, Reader> parser;
        typename Reader::iterator                          begin;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            begin = reader.position();
            return parser.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // Cancel in a new context.
            auto sub_context = context.sub_context(Production{});
            sub_context.on(_ev::production_start{}, begin);
            parser.cancel(sub_context);
            sub_context.on(_ev::production_cancel{}, begin);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(!lexy::_production_defines_whitespace<Production>);

            // Finish the production in a new context.
            auto sub_context = context.sub_context(Production{});
            sub_context.on(_ev::production_start{}, begin);
            if (_finish_production(parser, sub_context, reader))
            {
                sub_context.on(_ev::production_finish{}, reader.position());

                using continuation = lexy::_detail::context_finish_parser<NextParser>;
                return continuation::parse(context, reader, sub_context, LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                sub_context.on(_ev::production_cancel{}, reader.position());
                return false;
            }
        }
    };
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};
} // namespace lexyd

namespace lexy
{
struct max_recursion_depth_exceeded
{
    static LEXY_CONSTEVAL auto name()
    {
        return "maximum recursion depth exceeded";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Production, typename DepthError = void>
struct _recb : branch_base
{
    template <typename NextParser>
    struct _depth_handler
    {
        template <typename Context, typename Reader>
        static constexpr bool increment_depth(Context& context, Reader& reader)
        {
            auto control_block = context.control_block;
            LEXY_ASSERT(control_block->max_depth > 0,
                        "dsl::recurse_branch<P> is disabled in this context");

            // We're doing a recursive call, check for an exceeded depth.
            if (control_block->cur_depth >= control_block->max_depth)
            {
                // We did report error from which we can't recover.
                using tag = lexy::_detail::type_or<DepthError, lexy::max_recursion_depth_exceeded>;
                auto err  = lexy::error<Reader, tag>(reader.position());
                context.on(_ev::error{}, err);
                return false;
            }

            ++control_block->cur_depth;
            return true;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto control_block = context.control_block;
            --control_block->cur_depth;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        LEXY_REQUIRE_BRANCH_RULE(lexy::production_rule<Production>, "recurse_branch");

        using impl = lexy::branch_parser_for<_prd<Production>, Reader>;
        impl _impl;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return _impl.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            _impl.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            using depth = _depth_handler<NextParser>;
            if (!depth::increment_depth(context, reader))
                return false;
            return _impl.template finish<depth>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using depth = _depth_handler<NextParser>;
            if (!depth::increment_depth(context, reader))
                return false;

            return lexy::parser_for<_prd<Production>, depth>::parse(context, reader,
                                                                    LEXY_FWD(args)...);
        }
    };

    template <typename Tag>
    static constexpr _recb<Production, Tag> max_depth_error = _recb<Production, Tag>{};
};

template <typename Production, typename DepthError = void>
struct _rec : rule_base
{
    template <typename NextParser>
    using p = lexy::parser_for<_recb<Production, DepthError>, NextParser>;

    template <typename Tag>
    static constexpr _rec<Production, Tag> max_depth_error = _rec<Production, Tag>{};
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};

template <typename Production>
constexpr auto recurse_branch = _recb<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PUNCTUATOR_HPP_INCLUDED
#define LEXY_DSL_PUNCTUATOR_HPP_INCLUDED




namespace lexyd
{
#define LEXY_PUNCT(Name, String)                                                                   \
    struct _##Name : LEXY_NTTP_STRING(_lit, String)                                                \
    {};                                                                                            \
    inline constexpr auto(Name) = _##Name {}

LEXY_PUNCT(period, ".");
LEXY_PUNCT(comma, ",");
LEXY_PUNCT(colon, ":");
LEXY_PUNCT(double_colon, "::");
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
LEXY_PUNCT(vbar, "|");

LEXY_PUNCT(hash_sign, "#");
LEXY_PUNCT(dollar_sign, "$");
LEXY_PUNCT(at_sign, "@");
LEXY_PUNCT(percent_sign, "%");
LEXY_PUNCT(equal_sign, "=");

#undef LEXY_PUNCT
} // namespace lexyd

#endif // LEXY_DSL_PUNCTUATOR_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_REPEAT_HPP_INCLUDED
#define LEXY_DSL_REPEAT_HPP_INCLUDED






namespace lexyd
{
template <typename Item, typename Sep>
struct _rep_impl
{
    // Sink can either be present or not;
    // we only use variadic arguments to get rid of code duplication.
    template <typename Context, typename Reader, typename... Sink>
    static constexpr bool loop(Context& context, Reader& reader, std::size_t count, Sink&... sink)
    {
        using sink_parser
            = std::conditional_t<sizeof...(Sink) == 0, lexy::pattern_parser<>, lexy::sink_parser>;

        if (count == 0)
            return true;

        using item_parser = lexy::parser_for<Item, sink_parser>;
        if (!item_parser::parse(context, reader, sink...))
            return false;

        for (std::size_t i = 1; i != count; ++i)
        {
            using sep_parser = lexy::parser_for<typename Sep::rule, sink_parser>;
            if (!sep_parser::parse(context, reader, sink...))
                return false;

            if (!item_parser::parse(context, reader, sink...))
                return false;
        }

        using trailing_parser = lexy::parser_for<typename Sep::trailing_rule, sink_parser>;
        return trailing_parser::parse(context, reader, sink...);
    }
};
template <typename Item>
struct _rep_impl<Item, void>
{
    // Sink can either be present or not;
    // we only use variadic arguments to get rid of code duplication.
    template <typename Context, typename Reader, typename... Sink>
    static constexpr bool loop(Context& context, Reader& reader, std::size_t count, Sink&... sink)
    {
        using sink_parser
            = std::conditional_t<sizeof...(Sink) == 0, lexy::pattern_parser<>, lexy::sink_parser>;

        if (count == 0)
            return true;

        using item_parser = lexy::parser_for<Item, sink_parser>;
        if (!item_parser::parse(context, reader, sink...))
            return false;

        for (std::size_t i = 1; i != count; ++i)
        {
            if (!item_parser::parse(context, reader, sink...))
                return false;
        }

        return true;
    }
};

template <typename Item, typename Sep>
struct _repd : rule_base // repeat, discard
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, std::size_t count,
                                           Args&&... args)
        {
            if (!_rep_impl<Item, Sep>::loop(context, reader, count))
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep>
struct _repl : rule_base // repeat, list
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, std::size_t count,
                                           Args&&... args)
        {
            auto sink = context.value_callback().sink();
            if (!_rep_impl<Item, Sep>::loop(context, reader, count, sink))
                return false;

            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep>
struct _repc : rule_base // repeat, capture
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, std::size_t count,
                                           Args&&... args)
        {
            auto begin = reader.position();
            if (!_rep_impl<Item, Sep>::loop(context, reader, count))
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                     lexy::lexeme(reader, begin));
        }
    };
};

template <typename Count, typename Loop>
struct _rep : decltype(_maybe_branch(_pas<std::size_t, Count, true>{}, Loop{})){};

template <typename Count>
struct _rep_dsl
{
    template <typename Item>
    constexpr auto operator()(Item) const
    {
        return _rep<Count, _repd<Item, void>>{};
    }
    template <typename Item, typename Sep>
    constexpr auto operator()(Item, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _rep<Count, _repd<Item, Sep>>{};
    }

    template <typename Item>
    constexpr auto list(Item) const
    {
        return _rep<Count, _repl<Item, void>>{};
    }
    template <typename Item, typename Sep>
    constexpr auto list(Item, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _rep<Count, _repl<Item, Sep>>{};
    }

    template <typename Item>
    constexpr auto capture(Item) const
    {
        return _rep<Count, _repc<Item, void>>{};
    }
    template <typename Item, typename Sep>
    constexpr auto capture(Item, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _rep<Count, _repc<Item, Sep>>{};
    }
};

/// Parses a rule `n` times, where `n` is the value produced by `Count`.
template <typename Count>
constexpr auto repeat(Count)
{
    return _rep_dsl<Count>{};
}
} // namespace lexyd

#endif // LEXY_DSL_REPEAT_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_RETURN_HPP_INCLUDED
#define LEXY_DSL_RETURN_HPP_INCLUDED




namespace lexyd
{
struct _ret : rule_base
{
    // We unconditionally jump to the final parser.
    template <typename NextParser>
    using p = lexy::_detail::final_parser;
};

/// Finishes parsing a production without considering subsequent rules.
constexpr auto return_ = _ret{};
} // namespace lexyd

#endif // LEXY_DSL_RETURN_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SCAN_HPP_INCLUDED
#define LEXY_DSL_SCAN_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_FORWARD_HPP_INCLUDED
#define LEXY_CALLBACK_FORWARD_HPP_INCLUDED



namespace lexy
{
struct nullopt;

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
template <>
struct _fwd<void>
{
    using return_type = void;

    template <typename... Args>
    constexpr auto sink(const Args&...) const
    {
        // We don't need a separate type, forward itself can have the required functions.
        return *this;
    }

    constexpr void operator()() const {}
    constexpr void operator()(const lexy::nullopt&) const {}

    constexpr void finish() && {}
};

/// A callback that just forwards an existing object.
template <typename T>
constexpr auto forward = _fwd<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_FORWARD_HPP_INCLUDED






//=== rule forward declaration ===//
namespace lexyd
{
template <typename Production>
struct _prd;
template <typename Rule, typename Tag>
struct _peek;
template <typename Token>
struct _cap;
template <typename Rule>
struct _capr;
template <typename T, typename Base>
struct _int_dsl;

struct _scan;
} // namespace lexyd

namespace lexy::_detail
{
template <typename Derived, typename Reader>
class scanner;
}

//=== scan_result ===//
namespace lexy
{
constexpr struct scan_failed_t
{
} scan_failed;

template <typename T>
class scan_result
{
public:
    using value_type = T;

    constexpr scan_result() = default;
    constexpr scan_result(scan_failed_t) {}

    template <typename U = T, typename = std::enable_if_t<std::is_constructible_v<T, U>>>
    constexpr scan_result(U&& value)
    {
        _value.emplace(LEXY_MOV(value));
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

    constexpr decltype(auto) value() const& noexcept
    {
        return *_value;
    }
    constexpr decltype(auto) value() && noexcept
    {
        return LEXY_MOV(*_value);
    }

    template <typename U = T>
    constexpr decltype(auto) value_or(U&& fallback) const& noexcept
    {
        return _value ? *_value : LEXY_FWD(fallback);
    }
    template <typename U = T>
    constexpr decltype(auto) value_or(U&& fallback) && noexcept
    {
        return _value ? LEXY_MOV(*_value) : LEXY_FWD(fallback);
    }

private:
    constexpr explicit scan_result(_detail::lazy_init<T>&& value) : _value(LEXY_MOV(value)) {}

    _detail::lazy_init<T> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};
template <>
class scan_result<void>
{
public:
    using value_type = void;

    constexpr scan_result() = default;
    constexpr scan_result(scan_failed_t) {}
    constexpr scan_result(bool has_value)
    {
        if (has_value)
            _value.emplace();
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

private:
    constexpr explicit scan_result(_detail::lazy_init<void>&& value) : _value(LEXY_MOV(value)) {}

    _detail::lazy_init<void> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};

template <typename T>
scan_result(T&&) -> scan_result<std::decay_t<T>>;
template <typename T>
scan_result(_detail::lazy_init<T>&&) -> scan_result<T>;
} // namespace lexy

//=== scanner implementation ===//
namespace lexy::_detail
{
template <typename Reader>
struct scanner_input
{
    Reader _impl;

    constexpr auto reader() const&
    {
        return _impl;
    }
};

struct scan_final_parser
{
    template <typename Context, typename Reader, typename T>
    LEXY_PARSER_FUNC static bool parse(Context&, Reader&, lazy_init<T>* dest, T&& value)
    {
        dest->emplace(LEXY_MOV(value));
        return true;
    }

    template <typename Context, typename Reader>
    LEXY_PARSER_FUNC static bool parse(Context&, Reader&, lazy_init<void>* dest)
    {
        dest->emplace();
        return true;
    }
};

// The common interface of all scanner types.
template <typename Derived, typename Reader>
class scanner
{
public:
    using encoding = typename Reader::encoding;

    constexpr scanner(const scanner&) noexcept            = delete;
    constexpr scanner& operator=(const scanner&) noexcept = delete;

    //=== status ===//
    constexpr explicit operator bool() const noexcept
    {
        return _state == _state_normal;
    }

    constexpr bool is_at_eof() const
    {
        return _reader.peek() == Reader::encoding::eof();
    }

    constexpr auto position() const noexcept -> typename Reader::iterator
    {
        return _reader.position();
    }
    constexpr auto current() const noexcept -> typename Reader::marker
    {
        return _reader.current();
    }

    constexpr auto remaining_input() const noexcept
    {
        return scanner_input<Reader>{_reader};
    }

    //=== parsing ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr void parse(scan_result<T>& result, Rule)
    {
        if (_state == _state_failed)
            return;

        using parser = lexy::parser_for<lexyd::_pas<T, Rule>, scan_final_parser>;
        auto success
            = parser::parse(static_cast<Derived&>(*this).context(), _reader, &result._value);
        if (!success)
            _state = _state_failed;
    }

    template <typename Production, typename = lexy::production_rule<Production>>
    constexpr auto parse(Production = {})
    {
        using context_t = LEXY_DECAY_DECLTYPE(static_cast<Derived&>(*this).context());
        using value_type =
            typename lexy::production_value_callback<Production,
                                                     typename context_t::state_type>::return_type;

        scan_result<value_type> result;
        parse(result, lexyd::_prd<Production>{});
        return result;
    }

    template <typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr void parse(Rule rule)
    {
        scan_result<void> result;
        parse(result, rule);
    }

    //=== branch parsing ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr bool branch(scan_result<T>& result, Rule)
    {
        LEXY_REQUIRE_BRANCH_RULE(Rule, "branch");
        if (_state == _state_failed)
            return false;

        auto& context = static_cast<Derived&>(*this).context();
        lexy::branch_parser_for<lexyd::_pas<T, Rule>, Reader> parser{};
        if (!parser.try_parse(context.control_block, _reader))
        {
            parser.cancel(context);
            return false; // branch wasn't token
        }

        auto success = parser.template finish<lexy::_detail::scan_final_parser>(context, _reader,
                                                                                &result._value);
        if (!success)
            _state = _state_failed;
        return true; // branch was taken
    }

    template <typename Production, typename T, typename = lexy::production_rule<Production>>
    constexpr bool branch(scan_result<T>& result, Production = {})
    {
        return branch(result, lexyd::_prd<Production>{});
    }

    template <typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr bool branch(Rule rule)
    {
        scan_result<void> result;
        return branch(result, rule);
    }

    //=== error handling ===//
    class error_recovery_guard
    {
    public:
        error_recovery_guard(const error_recovery_guard&)            = delete;
        error_recovery_guard& operator=(const error_recovery_guard&) = delete;

        constexpr void cancel() &&
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_cancel{}, _self->_reader.position());
            _self->_state = _state_failed;
        }

        constexpr void finish() &&
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_finish{}, _self->_reader.position());
            _self->_state = _state_normal;
        }

    private:
        constexpr explicit error_recovery_guard(scanner& self) noexcept : _self(&self)
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_start{}, _self->_reader.position());
            _self->_state = _state_recovery;
        }

        scanner* _self;
        friend scanner;
    };

    constexpr auto error_recovery()
    {
        LEXY_PRECONDITION(_state == _state_failed);
        return error_recovery_guard(*this);
    }

    template <typename TokenRule>
    constexpr bool discard(TokenRule rule)
    {
        static_assert(lexy::is_token_rule<TokenRule>);
        if (_state == _state_failed)
            return false;

        auto begin  = _reader.position();
        auto result = lexy::try_match_token(rule, _reader);
        auto end    = _reader.position();

        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::token{}, lexy::error_token_kind, begin, end);

        return result;
    }

    template <typename Tag, typename... Args>
    constexpr void error(Tag, Args&&... args)
    {
        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::error{}, lexy::error<Reader, Tag>(LEXY_FWD(args)...));
    }

    template <typename... Args>
    constexpr void error(const char* msg, Args&&... args)
    {
        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::error{}, lexy::error<Reader, void>(LEXY_FWD(args)..., msg));
    }

    template <typename Tag, typename... Args>
    constexpr void fatal_error(Tag tag, Args&&... args)
    {
        error(tag, LEXY_FWD(args)...);
        _state = _state_failed;
    }

    template <typename... Args>
    constexpr void fatal_error(const char* msg, Args&&... args)
    {
        error(msg, LEXY_FWD(args)...);
        _state = _state_failed;
    }

    //=== convenience ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr auto parse(Rule rule)
    {
        scan_result<T> result;
        parse(result, rule);
        return result;
    }

    template <typename Rule>
    constexpr bool peek(Rule)
    {
        static_assert(lexy::is_rule<Rule>);
        return branch(dsl::_peek<Rule, void>{});
    }

    template <typename T, typename Base, typename Digits>
    constexpr auto integer(Digits digits)
    {
        scan_result<T> result;
        parse(result, lexyd::_int_dsl<T, Base>{}(digits));
        return result;
    }
    template <typename T, typename Digits>
    constexpr auto integer(Digits digits)
    {
        scan_result<T> result;
        parse(result, lexyd::_int_dsl<T, void>{}(digits));
        return result;
    }

    template <typename Token>
    constexpr auto capture(Token)
    {
        scan_result<lexeme<Reader>> result;
        parse(result, lexyd::_cap<Token>{});
        return result;
    }
    template <typename Production>
    constexpr auto capture(lexyd::_prd<Production>)
    {
        scan_result<lexeme<Reader>> result;
        parse(result, lexyd::_capr<lexyd::_prd<Production>>{});
        return result;
    }

protected:
    constexpr explicit scanner(const Reader& reader) noexcept
    : _reader(reader), _state(_state_normal)
    {}

    constexpr Reader& reader() noexcept
    {
        return _reader;
    }

private:
    Reader _reader;

    enum
    {
        _state_normal,
        _state_failed,   // after a failure
        _state_recovery, // recovery guard active
    } _state;
};
} // namespace lexy::_detail

//=== dsl::scan ===//
namespace lexy
{
template <typename Context, typename Reader>
class rule_scanner : public _detail::scanner<rule_scanner<Context, Reader>, Reader>
{
public:
    using production = typename Context::production;

    constexpr std::size_t recursion_depth() const noexcept
    {
        auto& cb = _context->control_block();
        return static_cast<std::size_t>(cb.cur_depth);
    }

    constexpr auto begin() const noexcept -> typename Reader::iterator
    {
        return _begin;
    }

private:
    constexpr explicit rule_scanner(Context& context, Reader reader)
    : _detail::scanner<rule_scanner<Context, Reader>, Reader>(reader), _context(&context),
      _begin(reader.position())
    {}

    constexpr Context& context() noexcept
    {
        return *_context;
    }

    Context*                  _context;
    typename Reader::iterator _begin;

    friend _detail::scanner<rule_scanner<Context, Reader>, Reader>;
    friend lexyd::_scan;
};
} // namespace lexy

namespace lexyd
{
template <typename Context, typename Scanner, typename StatePtr, typename... Args>
using _detect_scan_state = decltype(Context::production::scan(LEXY_DECLVAL(Scanner&), *StatePtr(),
                                                              LEXY_DECLVAL(Args)...));

struct _scan : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Scanner, typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool _parse(Scanner& scanner, Context& context, Reader& reader,
                                            Args&&... args)
        {
            typename Context::production::scan_result result = [&] {
                if constexpr (lexy::_detail::is_detected<
                                  _detect_scan_state, Context, decltype(scanner),
                                  decltype(context.control_block->parse_state), Args&&...>)
                    return Context::production::scan(scanner, *context.control_block->parse_state,
                                                     LEXY_FWD(args)...);
                else
                    return Context::production::scan(scanner, LEXY_FWD(args)...);
            }();
            reader.reset(scanner.current());
            if (!result)
                return false;

            if constexpr (std::is_void_v<typename decltype(result)::value_type>)
                return NextParser::parse(context, reader);
            else
                return NextParser::parse(context, reader, LEXY_MOV(result).value());
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::rule_scanner scanner(context, reader);
            return _parse(scanner, context, reader, LEXY_FWD(args)...);
        }
    };
};

inline constexpr auto scan = _scan{};
} // namespace lexyd

namespace lexy
{
template <typename T>
struct scan_production
{
    using scan_result = lexy::scan_result<T>;

    static constexpr auto rule  = dsl::scan;
    static constexpr auto value = lexy::forward<T>;
};
} // namespace lexy

#endif // LEXY_DSL_SCAN_HPP_INCLUDED



// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
struct _plus : decltype(op<lexy::plus_sign>(LEXY_LIT("+")))
{};
struct _minus : decltype(op<lexy::minus_sign>(LEXY_LIT("-")))
{};

/// Matches a plus sign or nothing, producing +1.
constexpr auto plus_sign = if_(_plus{});
/// Matches a minus sign or nothing, producing +1 or -1.
constexpr auto minus_sign = if_(_minus{});

/// Matches a plus or minus sign or nothing, producing +1 or -1.
constexpr auto sign = if_(_plus{} | _minus{});
} // namespace lexyd

#endif // LEXY_DSL_SIGN_HPP_INCLUDED

// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SUBGRAMMAR_HPP_INCLUDED
#define LEXY_DSL_SUBGRAMMAR_HPP_INCLUDED




namespace lexy
{
template <typename Production, typename Handler, typename State, typename Reader>
struct _subgrammar;

template <typename Production, typename Action>
using _subgrammar_for = _subgrammar<Production, typename Action::handler, typename Action::state,
                                    lexy::input_reader<typename Action::input>>;
} // namespace lexy

#define LEXY_DECLARE_SUBGRAMMAR(Production)                                                        \
    namespace lexy                                                                                 \
    {                                                                                              \
        template <typename ParseState>                                                             \
        constexpr auto production_has_value_callback<Production, ParseState> = true;               \
                                                                                                   \
        template <typename Handler, typename State, typename Reader>                               \
        struct _subgrammar<Production, Handler, State, Reader>                                     \
        {                                                                                          \
            template <typename T>                                                                  \
            static bool parse(_detail::lazy_init<T>&                                value,         \
                              _detail::parse_context_control_block<Handler, State>* control_block, \
                              Reader&                                               reader);                                                     \
        };                                                                                         \
    }

#define LEXY_DEFINE_SUBGRAMMAR(Production)                                                         \
    template <typename Handler, typename State, typename Reader>                                   \
    template <typename T>                                                                          \
    bool ::lexy::_subgrammar<Production, Handler, State, Reader>::                                 \
        parse(::lexy::_detail::lazy_init<T>&                                value,                 \
              ::lexy::_detail::parse_context_control_block<Handler, State>* control_block,         \
              Reader&                                                       reader)                \
    {                                                                                              \
        lexy::_pc<Handler, State, Production> context(control_block);                              \
        auto                                  success = ::lexy::_do_action(context, reader);       \
        value                                         = LEXY_MOV(context.value);                   \
        return success;                                                                            \
    }

#define LEXY_INSTANTIATE_SUBGRAMMAR(Production, ...)                                               \
    template bool ::lexy::_subgrammar_for<Production, __VA_ARGS__> /**/                            \
        ::parse<::lexy::_production_value_type<typename __VA_ARGS__::handler,                      \
                                               typename __VA_ARGS__::state, Production>> /**/      \
        (::lexy::_detail::lazy_init<::lexy::_production_value_type<                                \
             typename __VA_ARGS__::handler, typename __VA_ARGS__::state, Production>>&,            \
         ::lexy::_detail::parse_context_control_block<typename __VA_ARGS__::handler,               \
                                                      typename __VA_ARGS__::state>*,               \
         ::lexy::input_reader<typename __VA_ARGS__::input>&);

namespace lexyd
{
template <typename Production, typename T>
struct _subg : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using control_block_type = LEXY_DECAY_DECLTYPE(*context.control_block);
            using handler_type       = typename control_block_type::handler_type;
            using state_type         = typename control_block_type::state_type;

            auto vars                   = context.control_block->vars;
            context.control_block->vars = nullptr;

            constexpr auto production_uses_void_callback = std::is_same_v<
                typename handler_type::template value_callback<Production, state_type>,
                lexy::_detail::void_value_callback>;
            using value_type = std::conditional_t<production_uses_void_callback, void, T>;
            lexy::_detail::lazy_init<value_type> value;

            using subgrammar_traits
                = lexy::_subgrammar<Production, handler_type, state_type, Reader>;
            auto rule_result
                = subgrammar_traits::template parse<value_type>(value, context.control_block,
                                                                reader);

            context.control_block->vars = vars;

            if (!rule_result)
                return false;

            if constexpr (std::is_void_v<value_type>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., *LEXY_MOV(value));
        }
    };
};

/// Parses the entry production of a subgrammar, which may be defined in a different
/// file.
template <typename Production, typename T>
constexpr auto subgrammar = _subg<Production, T>{};
} // namespace lexyd

#endif // LEXY_DSL_SUBGRAMMAR_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_TIMES_HPP_INCLUDED
#define LEXY_DSL_TIMES_HPP_INCLUDED




namespace lexyd
{
template <std::size_t N, typename Rule, typename Sep>
struct _times : rule_base
{
    template <std::size_t I = N>
    static constexpr auto _repeated_rule()
    {
        if constexpr (I == 1)
        {
            if constexpr (std::is_same_v<Sep, void>)
                return Rule{};
            else
                return Rule{} + typename Sep::trailing_rule{};
        }
        else
        {
            if constexpr (std::is_same_v<Sep, void>)
                return Rule{} + _repeated_rule<I - 1>();
            else
                return Rule{} + typename Sep::rule{} + _repeated_rule<I - 1>();
        }
    }

    template <typename NextParser>
    using p = lexy::parser_for<decltype(_repeated_rule()), NextParser>;
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
    static_assert(lexy::is_separator<Sep>);
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


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_UNICODE_HPP_INCLUDED
#define LEXY_DSL_UNICODE_HPP_INCLUDED






namespace lexyd::unicode
{
struct _nctrl : char_class_base<_nctrl>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.non-control";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_print::char_class_ascii();
    }

    static constexpr bool char_class_match_cp(char32_t cp)
    {
        return !lexy::code_point(cp).is_control();
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_print::template char_class_match_swar<Encoding>(c);
    }
};

struct _control : char_class_base<_control>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.control";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_control::char_class_ascii();
    }

    static constexpr bool char_class_match_cp(char32_t cp)
    {
        return lexy::code_point(cp).is_control();
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_control::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto control = _control{};

constexpr auto operator-(_control)
{
    return _nctrl{};
}
constexpr auto operator-(_nctrl)
{
    return _control{};
}

//=== whitespace ===//
struct _blank : char_class_base<_blank>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.blank";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_blank::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // tab already handled as part of ASCII
        return lexy::code_point(cp).general_category() == lexy::code_point::space_separator;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_blank::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto blank = _blank{};

struct _newline : char_class_base<_newline>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.newline";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_newline::char_class_ascii();
    }

    static constexpr bool char_class_match_cp(char32_t cp)
    {
        // NEL, PARAGRAPH SEPARATOR, LINE SEPARATOR
        return cp == 0x85 || cp == 0x2029 || cp == 0x2028;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_newline::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto newline = _newline{};

struct _other_space : char_class_base<_other_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.other-space";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_other_space::char_class_ascii();
    }

    // The same as in ASCII, so no match function needed.

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_other_space::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto other_space = _other_space{};

struct _space : char_class_base<_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.whitespace";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_space::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(whitespace)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_space::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : char_class_base<_lower>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.lowercase";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_lower::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(lowercase)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_lower::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto lower = _lower{};

struct _upper : char_class_base<_upper>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.uppercase";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_upper::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(uppercase)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_upper::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto upper = _upper{};

struct _alpha : char_class_base<_alpha>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.alphabetic";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alpha::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(alphabetic)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_alpha::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto alpha = _alpha{};

//=== digit ===//
struct _digit : char_class_base<_digit>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.decimal-number";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_digit::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::code_point(cp).general_category() == lexy::code_point::decimal_number;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_digit::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto digit = _digit{};

struct _alnum : char_class_base<_alnum>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.alphabetic-decimal";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alnum::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(alphabetic)>(cp)
               || lexy::code_point(cp).general_category() == lexy::code_point::decimal_number;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_alnum::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = alnum;

struct _word : char_class_base<_word>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.word";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_word::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        auto cat = lexy::code_point(cp).general_category();
        if (cat == lexy::code_point::mark || cat == lexy::code_point::connector_punctuation
            || cat == lexy::code_point::decimal_number)
            return true;

        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(alphabetic),
                                                        LEXY_UNICODE_PROPERTY(join_control)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_word::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto word = _word{};

//=== categories ===//
struct _graph : char_class_base<_graph>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.graph";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_graph::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t _cp)
    {
        auto cp = lexy::code_point(_cp);

        // Everything that isn't control, surrogate, unassigned, or space.
        return !cp.is_control() && !cp.is_surrogate()
               && cp.general_category() != lexy::code_point::unassigned
               && !_space::char_class_match_cp(cp.value());
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_graph::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto graph = _graph{};

struct _print : char_class_base<_print>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.print";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_print::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // blank or graph without control.
        return !_control::char_class_match_cp(cp)
               && (_blank::char_class_match_cp(cp) || _graph::char_class_match_cp(cp));
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_print::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto print = _print{};

struct _char : char_class_base<_char>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.character";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_char::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::code_point(cp).general_category() != lexy::code_point::unassigned;
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_char::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto character = _char{};
} // namespace lexyd::unicode

namespace lexyd::unicode
{
struct _xid_start : char_class_base<_xid_start>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.XID-start";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alpha::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(xid_start)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_alpha::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto xid_start = _xid_start{};

struct _xid_start_underscore : char_class_base<_xid_start_underscore>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.XID-start-underscore";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alphau::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // underscore handled as part of ASCII.
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(xid_start)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_alphau::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto xid_start_underscore = _xid_start_underscore{};

struct _xid_continue : char_class_base<_xid_continue>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.XID-continue";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_word::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // underscore handled as part of ASCII.
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(xid_continue)>(cp);
    }

    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c)
    {
        return ascii::_word::template char_class_match_swar<Encoding>(c);
    }
};
inline constexpr auto xid_continue = _xid_continue{};
} // namespace lexyd::unicode

#endif // LEXY_DSL_UNICODE_HPP_INCLUDED

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED





namespace lexyd
{
struct _nl;

template <typename Condition, typename Reader>
constexpr void _until_swar([[maybe_unused]] Reader& reader)
{
    if constexpr (std::is_same_v<Condition, _nl> //
                  && lexy::_detail::is_swar_reader<Reader>)
    {
        // We use SWAR to skip characters until we have one that is <= 0xF or EOF.
        // Then we need to inspect it in more detail.
        using char_type = typename Reader::encoding::char_type;

        while (true)
        {
            auto cur = reader.peek_swar();
            if (lexy::_detail::swar_has_char<char_type, Reader::encoding::eof()>(cur)
                || lexy::_detail::swar_has_char_less<char_type, 0xF>(cur))
                break;
            reader.bump_swar();
        }
    }
}

template <typename Condition>
struct _until_eof : token_base<_until_eof<Condition>, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            while (true)
            {
                _until_swar<Condition>(reader);

                // Check whether we've reached the end of the input or the condition.
                // Note that we're checking for EOF before the condition.
                // This is a potential optimization: as we're accepting EOF anyway, we don't need to
                // enter Condition's parsing logic.
                if (reader.peek() == Reader::encoding::eof()
                    || lexy::try_match_token(Condition{}, reader))
                {
                    // It did, so we're done.
                    break;
                }

                // It did not match, consume one code unit and try again.
                reader.bump();
            }

            end = reader.current();
            return {};
        }
    };
};

template <typename Condition>
struct _until : token_base<_until<Condition>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            while (true)
            {
                _until_swar<Condition>(reader);

                // Try to parse the condition.
                if (lexy::try_match_token(Condition{}, reader))
                {
                    // It did match, we're done at that end.
                    end = reader.current();
                    return true;
                }

                // Check whether we've reached the end of the input.
                // We need to do it after checking for condition, as the condition might just
                // accept EOF.
                if (reader.peek() == Reader::encoding::eof())
                {
                    // It did, so we did not succeed.
                    end = reader.current();
                    return false;
                }

                // It did not match, consume one code unit and try again.
                reader.bump();
            }

            return false; // unreachable
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            // We need to trigger the error `Condition` would.
            // As such, we try parsing it, which will report an error.

            reader.reset(end);
            LEXY_ASSERT(reader.peek() == Reader::encoding::eof(),
                        "forgot to set end in try_parse()");

            lexy::token_parser_for<Condition, Reader> parser(reader);
            auto                                      result = parser.try_parse(reader);
            LEXY_ASSERT(!result, "condition shouldn't have matched?!");
            parser.report_error(context, reader);
        }
    };

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

namespace lexy
{
template <typename Condition>
constexpr auto token_kind_of<lexy::dsl::_until_eof<Condition>> = lexy::any_token_kind;
template <typename Condition>
constexpr auto token_kind_of<lexy::dsl::_until<Condition>> = lexy::any_token_kind;
} // namespace lexy

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED



#if LEXY_EXPERIMENTAL
// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PARSE_TREE_NODE_HPP_INCLUDED
#define LEXY_DSL_PARSE_TREE_NODE_HPP_INCLUDED




#if !LEXY_EXPERIMENTAL
#    error "lexy::dsl::tnode/pnode are experimental"
#endif

//=== impl ===//
namespace lexyd
{
template <typename Derived>
struct _n;

template <typename Derived, typename R>
struct _nr : branch_base
{
    template <typename NextParser>
    struct _cont
    {
        template <typename Context, typename ChildReader, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, ChildReader& child_reader,
                                           bool& rule_succeded, Reader& reader, Args&&... args)
        {
            rule_succeded = true;

            if (child_reader.peek() != ChildReader::encoding::eof())
            {
                auto begin = child_reader.position();
                auto end   = reader.position();
                context.on(_ev::token{}, lexy::error_token_kind, begin, end);

                auto err = lexy::error<Reader, typename Derived::node_end_error>(begin, end);
                context.on(_ev::error{}, err);
            }

            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename NextParser, typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool _parse_rule(Context& context, Reader& reader,
                                             typename Reader::marker end, Args&&... args)
    {
        auto child_reader = Derived::node_child_reader(reader);
        reader.reset(end);

        using rule_parser
            = lexy::whitespace_parser<Context, lexy::parser_for<R, _cont<NextParser>>>;
        if (auto rule_succeded = false;
            rule_parser::parse(context, child_reader, rule_succeded, reader, LEXY_FWD(args)...))
        {
            return true;
        }
        else
        {
            if (!rule_succeded)
                // Report an error token for the child span that wasn't able to be parsed.
                context.on(_ev::token{}, lexy::error_token_kind, child_reader.position(),
                           end.position());
            return false;
        }
    }

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr bool try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<_n<Derived>, Reader> parser(reader);
            auto                                        result = parser.try_parse(reader);
            end                                                = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return _parse_rule<NextParser>(context, reader, end, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::token_parser_for<_n<Derived>, Reader> parser(reader);
            if (!parser.try_parse(reader))
            {
                LEXY_ASSERT(parser.end.position() == reader.position(), "impl should be LL(1)");
                parser.report_error(context, reader);
                return false;
            }

            return _parse_rule<NextParser>(context, reader, parser.end, LEXY_FWD(args)...);
        }
    };
};

template <typename Derived>
struct _n : token_base<Derived>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr auto try_parse(Reader reader)
        {
            if constexpr (lexy::is_node_encoding<typename Reader::encoding>)
            {
                if (!Reader::encoding::match(reader.peek(), Derived::node_kind()))
                    return false;

                reader.bump();
                end = reader.current();
                return true;
            }
            else
            {
                // This happens when it is used as whitespace, which is inherited while parsing the
                // token lexeme, we don't match anything in that case.
                return std::false_type{};
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            constexpr auto name = Derived::node_kind_name();

            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), name);
            context.on(_ev::error{}, err);
        }
    };

    template <typename Rule>
    constexpr auto operator()(Rule) const
    {
        return _nr<Derived, Rule>{};
    }
};
} // namespace lexyd

//=== dsl::tnode ===//
namespace lexy
{
struct expected_token_end
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected token end";
    }
};
} // namespace lexy

namespace lexyd
{
template <auto Kind>
struct _tn : _n<_tn<Kind>>
{
    static LEXY_CONSTEVAL auto node_kind()
    {
        return Kind;
    }

    static LEXY_CONSTEVAL auto node_kind_name()
    {
        using lexy::token_kind_name;
        return token_kind_name(Kind);
    }

    using node_end_error = lexy::expected_token_end;

    template <typename Reader>
    static constexpr auto node_child_reader(Reader& reader)
    {
        return reader.lexeme_reader();
    }
};

template <auto Kind>
constexpr auto tnode = _tn<Kind>{};
} // namespace lexyd

namespace lexy
{
template <auto Kind>
constexpr auto token_kind_of<lexy::dsl::_tn<Kind>> = Kind;
} // namespace lexy

//=== dsl::pnode ===//
namespace lexy
{
struct expected_production_end
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected production end";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Production>
struct _pn : _n<_pn<Production>>
{
    static_assert(lexy::is_production<Production>);

    static LEXY_CONSTEVAL auto node_kind()
    {
        return Production{};
    }

    static LEXY_CONSTEVAL auto node_kind_name()
    {
        return lexy::production_name<Production>();
    }

    using node_end_error = lexy::expected_production_end;

    template <typename Reader>
    static constexpr auto node_child_reader(Reader& reader)
    {
        return reader.child_reader();
    }
};

template <typename Production>
constexpr auto pnode = _pn<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PARSE_TREE_NODE_HPP_INCLUDED

#endif

#endif // LEXY_DSL_HPP_INCLUDED


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED
#define LEXY_EXT_COMPILER_EXPLORER_HPP_INCLUDED

#include <cstdio>
// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

    buffer_builder(const buffer_builder&)            = delete;
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
            if (!_buffer || !rhs._buffer)
                return !_buffer && !rhs._buffer;
            else
            {
                LEXY_PRECONDITION(_buffer == rhs._buffer);
                return _idx == rhs._idx;
            }
        }

        constexpr std::size_t index() const noexcept
        {
            return _idx;
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


// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_BUFFER_HPP_INCLUDED
#define LEXY_INPUT_BUFFER_HPP_INCLUDED

#include <cstring>






namespace lexy
{
// The reader used by the buffer if it can use a sentinel.
template <typename Encoding>
class _br : public _detail::swar_reader_base<_br<Encoding>>
{
public:
    using encoding = Encoding;
    using iterator = const typename Encoding::char_type*;

    struct marker
    {
        iterator _it;

        constexpr iterator position() const noexcept
        {
            return _it;
        }
    };

    explicit _br(iterator begin) noexcept : _cur(begin) {}

    auto peek() const noexcept
    {
        // The last one will be EOF.
        return *_cur;
    }

    void bump() noexcept
    {
        ++_cur;
    }

    iterator position() const noexcept
    {
        return _cur;
    }

    marker current() const noexcept
    {
        return {_cur};
    }
    void reset(marker m) noexcept
    {
        _cur = m._it;
    }

private:
    iterator _cur;
};

// We use aliases for the three encodings that can actually use it.
// (i.e. where char_type == int_type).
LEXY_INSTANTIATION_NEWTYPE(_bra, _br, lexy::ascii_encoding);
LEXY_INSTANTIATION_NEWTYPE(_br8, _br, lexy::utf8_encoding);
LEXY_INSTANTIATION_NEWTYPE(_brc, _br, lexy::utf8_char_encoding);
LEXY_INSTANTIATION_NEWTYPE(_br32, _br, lexy::utf32_encoding);

// Create the appropriate buffer reader.
template <typename Encoding>
constexpr auto _buffer_reader(const typename Encoding::char_type* data)
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
        return _bra(data);
    else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
        return _br8(data);
    else if constexpr (std::is_same_v<Encoding, lexy::utf8_char_encoding>)
        return _brc(data);
    else if constexpr (std::is_same_v<Encoding, lexy::utf32_encoding>)
        return _br32(data);
    else
        return _br<Encoding>(data);
}
} // namespace lexy

namespace lexy
{
/// Stores the input that will be parsed.
/// For encodings with spare code points, it can append an EOF sentinel.
/// This allows branch-less detection of EOF.
template <typename Encoding = default_encoding, typename MemoryResource = void>
class buffer
{
    static_assert(lexy::is_char_encoding<Encoding>);
    static constexpr auto _has_sentinel
        = std::is_same_v<typename Encoding::char_type, typename Encoding::int_type>;

public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;
    static_assert(std::is_trivially_copyable_v<char_type>);

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

    static buffer adopt(const char_type* data, std::size_t size,
                        MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    {
        buffer result(resource);
        // We can cast away the const-ness, since we require that `data` came from a buffer
        // origionally, where it wasn't const.
        result._data = const_cast<char_type*>(data);
        result._size = size;
        return result;
    }

    constexpr buffer() noexcept : buffer(_detail::get_memory_resource<MemoryResource>()) {}

    constexpr explicit buffer(MemoryResource* resource) noexcept
    : _resource(resource), _data(nullptr), _size(0)
    {}

    explicit buffer(const char_type* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : _resource(resource), _size(size)
    {
        _data = allocate(size);
        if (size > 0)
            std::memcpy(_data, data, size * sizeof(char_type));
    }
    explicit buffer(const char_type* begin, const char_type* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(begin, std::size_t(end - begin), resource)
    {}

    template <typename CharT, typename = _detail::require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(data), size, resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }
    template <typename CharT, typename = _detail::require_secondary_char_type<encoding, CharT>>
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
            _resource->deallocate(_data,
                                  _detail::round_size_for_swar(_size + 1) * sizeof(char_type),
                                  alignof(char_type));
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

    const char_type* release() && noexcept
    {
        auto result = _data;
        _data       = nullptr;
        _size       = 0;
        return result;
    }

    //=== input ===//
    auto reader() const& noexcept
    {
        if constexpr (_has_sentinel)
            return _buffer_reader<encoding>(_data);
        else
            return _range_reader<encoding>(_data, _data + _size);
    }

private:
    char_type* allocate(std::size_t size) const
    {
        if constexpr (_has_sentinel)
        {
            auto mem_size = _detail::round_size_for_swar(size + 1);
            auto memory   = static_cast<char_type*>(
                _resource->allocate(mem_size * sizeof(char_type), alignof(char_type)));

            for (auto ptr = memory + size; ptr != memory + mem_size; ++ptr)
                *ptr = encoding::eof();

            return memory;
        }
        else
        {
            return static_cast<char_type*>(
                _resource->allocate(size * sizeof(char_type), alignof(char_type)));
        }
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
buffer(const View&) -> buffer<deduce_encoding<LEXY_DECAY_DECLTYPE(*LEXY_DECLVAL(View).data())>>;

template <typename CharT, typename MemoryResource>
buffer(const CharT*, const CharT*,
       MemoryResource*) -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename CharT, typename MemoryResource>
buffer(const CharT*, std::size_t,
       MemoryResource*) -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename View, typename MemoryResource>
buffer(const View&, MemoryResource*)
    -> buffer<deduce_encoding<LEXY_DECAY_DECLTYPE(*LEXY_DECLVAL(View).data())>, MemoryResource>;

//=== make_buffer ===//
template <typename Encoding, encoding_endianness Endian>
struct _make_buffer
{
    template <typename MemoryResource = void>
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
    template <typename MemoryResource = void>
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
struct _make_buffer<utf8_char_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = void>
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

        return _make_buffer<utf8_char_encoding, encoding_endianness::big>{}(memory, size, resource);
    }
};
template <>
struct _make_buffer<utf16_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = void>
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
    template <typename MemoryResource = void>
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

//=== make_buffer_from_input ===//
template <typename Input>
using _detect_input_data = decltype(LEXY_DECLVAL(Input&).data());

template <typename Input, typename MemoryResource = void>
constexpr auto make_buffer_from_input(const Input&    input,
                                      MemoryResource* resource
                                      = _detail::get_memory_resource<MemoryResource>())
    -> buffer<typename input_reader<Input>::encoding, MemoryResource>
{
    using type = buffer<typename input_reader<Input>::encoding, MemoryResource>;
    if constexpr (_detail::is_detected<_detect_input_data, Input>)
    {
        return type(input.data(), input.size(), resource);
    }
    else
    {
        auto reader = input.reader();
        auto begin  = reader.position();
        while (reader.peek() != input_reader<Input>::encoding::eof())
            reader.bump();
        auto end = reader.position();

        if constexpr (std::is_pointer_v<decltype(begin)>)
        {
            return type(begin, end, resource);
        }
        else
        {
            auto                   size = _detail::range_size(begin, end);
            typename type::builder builder(size, resource);
            auto                   dest = builder.data();
            for (auto cur = begin; cur != end; ++cur)
                *dest++ = *cur; // NOLINT: clang-analyzer thinks this might access zero memory for
                                // some reason?!
            return LEXY_MOV(builder).finish();
        }
    }
}

//=== convenience typedefs ===//
template <typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_lexeme = lexeme_for<buffer<Encoding, MemoryResource>>;

template <typename Tag, typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_error = error_for<buffer<Encoding, MemoryResource>, Tag>;

template <typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_error_context = error_context<buffer<Encoding, MemoryResource>>;
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

// Copyright (C) 2020-2025 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>





namespace lexy_ext
{
/// The kind of diagnostic message.
enum class diagnostic_kind
{
    error,
    warning,
    note,
    info,
    debug,
    fixit,
    help,
};

/// Classifies a source code annotation.
enum class annotation_kind
{
    // foo
    // ^^^ primary annotation
    primary,
    // bar
    // ~~~ secondary annotation
    secondary,
};

/// Formats and writes diagnostic messages.
template <typename Input>
class diagnostic_writer
{
public:
    explicit diagnostic_writer(const Input& input, lexy::visualization_options opts = {})
    : _input(&input), _opts(opts)
    {}

    //=== writers ===//
    /// Writes a message.
    ///
    /// error: A message.
    template <typename OutputIt, typename Writer>
    OutputIt write_message(OutputIt out, diagnostic_kind kind, const Writer& message) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        switch (kind)
        {
        case diagnostic_kind::error:
            out = write_color<color::red, color::bold>(out, _opts);
            out = write_str(out, "error: ");
            break;
        case diagnostic_kind::warning:
            out = write_color<color::yellow, color::bold>(out, _opts);
            out = write_str(out, " warn: ");
            break;
        case diagnostic_kind::note:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, " note: ");
            break;
        case diagnostic_kind::info:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, " info: ");
            break;
        case diagnostic_kind::debug:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, "debug: ");
            break;
        case diagnostic_kind::fixit:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, "fixit: ");
            break;
        case diagnostic_kind::help:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, " help: ");
            break;
        }
        out = write_color<color::reset>(out, _opts);

        out    = message(out, _opts);
        *out++ = '\n';

        return out;
    }

    /// Writes a path.
    template <typename OutputIt>
    OutputIt write_path(OutputIt out, const char* path) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        out    = write_color<color::blue>(out, _opts);
        out    = write_str(out, path);
        out    = write_color<color::reset>(out, _opts);
        *out++ = '\n';
        return out;
    }

    /// Writes an empty annotation.
    ///
    ///    |\n
    template <typename OutputIt>
    OutputIt write_empty_annotation(OutputIt out) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = '\n';
        return out;
    }

    /// Writes a highlighted line with an annotation.
    ///
    /// 100 | void foo();
    ///     |      ^^^ annotation
    template <typename OutputIt, typename Location, typename IteratorOrSize, typename Writer>
    OutputIt write_annotation(OutputIt out, annotation_kind kind, const Location& begin_location,
                              IteratorOrSize end, const Writer& message) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        auto line = lexy::get_input_line_annotation(*_input, begin_location, end);
        // If we had to truncate the annotation but don't include the newline,
        // this is a "multiline" annotation in the last line.
        auto annotate_eof = line.truncated_multiline && !line.annotated_newline;

        //=== Line with file contents ===//
        // Location column.
        out    = write_color<color::blue>(out, _opts);
        out    = write_format(out, "%4zd ", begin_location.line_nr());
        out    = write_color<color::reset>(out, _opts);
        out    = write_str(out, column());
        *out++ = ' ';

        // Print before underlined normally.
        out = lexy::visualize_to(out, line.before, _opts);

        // Print underlined colored.
        out = colorize_underline(out, kind);
        out = lexy::visualize_to(out, line.annotated, _opts.reset(lexy::visualize_use_color));
        out = write_color<color::reset>(out, _opts);

        // Print after underlined normally.
        out    = lexy::visualize_to(out, line.after, _opts);
        *out++ = '\n';

        //=== Line with annotation ===//
        // Initial column.
        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = ' ';

        // Indent until the underline.
        auto indent_count = lexy::visualization_display_width(line.before, _opts);
        for (auto i = 0u; i != indent_count; ++i)
            *out++ = ' ';

        // Colorize.
        out = colorize_underline(out, kind);

        // Then underline.
        auto underline_count = lexy::visualization_display_width(line.annotated, _opts);
        for (auto i = 0u; i != underline_count; ++i)
            out = write_str(out, underline(kind));
        if (underline_count == 0 || annotate_eof)
            out = write_str(out, underline(kind));
        *out++ = ' ';

        // Print the message.
        out    = message(out, _opts.reset(lexy::visualize_use_color));
        *out++ = '\n';

        return write_color<color::reset>(out, _opts);
    }

private:
    const auto* column() const
    {
        if (_opts.is_set(lexy::visualize_use_unicode))
            return u8"│";
        else
            return u8"|";
    }

    template <typename OutputIt>
    OutputIt colorize_underline(OutputIt out, annotation_kind kind) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        switch (kind)
        {
        case annotation_kind::primary:
            return write_color<color::red, color::bold>(out, _opts);
        case annotation_kind::secondary:
            return write_color<color::yellow>(out, _opts);
        }

        return out;
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

    const Input*                _input;
    lexy::visualization_options _opts;
};
} // namespace lexy_ext

namespace lexy_ext::_detail
{
template <typename OutputIt, typename Input, typename Reader, typename Tag>
OutputIt write_error(OutputIt out, const lexy::error_context<Input>& context,
                     const lexy::error<Reader, Tag>& error, lexy::visualization_options opts,
                     const char* path)
{
    diagnostic_writer<Input> writer(context.input(), opts);

    // Convert the context location and error location into line/column information.
    auto context_location = lexy::get_input_location(context.input(), context.position());
    auto location
        = lexy::get_input_location(context.input(), error.position(), context_location.anchor());

    // Write the main error headline.
    out = writer.write_message(out, diagnostic_kind::error,
                               [&](OutputIt out, lexy::visualization_options) {
                                   out = lexy::_detail::write_str(out, "while parsing ");
                                   out = lexy::_detail::write_str(out, context.production());
                                   return out;
                               });
    if (path != nullptr)
        out = writer.write_path(out, path);
    out = writer.write_empty_annotation(out);

    // Write an annotation for the context.
    if (location.line_nr() != context_location.line_nr())
    {
        out = writer.write_annotation(out, annotation_kind::secondary, context_location,
                                      lexy::_detail::next(context.position()),
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, "beginning here");
                                      });
        out = writer.write_empty_annotation(out);
    }

    // Write the main annotation.
    if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                    error.length());

        out = writer.write_annotation(out, annotation_kind::primary, location, error.index() + 1,
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                    error.length());

        out = writer.write_annotation(out, annotation_kind::primary, location, error.end(),
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected keyword '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
    {
        out = writer.write_annotation(out, annotation_kind::primary, location, 1u,
                                      [&](OutputIt out, lexy::visualization_options) {
                                          out = lexy::_detail::write_str(out, "expected ");
                                          out = lexy::_detail::write_str(out, error.name());
                                          return out;
                                      });
    }
    else
    {
        out = writer.write_annotation(out, annotation_kind::primary, location, error.end(),
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, error.message());
                                      });
    }

    return out;
}
} // namespace lexy_ext::_detail

namespace lexy_ext
{
template <typename OutputIterator>
struct _report_error
{
    OutputIterator              _iter;
    lexy::visualization_options _opts;
    const char*                 _path;

    struct _sink
    {
        OutputIterator              _iter;
        lexy::visualization_options _opts;
        const char*                 _path;
        std::size_t                 _count;

        using return_type = std::size_t;

        template <typename Input, typename Reader, typename Tag>
        void operator()(const lexy::error_context<Input>& context,
                        const lexy::error<Reader, Tag>&   error)
        {
            _iter = _detail::write_error(_iter, context, error, _opts, _path);
            ++_count;
        }

        std::size_t finish() &&
        {
            if (_count != 0)
                *_iter++ = '\n';
            return _count;
        }
    };
    constexpr auto sink() const
    {
        return _sink{_iter, _opts, _path, 0};
    }

    /// Specifies a path that will be printed alongside the diagnostic.
    constexpr _report_error path(const char* path) const
    {
        return {_iter, _opts, path};
    }

    /// Specifies an output iterator where the errors are written to.
    template <typename OI>
    constexpr _report_error<OI> to(OI out) const
    {
        return {out, _opts, _path};
    }

    /// Overrides visualization options.
    constexpr _report_error opts(lexy::visualization_options opts) const
    {
        return {_iter, opts, _path};
    }
};

/// An error callback that uses diagnostic_writer to print to stderr (by default).
constexpr auto report_error = _report_error<lexy::stderr_output_iterator>{};
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

