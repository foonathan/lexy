#define LEXY_HAS_UNICODE_DATABASE 1
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

/// Creates a new type from the instantiation of a template.
/// This is used to shorten type names.
#define LEXY_INSTANTIATION_NEWTYPE(Name, Templ, ...)                                               \
    struct Name : Templ<__VA_ARGS__>                                                               \
    {                                                                                              \
        using Templ<__VA_ARGS__>::Templ;                                                           \
    }

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
//   See https://github.com/foonathan/lexy/issues/15.
#    if __cpp_nontype_template_parameter_class >= 201806 || __cpp_nontype_template_args >= 201911
#        define LEXY_HAS_NTTP 1
#    else
#        define LEXY_HAS_NTTP 0
#    endif
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

    _lazy_init_storage_non_trivial(_lazy_init_storage_non_trivial&& other) noexcept
    : _init(other._init), _empty()
    {
        if (_init)
            ::new (static_cast<void*>(&_value)) T(LEXY_MOV(other._value));
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
        LEXY_PRECONDITION(!*this);

        *this = lazy_init(0, LEXY_FWD(args)...);
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED
#define LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED


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
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = string_view("auto lexy::_detail::_full_type_name() [T = ");
    constexpr auto suffix = string_view("]");

    auto function = string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
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
    return function;

#elif defined(_MSC_VER)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1

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
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 0

    return string_view("unknown-type");

#endif
}

template <typename T, int NsCount>
constexpr string_view _type_name()
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

template <typename T>
constexpr const void* type_id()
{
    // As different types have different type names, the compiler can't merge them,
    // and we necessarily have different addresses.
    if constexpr (_detail::is_detected<_detect_name_f, T>)
        return T::name();
    else if constexpr (_detail::is_detected<_detect_name_v, T>)
        return T::name;
    else
    {
        static_assert(LEXY_HAS_AUTOMATIC_TYPE_NAME,
                      "you need to manuall add a ::name() or ::name to your type");
        return _full_type_name<T>().data();
    }
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TYPE_NAME_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_NOOP_HPP_INCLUDED
#define LEXY_CALLBACK_NOOP_HPP_INCLUDED

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

template <typename T, typename State>
using _detect_callback_state = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(const State&)]);
template <typename T, typename State>
constexpr bool is_callback_state
    = _detail::is_detected<_detect_callback_state, T, std::decay_t<State>>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(const T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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
} // namespace lexy

//=== predefined_token_kind ===//
namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind              = UINT_LEAST16_MAX,
    error_token_kind                = UINT_LEAST16_MAX - 1,
    whitespace_token_kind           = UINT_LEAST16_MAX - 2,
    position_token_kind             = UINT_LEAST16_MAX - 3,
    eof_token_kind                  = UINT_LEAST16_MAX - 4,
    eol_token_kind                  = UINT_LEAST16_MAX - 5,
    identifier_token_kind           = UINT_LEAST16_MAX - 6,
    _smallest_predefined_token_kind = identifier_token_kind,
};

constexpr const char* _kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";
    case error_token_kind:
        return "error token";
    case whitespace_token_kind:
        return "whitespace";
    case position_token_kind:
        return "position";
    case eof_token_kind:
        return "EOF";
    case eol_token_kind:
        return "EOL";
    case identifier_token_kind:
        return "identifier";
    }

    return ""; // unreachable
}

/// Specialize to define the token kind of a rule.
template <typename TokenRule>
constexpr auto token_kind_of = nullptr;

template <typename TokenRule>
constexpr auto token_kind_of<const TokenRule> = token_kind_of<TokenRule>;
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
template <typename To, typename... Args>
inline constexpr auto _is_convertible = false;
template <typename To, typename Arg>
inline constexpr auto _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;
template <>
inline constexpr auto _is_convertible<void> = true;

template <typename Production, typename ParseState = void>
class production_value_callback
{
    using _type = std::decay_t<decltype(Production::value)>;

    static auto _return_type_callback()
    {
        if constexpr (lexy::is_callback<_type>)
            return Production::value;
        else if constexpr (lexy::is_sink<_type, ParseState>)
            return Production::value.sink(LEXY_DECLVAL(const ParseState&));
        else
            return Production::value.sink();
    }

public:
    constexpr explicit production_value_callback(const ParseState* state) : _state(state) {}

    template <typename State = ParseState, typename = std::enable_if_t<std::is_void_v<State>>>
    constexpr production_value_callback() : _state(nullptr)
    {}

    template <typename State = ParseState,
              typename       = std::enable_if_t<std::is_same_v<State, ParseState>>>
    constexpr explicit production_value_callback(const State& state) : _state(&state)
    {}

    using return_type = typename decltype(_return_type_callback())::return_type;

    constexpr auto sink() const
    {
        if constexpr (lexy::is_sink<_type, ParseState>)
            return Production::value.sink(*_state);
        else
            return Production::value.sink();
    }

    template <typename... Args>
    constexpr return_type operator()(Args&&... args)
    {
        if constexpr (lexy::is_callback_for<_type, Args&&...>)
        {
            if constexpr (lexy::is_callback_state<_type, ParseState>)
                return Production::value[*_state](LEXY_FWD(args)...);
            else
                return Production::value(LEXY_FWD(args)...);
        }
        else if constexpr (lexy::is_sink<_type> || lexy::is_sink<_type, ParseState>)
        {
            if constexpr (_is_convertible<return_type, Args&&...>)
            {
                // We don't have a matching callback, but it is a single argument that has
                // the correct type already, or we return void and have no arguments.
                // Assume it came from the list sink and return the value without invoking a
                // callback.
                return (LEXY_FWD(args), ...);
            }
            else
            {
                static_assert(_detail::error<Production, Args...>,
                              "missing value callback overload for production; only have sink");
            }
        }
        else
        {
            static_assert(_detail::error<Production, Args...>,
                          "missing value callback overload for production");
        }
    }

private:
    const ParseState* _state;
};
} // namespace lexy

#endif // LEXY_GRAMMAR_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED


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

    /// An iterator of char_type, not int_type.
    using iterator = ForwardIterator;

    /// If the reader is at eof, returns Encoding::eof().
    /// Otherwise, returns Encoding::to_int_type(/* current character */).
    typename Encoding::int_type peek() const;

    /// Advances to the next character in the input.
    void bump();

    /// Returns an iterator to the current character.
    /// The following code must produce a valid range:
    /// ```
    /// auto begin = reader.position();
    /// reader.bump();
    /// ... // more bumps
    /// auto end = reader.position();
    /// ```
    iterator position() const;

    /// Sets the reader to a position.
    /// It must be returned by a previous call to `position()` of this reader or a copy,
    /// and can either backtrack the reader or move it forward.
    void set_position(iterator new_pos);
};

/// An Input produces a reader.
class Input
{
public:
    /// Returns a reader to the beginning of the input.
    Reader reader() const &;
};
#endif

namespace lexy
{
// A generic reader from an iterator range.
template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class _rr
{
public:
    using encoding = Encoding;
    using iterator = Iterator;

    constexpr explicit _rr(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end)
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(begin, end));
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
        LEXY_PRECONDITION(_cur != _end);
        ++_cur;
    }

    constexpr iterator position() const noexcept
    {
        return _cur;
    }

    constexpr void set_position(iterator new_pos) noexcept
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(new_pos, _end));
        _cur = new_pos;
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

template <typename Reader, typename CharT>
constexpr bool char_type_compatible_with_reader
    = (std::is_same_v<CharT, typename Reader::encoding::char_type>)
      || Reader::encoding::template is_secondary_char_type<CharT>();

template <typename Reader>
constexpr auto partial_reader(const Reader&, typename Reader::iterator begin,
                              typename Reader::iterator end)
{
    return _range_reader<typename Reader::encoding>(begin, end);
}

/// Creates a reader that only reads until the given end.
template <typename Reader>
constexpr auto partial_reader(const Reader& reader, typename Reader::iterator end)
{
    return partial_reader(reader, reader.position(), end);
}
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED


//=== parse_events ===//
namespace lexy::parse_events
{
/// Start of the given production.
/// Arguments: position
struct production_start
{};
/// End of the given production.
/// Arguments: position
struct production_finish
{};
/// Production is canceled.
/// Arguments: position
struct production_cancel
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

template <typename BranchRule, typename Context, typename Reader>
using branch_parser_for = typename BranchRule::template bp<Context, Reader>;

/// A branch parser that takes a branch unconditionally and forwards to the regular parser.
template <typename Rule, typename Context, typename Reader>
struct unconditional_branch_parser
{
    constexpr std::true_type try_parse(const Context&, const Reader&)
    {
        return {};
    }

    template <typename NextParser, typename... Args>
    LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
    {
        return parser_for<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
    }
};

/// A branch parser that parses a branch rule but with a special continuation.
template <typename BranchRule, typename Context, typename Reader,
          template <typename> typename Continuation>
struct continuation_branch_parser
{
    branch_parser_for<BranchRule, Context, Reader> impl;

    constexpr auto try_parse(Context& context, const Reader& reader)
    {
        return impl.try_parse(context, reader);
    }

    template <typename NextParser, typename... Args>
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
                                                 typename Context::root_production>>
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
        reader.set_position(parser.end);
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

        reader.set_position(parser.end);
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
        : parse_context_var_base(&type_id), value(LEXY_MOV(value))
        {}

        template <typename Context>
        static constexpr T& get(Context& context)
        {
            auto cb = context.control_block;

            for (auto cur = cb->vars; cur; cur = cur->next)
                if (cur->id == &type_id)
                    return static_cast<parse_context_var*>(cur)->value;

            LEXY_ASSERT(false, "context variable hasn't been created");
            return LEXY_DECLVAL(T&);
        }
    };

    template <typename Handler, typename State = void>
    struct parse_context_control_block
    {
        LEXY_EMPTY_MEMBER Handler parse_handler;
        const State*              parse_state;

        parse_context_var_base* vars;

        int  cur_depth, max_depth;
        bool enable_whitespace_skipping;

        constexpr parse_context_control_block(Handler&& handler, const State* state,
                                              std::size_t max_depth)
        : parse_handler(LEXY_MOV(handler)), parse_state(state), //
          vars(nullptr),                                        //
          cur_depth(0), max_depth(static_cast<int>(max_depth)), enable_whitespace_skipping(true)
        {}
    };
} // namespace _detail

template <typename Handler, typename State, typename Production,
          typename RootProduction = Production>
struct _pc
{
    using production      = Production;
    using root_production = RootProduction;
    using value_type = typename Handler::template value_callback<Production, State>::return_type;

    typename Handler::template event_handler<Production>  handler;
    _detail::parse_context_control_block<Handler, State>* control_block;
    _detail::lazy_init<value_type>                        value;

    constexpr explicit _pc(_detail::parse_context_control_block<Handler, State>* cb)
    : control_block(cb)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction)
    {
        // Update the root production if necessary.
        using new_root = std::conditional_t<lexy::is_token_production<ChildProduction>,
                                            ChildProduction, RootProduction>;
        return _pc<Handler, State, ChildProduction, new_root>(control_block);
    }

    constexpr auto value_callback()
    {
        using callback = typename Handler::template value_callback<Production, State>;
        return callback(control_block->parse_state);
    }

    template <typename Event, typename... Args>
    constexpr void on(Event ev, Args&&... args)
    {
        handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
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

template <typename Production, typename Handler, typename State, typename Reader>
constexpr auto do_action(Handler&& handler, const State* state, Reader& reader)
{
    static_assert(!std::is_reference_v<Handler>, "need to move handler in");

    _detail::parse_context_control_block control_block(LEXY_MOV(handler), state,
                                                       max_recursion_depth<Production>());
    _pc<Handler, State, Production>      context(&control_block);

    context.on(parse_events::production_start{}, reader.position());

    using parser     = lexy::parser_for<lexy::production_rule<Production>, _detail::final_parser>;
    auto rule_result = parser::parse(context, reader);

    if (rule_result)
        context.on(parse_events::production_finish{}, reader.position());
    else
        context.on(parse_events::production_cancel{}, reader.position());

    using value_type = typename decltype(context)::value_type;
    if constexpr (std::is_void_v<value_type>)
        return LEXY_MOV(control_block.parse_handler).get_result_void(rule_result);
    else if (context.value)
        return LEXY_MOV(control_block.parse_handler)
            .template get_result<value_type>(rule_result, LEXY_MOV(*context.value));
    else
        return LEXY_MOV(control_block.parse_handler).template get_result<value_type>(rule_result);
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_VALIDATE_HPP_INCLUDED
#define LEXY_ACTION_VALIDATE_HPP_INCLUDED




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
struct _list_sink
{
    Container _result;

    using return_type = Container;

    template <typename C = Container, typename U>
    auto operator()(U&& obj) -> decltype(LEXY_DECLVAL(C&).push_back(LEXY_FWD(obj)))
    {
        return _result.push_back(LEXY_FWD(obj));
    }

    template <typename C = Container, typename... Args>
    auto operator()(Args&&... args) -> decltype(LEXY_DECLVAL(C&).emplace_back(LEXY_FWD(args)...))
    {
        return _result.emplace_back(LEXY_FWD(args)...);
    }

    Container&& finish() &&
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
        const State&   _state;
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
            -> std::decay_t<decltype((LEXY_DECLVAL(Container&).push_back(LEXY_FWD(args)), ...),
                                     LEXY_DECLVAL(Container))>
        {
            Container result(_detail::invoke(_alloc, _state));
            if constexpr (_has_reserve<Container>)
                result.reserve(sizeof...(args));
            (result.emplace_back(LEXY_FWD(args)), ...);
            return result;
        }
    };

    template <typename State>
    constexpr auto operator[](const State& state) const
    {
        return _with_state<State>{state, _alloc};
    }

    template <typename State>
    constexpr auto sink(const State& state) const
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
        -> std::decay_t<decltype((LEXY_DECLVAL(Container&).push_back(LEXY_FWD(args)), ...),
                                 LEXY_DECLVAL(Container))>
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

namespace lexy
{
template <typename Container>
struct _collection_sink
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

template <typename Container, typename AllocFn>
struct _collection_alloc
{
    AllocFn _alloc;

    using return_type = Container;

    template <typename State>
    struct _with_state
    {
        const State&   _state;
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
            -> std::decay_t<decltype((LEXY_DECLVAL(Container&).insert(LEXY_FWD(args)), ...),
                                     LEXY_DECLVAL(Container))>
        {
            Container result(_detail::invoke(_alloc, _state));
            if constexpr (_has_reserve<Container>)
                result.reserve(sizeof...(args));
            (result.emplace(LEXY_FWD(args)), ...);
            return result;
        }
    };

    template <typename State>
    constexpr auto operator[](const State& state) const
    {
        return _with_state<State>{state, _alloc};
    }

    template <typename State>
    constexpr auto sink(const State& state) const
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
        -> std::decay_t<decltype((LEXY_DECLVAL(Container&).insert(LEXY_FWD(args)), ...),
                                 LEXY_DECLVAL(Container))>
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

#ifndef LEXY_ERROR_HPP_INCLUDED
#define LEXY_ERROR_HPP_INCLUDED





namespace lexy
{
/// Generic failure.
template <typename Reader, typename Tag>
class error
{
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
public:
    constexpr explicit error(typename Reader::iterator                   pos,
                             const typename Reader::encoding::char_type* str,
                             std::size_t                                 index) noexcept
    : _pos(pos), _str(str), _idx(index)
    {}

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

    constexpr auto character() const noexcept
    {
        return _str[_idx];
    }

private:
    typename Reader::iterator                   _pos;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _idx;
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
                             const typename Reader::encoding::char_type* str)
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

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

private:
    typename Reader::iterator                   _begin;
    typename Reader::iterator                   _end;
    const typename Reader::encoding::char_type* _str;
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
public:
    constexpr explicit validate_handler(const Input& input, const ErrorCallback& callback)
    : _sink(_get_error_sink(callback)), _input(&input)
    {}

    template <typename Production>
    class event_handler
    {
        using iterator = typename lexy::input_reader<Input>::iterator;

    public:
        constexpr event_handler() = default;

        constexpr void on(validate_handler&, parse_events::production_start, iterator pos)
        {
            _begin = pos;
        }

        template <typename Error>
        constexpr void on(validate_handler& handler, parse_events::error, Error&& error)
        {
            lexy::error_context err_ctx(Production{}, *handler._input, _begin);
            handler._sink(err_ctx, LEXY_FWD(error));
        }

        template <typename Event, typename... Args>
        constexpr void on(validate_handler&, Event, const Args&...)
        {}

    private:
        iterator _begin = {};
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr auto get_result_void(bool rule_parse_result) &&
    {
        return validate_result<ErrorCallback>(rule_parse_result, LEXY_MOV(_sink).finish());
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
    return lexy::do_action<Production>(LEXY_MOV(handler), no_parse_state, reader);
}

template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto validate(const Input& input, const State& state, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = validate_handler(input, callback);
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), &state, reader);
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

#ifndef LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED
#define LEXY_DETAIL_MEMORY_RESOURCE_HPP_INCLUDED

#include <cstring>


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

template <typename MemoryResource,
          typename
          = std::enable_if_t<std::is_void_v<MemoryResource> || std::is_empty_v<MemoryResource>>>
constexpr MemoryResource* get_memory_resource()
{
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
    : _begin(begin), _end(reader.position())
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

template <typename TokenRule>
constexpr auto _has_special_token_kind = [] {
    using kind = std::decay_t<decltype(lexy::token_kind_of<TokenRule>)>;
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
    constexpr token_kind(TokenRule) noexcept : token_kind()
    {
        // Look for internal mapping first.
        constexpr auto token_rule_kind = lexy::token_kind_of<TokenRule>;
        if constexpr (_is_compatible_kind_type<decltype(token_rule_kind)>())
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
template <typename TokenRule, typename = std::enable_if_t<_has_special_token_kind<TokenRule>>>
token_kind(TokenRule) -> token_kind<std::decay_t<decltype(lexy::token_kind_of<TokenRule>)>>;
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
token(TokenRule, lexy::lexeme<Reader>)
    -> token<Reader, std::decay_t<decltype(lexy::token_kind_of<TokenRule>)>>;

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
template <typename Reader, typename TokenKind = void, typename MemoryResource = void>
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

template <typename Input, typename TokenKind = void, typename MemoryResource = void>
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
            // We merge subsequent token nodes with the same kind inside of a token production,
            // or if the kind is error_token_kind.
            token && token->kind == kind
            && (_cur.prod->token_production || kind == lexy::error_token_kind))
        {
            // No need to allocate a new node, just extend the previous node.
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
template <typename Tree, typename Input, typename ErrorCallback>
class parse_tree_handler
{
public:
    explicit parse_tree_handler(Tree& tree, const Input& input, const ErrorCallback& cb)
    : _tree(&tree), _depth(0), _validate(input, cb)
    {}

    template <typename Production>
    class event_handler
    {
        using iterator = typename lexy::input_reader<Input>::iterator;

    public:
        void on(parse_tree_handler& handler, parse_events::production_start ev, iterator pos)
        {
            if (handler._depth++ == 0)
                handler._builder.emplace(LEXY_MOV(*handler._tree), Production{});
            else
                _marker = handler._builder->start_production(Production{});

            _validate.on(handler._validate, ev, pos);
        }

        void on(parse_tree_handler& handler, parse_events::production_finish ev, iterator pos)
        {
            if (--handler._depth == 0)
                *handler._tree = LEXY_MOV(*handler._builder).finish();
            else
                handler._builder->finish_production(LEXY_MOV(_marker));

            _validate.on(handler._validate, ev, pos);
        }

        void on(parse_tree_handler& handler, parse_events::production_cancel ev, iterator pos)
        {
            if (--handler._depth == 0)
                handler._tree->clear();
            else
                handler._builder->cancel_production(LEXY_MOV(_marker));

            _validate.on(handler._validate, ev, pos);
        }

        template <typename TokenKind>
        void on(parse_tree_handler& handler, parse_events::token ev, TokenKind kind, iterator begin,
                iterator end)
        {
            handler._builder->token(kind, begin, end);
            _validate.on(handler._validate, ev, kind, begin, end);
        }

        template <typename Error>
        void on(parse_tree_handler& handler, parse_events::error ev, Error&& error)
        {
            _validate.on(handler._validate, ev, LEXY_FWD(error));
        }

        template <typename Event, typename... Args>
        void on(parse_tree_handler& handler, Event ev, Args&&... args)
        {
            _validate.on(handler._validate, ev, LEXY_FWD(args)...);
        }

    private:
        typename Tree::builder::marker _marker;
        typename validate_handler<Input, ErrorCallback>::template event_handler<Production>
            _validate;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr auto get_result_void(bool rule_parse_result) &&
    {
        return LEXY_MOV(_validate).get_result_void(rule_parse_result);
    }

private:
    lexy::_detail::lazy_init<typename Tree::builder> _builder;
    Tree*                                            _tree;
    int                                              _depth;

    validate_handler<Input, ErrorCallback> _validate;
};

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = parse_tree_handler(tree, input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), no_parse_state, reader);
}

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename State, typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const State& state, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    auto handler = parse_tree_handler(tree, input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), &state, reader);
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
    static constexpr T c_str[sizeof...(Cs) + 1] = {T(Cs)..., T()};
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
            ::lexy::_detail::type_string<::std::decay_t<decltype(Str[0])>,                         \
                                         (I >= LEXY_NTTP_STRING_LENGTH(Str) ? Str[0] : Str[I])>,   \
            ::lexy::_detail::type_string<::std::decay_t<decltype(Str[0])>>>

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



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_VISUALIZE_HPP_INCLUDED
#define LEXY_VISUALIZE_HPP_INCLUDED

#include <cstdio>

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_CODE_POINT_HPP_INCLUDED
#define LEXY_DETAIL_CODE_POINT_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CODE_POINT_HPP_INCLUDED
#define LEXY_CODE_POINT_HPP_INCLUDED

#include <cstdint>



#ifndef LEXY_HAS_UNICODE_DATABASE
#    define LEXY_HAS_UNICODE_DATABASE 0
#endif

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
        LEXY_UNICODE_CATEGORY(Mc, spaing_mark),
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

    LEXY_UNICODE_CONSTEXPR general_category_t general_category() const;

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

#define LEXY_UNICODE_DATABASE_VERSION "14.0.0"

namespace lexy::_unicode_db
{
constexpr const char* block_starts = "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\021\025\026\027\030\031\032\033\034\035\036\037 !\"#$%&'(!)*+,-./0'\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\0211\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\0212\021\021\0213\021456789\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021:;;;;;;;;<<<<<<<<<<<<<<<<<<<<<<<<<\021=>?@ABCDEFGH\021IJKLMNOPQRSTUVWXYZ[\\]^_`a\021\021\021bcd]]]]]]]]]e\021\021\021\021f]]]]]]]]]]]]]]]\021\021g]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\021\021hi]]jk\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021l\021\021\021\021mn]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]o\021pq]]]]]]]]]r]]]]]]]]]]]]]]]]]]stuvwxyz{''|]]]]}~\177\200]]]]\201\202\203]]\204\205\206]\207\210\211\212''\213\214\215'\216\217]]]]\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\220\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\221\222\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\223\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\224]]]]]]]]]]]]\021\021\225]]]]]\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\226]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"    
"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"    
"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"    
"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\227\230]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\231"    
"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\231";
constexpr const char* blocks = "\000\000\000\000\000\000\000\000\000\001\001\001\001\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\003\003\003\004\003\003\003\005\006\003\007\003\010\003\003\011\011\011\011\011\011\011\011\011\011\003\003\007\007\007\003\003\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\005\003\006\013\014\013\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\005\007\006\007\000\000\000\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\003\004\004\004\004\016\003\013\016\017\020\007\021\016\013\016\007\022\022\013\015\003\023\013\022\017\024\022\022\022\003\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\012\015\012\015\012\015\015\015\012\012\015\012\015\012\012\015\012\012\012\015\015\012\012\012\012\015\012\012\015\012\012\012\015\015\015\012\012\015\012\012\015\012\015\012\015\012\012\015\012\015\015\012\015\012\012\015\012\012\012\015\012\015\012\012\015\015\025\012\015\015\015\025\025\025\025\012\026\015\012\026\015\012\026\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\012\026\015\012\015\012\012\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\015\015\015\015\015\012\012\015\012\012\015\015\012\015\012\012\012\012\015\012\015\012\015\012\015\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\025\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\030\030\030\030\030\030\030\027\027\013\013\013\013\030\030\030\030\030\030\030\030\030\030\030\030\013\013\013\013\013\013\013\013\013\013\013\013\013\013\027\027\027\027\027\013\013\013\013\013\013\013\030\013\030\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\032\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\012\015\012\015\030\013\012\015\033\033\034\015\015\015\003\012\033\033\033\033\013\013\012\023\012\012\012\033\012\033\012\012\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\015\015\012\012\012\015\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\015\015\015\012\015\007\012\015\012\012\015\015\012\012\012"    
"\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\016\031\031\031\031\031\035\035\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\033\030\003\003\003\003\003\003\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\003\010\033\033\016\016\004\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\036\036\036\036\036\036\036\036\036\036\036\036\036\036\010\036\003\036\036\003\036\036\003\036\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\025\025\025\025\003\003\033\033\033\033\033\033\033\033\033\033\033\021\021\021\021\021\021\007\007\007\003\003\004\003\003\016\016\036\036\036\036\036\036\036\036\036\036\036\003\021\003\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\036\036\031\036\036\036\036\036\036\036\011\011\011\011\011\011\011\011\011\011\003\003\003\003\025\025\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\003\025\036\036\036\036\036\036\036\021\016\031\031\036\036\036\036\030\030\036\036\016\031\031\031\036\025\025\011\011\011\011\011\011\011\011\011\011\025\025\025\016\016\025\003\003\003\003\003\003\003\003\003\003\003\003\003\003\033\021\025\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\031\031\031\031\031\031\031\031\031\031\031\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\031\031\031\031\031\031\031\030\030\016\003\003\003\030\033\033\031\004\004"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\031\031\030\036\036\036\036\036\036\036\036\036\030\036\036\036\030\036\036\036\036\031\033\033\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\031\033\033\003\033\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\013\025\025\025\025\025\025\033\021\021\033\033\033\033\033\033\031\031\031\031\031\031\031\031\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\031\031\031\031\031\031\031\031\031\031\036\036\036\036\036\036\036\036\036\036\036\036\031\031\021\036\036\036\036\036\036\036\031\031\031\031\031\031\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\037\031\025\037\037\037\036\036\036\036\036\036\036\036\037\037\037\037\031\037\037\025\031\031\031\031\036\036\036\025\025\025\025\025\025\025\025\025\025\036\036\003\003\011\011\011\011\011\011\011\011\011\011\003\030\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\037\037\033\025\025\025\025\025\025\025\025\033\033\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\033\033\033\025\025\025\025\033\033\031\025\037\037\037\036\036\036\036\033\033\037\037\033\033\037\037\031\025\033\033\033\033\033\033\033\033\037\033\033\033\033\025\025\033\025\025\025\036\036\033\033\011\011\011\011\011\011\011\011\011\011\025\025\004\004\022\022\022\022\022\022\016\004\025\003\031\033\033\036\036\037\033\025\025\025\025\025\025\033\033\033\033\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\033\025\025\033\025\025\033\033\031\033\037\037\037\036\036\033\033\033\033\036\036\033\033\036\036\031\033\033\033\036\033\033\033\033\033\033\033\025\025\025\025\033\025\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\036\036\025\025\025\036\003\033\033\033\033\033\033\033\033\033\033\036\036\037\033\025\025\025\025\025\025\025\025\025\033\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\033\033\031\025\037\037\037\036\036\036\036\036\033\036\036\037\033\037\037\031\033\033\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\036\036\033\033\011\011\011\011\011\011\011\011\011\011\003\004\033\033\033\033\033\033\033\025\036\036\036\031\031\031\033\036\037\037\033\025\025\025\025\025\025\025\025\033\033\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\033\033\031\025\037\036\037\036\036\036\036\033\033\037\037\033\033\037\037\031\033\033\033\033\033\033\033\031\036\037\033\033\033\033\025\025\033\025\025\025\036\036\033\033\011\011\011\011\011\011\011\011\011\011\016\025\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\036\025\033\025\025\025\025\025\025\033\033\033\025\025\025\033\025\025\025\025\033\033\033\025\025\033\025\033\025\025\033\033\033\025\025\033\033\033\025\025\025\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\037\037\036\037\037\033\033\033\037\037\037\033\037\037\037\031\033\033\025\033\033\033\033\033\033\037\033\033\033\033\033\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\022\022\022\016\016\016\016\016\016\004\016\033\033\033\033\033"    
"\036\037\037\037\031\025\025\025\025\025\025\025\025\033\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\031\025\036\036\036\037\037\037\037\033\036\036\036\033\036\036\036\031\033\033\033\033\033\033\033\036\036\033\025\025\025\033\033\025\033\033\025\025\036\036\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\033\003\022\022\022\022\022\022\022\016\025\036\037\037\003\025\025\025\025\025\025\025\025\033\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\033\033\031\025\037\036\037\037\037\037\037\033\036\037\037\033\037\037\036\031\033\033\033\033\033\033\033\037\037\033\033\033\033\033\033\025\025\033\025\025\036\036\033\033\011\011\011\011\011\011\011\011\011\011\033\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\036\036\037\037\025\025\025\025\025\025\025\025\025\033\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\025\037\037\037\036\036\036\036\033\037\037\037\033\037\037\037\031\025\016\033\033\033\033\025\025\025\037\022\022\022\022\022\022\022\025\025\025\036\036\033\033\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\016\025\025\025\025\025\025\033\036\037\037\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\033\025\033\033\025\025\025\025\025\025\025\033\033\033\031\033\033\033\033\037\037\037\036\036\036\033\036\033\037\037\037\037\037\037\037\037\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\037\037\003\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\025 \036\036\036\036\036\036\036\033\033\033\033\004\025\025\025\025\025\025\030\031\031\031\031\031\031\036\031\003\011\011\011\011\011\011\011\011\011\011\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\033\025\033\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\033\025\025\025\025\025\025\025\025\025\025\036\025 \036\036\036\036\036\036\031\036\036\025\033\033\025\025\025\025\025\033\030\033\031\031\031\031\031\036\033\033\011\011\011\011\011\011\011\011\011\011\033\033\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\016\016\016\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\016\003\016\016\016\031\031\016\016\016\016\016\016\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\022\016\031\016\031\016\031\005\006\005\006!!\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\036\036\036\036\036\036\036\036\036\036\036\036\036\036\037\036\036\031\031\031\003\031\031\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\033\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\033\016\016\016\016\016\016\016\016\031\016\016\016\016\016\016\033\016\016\003\003\003\003\003\016\016\016\016\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\036\036\036\036\037\036\036\036\036\036\031\037\031\031\037\037\036\036\025\011\011\011\011\011\011\011\011\011\011\003\003\003\003\003\003\025\025\025\025\025\025\037\037\036\036\025\025\025\025\036\036\036\025\037\037\037\025\025\037\037\037\037\037\037\037\025\025\025\036\036\036\036\025\025\025\025\025\025\025\025\025\025\025\025\025\036\037\037\036\036\037\037\037\037\037\037\036\025\037\011\011\011\011\011\011\011\011\011\011\037\037\037\036\016\016\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\012\033\033\033\033\033\012\033\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\003\030\015\015\015\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\033\033\025\025\025\025\025\025\025\033\025\033\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\033\033\025\025\025\025\025\025\025\033\025\033\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\031\031\031\003\003\003\003\003\003\003\003\003\"\"\"\"\"\"\"\"\"\022\022\022\022\022\022\022\022\022\022\022\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\033\015\015\015\015\015\015\033\033"    
"\010\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\016\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\002\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\005\006\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\003\003\003###\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\031!\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036!\003\003\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\033\036\036\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\037\036\036\036\036\036\036\036\037\037\037\037\037\037\037\037\036\037\037\031\031\031\031\031\031\031\031\031\031\031\003\003\003\030\003\003\003\004\025\031\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\003\003\003\003\003\003\010\003\003\003\003\031\031\031\021\031\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\025\025\025\025\025$$\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\025\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\036\036\036\037\037\037\037\036\036\037\037\037\033\033\033\033\037\037\036\037\037\037\037\037\037\031\031\031\033\033\033\033\016\033\033\033\003\003\011\011\011\011\011\011\011\011\011\011\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\"\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\037\037\036\033\033\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\036\037\036\036\036\036\036\036\036\033\031\037\036\037\037\036\036\036\036\036\036\036\036\037\037\037\037\037\037\036\036\031\031\031\031\031\031\031\031\033\033\031\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\003\003\003\003\003\003\003\030\003\003\003\003\003\003\033\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\035\036\036\031\031\031\031\031\031\031\031\031\031\031\036\036\036\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\036\036\036\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\037\036\036\036\036\036\037\036\037\037\037\037\037\036\037!\025\025\025\025\025\025\025\025\033\033\033\011\011\011\011\011\011\011\011\011\011\003\003\003\003\003\003\003\016\016\016\016\016\016\016\016\016\016\031\031\031\031\031\031\031\031\031\016\016\016\016\016\016\016\016\016\003\003\033\036\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\036\036\036\036\037\037\036\036!\031\036\036\025\025\011\011\011\011\011\011\011\011\011\011\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\037\036\036\037\037\037\036\037\036\036\036!!\033\033\033\033\033\033\033\033\003\003\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\037\037\037\037\037\036\036\036\036\036\036\036\036\037\037\036\031\033\033\033\003\003\003\003\003\011\011\011\011\011\011\011\011\011\011\033\033\033\025\025\025\011\011\011\011\011\011\011\011\011\011\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\030\030\030\030\030\003\003\015\015\015\015\015\015\015\015\015\033\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\033\012\012\012\003\003\003\003\003\003\003\003\033\033\033\033\033\033\033\033\031\031\031\003\031\031\031\031\031\031\031\031\031\031\031\031\031!\031\031\031\031\031\031\031\025\025\025\025\031\025\025\025\025\025\025\031\025\025!\031\031\025\033\033\033\033\033"    
"\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\015\015\015\015\015\015\015\015\015\015\015\015\015\027\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\036\036\036\036\036\036\036\036\036\036\036\036\036\036\031\031\031\031\031\031\031\031\031\031\031\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\015\015\015\015\015\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\015\015\015\015\015\015\033\033\012\012\012\012\012\012\033\033\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\015\015\015\015\015\015\033\033\012\012\012\012\012\012\033\033\015\015\015\015\015\015\015\015\033\012\033\012\033\012\033\012\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\033\015\015\015\015\015\015\015\015\026\026\026\026\026\026\026\026\015\015\015\015\015\015\015\015\026\026\026\026\026\026\026\026\015\015\015\015\015\015\015\015\026\026\026\026\026\026\026\026\015\015\015\015\015\033\015\015\012\012\012\012\026\013\015\013\013\013\015\015\015\033\015\015\012\012\012\012\026\013\013\013\015\015\015\015\033\033\015\015\012\012\012\012\033\013\013\013\015\015\015\015\015\015\015\015\012\012\012\012\012\013\013\013\033\033\015\015\015\033\015\015\012\012\012\012\026\013\013\033\002\002\002\002\002\002\002\002\002\002\002\021%%\021\021\010\010\010\010\010\010\003\003\020\024\005\020\020\024\005\020\003\003\003\003\003\003\003\003&'\021\021\021\021\021\002\003\003\003\003\003\003\003\003\003\020\024\003\003\003\003\014\014\003\003\003\007\005\006\003\003\003\003\003\003\003\003\003\003\003\007\003\014\003\003\003\003\003\003\003\003\003\003\002\021\021\021\021\021\033\021\021\021\021\021\021\021\021\021\021\022\027\033\033\022\022\022\022\022\022\007\007\007\005\006\027\022\022\022\022\022\022\022\022\022\022\007\007\007\005\006\033\027\027\027\027\027\027\027\027\027\027\027\027\027\033\033\033\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\004\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\031\031\031\031\031\031\031\031\031\031\031\031\031\035\035\035\035\031\035\035\035\031\031\031\031\031\031\031\031\031\031\031\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\016\016\012\016\016\016\016\012\016\016\015\012\012\012\015\015\012\012\012\015\016\012\016\016(\012\012\012\012\012\016\016\016\016\016\016\012\016\012\016\012\016\012\012\012\012)\015\012\012\012\012\015\025\025\025\025\015\016\016\015\015\012\012\007\007\007\007\007\012\015\015\015\015\016\007\016\016\015\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022****************++++++++++++++++###\012\015####\022\016\016\033\033\033\033\007\007\007\007\007\016\016\016\016\016\007\007\016\016\016\016\007\016\016\007\016\016\007\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\016\016\007\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\005\006\005\006\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\016\016\016\016\016\016\016\005\006\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016,,,,,,,,,,,,,,,,,,,,,,,,,,--------------------------\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"    
"\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\005\006\005\006\005\006\005\006\005\006\005\006\005\006\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\005\006\005\006\005\006\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016"    
"\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\005\006\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\005\006\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\007\016\016\007\007\007\007\007\007\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\015\012\012\012\015\015\012\015\012\015\012\015\012\012\012\012\015\012\015\015\012\015\015\015\015\015\015\027\027\012\012\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\016\016\016\016\016\016\012\015\012\015\031\031\031\012\015\033\033\033\033\033\003\003\003\003\022\003\003\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\015\033\033\033\033\033\015\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\030\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\031\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036"    
"\003\003\020\024\020\024\003\003\003\020\024\003\020\024\003\003\003\003\003\003\003\003\003\010\003\003\010\003\020\024\003\003\020\024\005\006\005\006\005\006\005\006\003\003\003\003\003.\003\003\003\003\003\003\003\003\003\003\010\010\003\003\003\003\010\003\005\003\003\003\003\003\003\003\003\003\003\003\003\003\016\016\003\003\003\005\006\005\006\005\006\005\006\010\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\002\003\003\003\016\030\025#\005\006\005\006\005\006\005\006\005\006\016\016\005\006\005\006\005\006\005\006\010\005\006\006\016#########\031\031\031\031!!\010\030\030\030\030\030\016\016###\030\025\003\016\016\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\031\031\013\013\030\030\025\010\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\003\030\030\030\025\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\016\016\022\022\022\022\016\016\016\016\016\016\016\016\016\016\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025"    
"\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\022\022\022\022\022\022\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\030\030\030\030\030\003\003"    
"\025\025\025\025\025\025\025\025\025\025\025\025\030\003\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\011\011\011\011\011\011\011\011\011\011\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\025\031\035\035\035\003\036\036\036\036\036\036\036\036\031\031\003\030\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\027\027\036\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025##########\031\031\003\003\003\003\003\003\033\033\033\033\033\033\033\033\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\030\030\030\030\030\030\030\030\030\013\013\012\015\012\015\012\015\012\015\012\015\012\015\012\015\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\027\015\015\015\015\015\015\015\015\012\015\012\015\012\012\015\012\015\012\015\012\015\012\015\030\013\013\012\015\012\015\025\012\015\012\015\015\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\012\012\012\012\015\012\012\012\012\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\015\012\012\012\012\015\012\015\033\033\033\033\033\012\015\033\015\033\015\012\015\012\015\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\030\030\030\012\015\025\027\027\015\025\025\025\025\025\025\025\036\025\025\025\031\025\025\025\025\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\036\036\037\016\016\016\016\031\033\033\033\022\022\022\022\022\022\016\016\004\016\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\003\003\003\003\033\033\033\033\033\033\033\033\037\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\031\036\033\033\033\033\033\033\033\033\003\003\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\025\025\025\025\025\025\003\003\003\025\003\025\025\036\011\011\011\011\011\011\011\011\011\011\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\031\031\031\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\037!\033\033\033\033\033\033\033\033\033\033\033\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\036\036\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\037\037\036\036\036\036\037\037\036\036\037\037!\003\003\003\003\003\003\003\003\003\003\003\003\003\033\030\011\011\011\011\011\011\011\011\011\011\033\033\033\033\003\003\025\025\025\025\025\036\030\025\025\025\025\025\025\025\025\025\011\011\011\011\011\011\011\011\011\011\025\025\025\025\025\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\037\037\036\036\037\037\036\036\033\033\033\033\033\033\033\033\033\025\025\025\036\025\025\025\025\025\025\025\025\036\037\033\033\011\011\011\011\011\011\011\011\011\011\033\033\003\003\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\030\025\025\025\025\025\025\016\016\016\025\037\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\025\036\036\036\025\025\036\036\025\025\025\025\025\036\031\025\031\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\030\003\003\025\025\025\025\025\025\025\025\025\025\025\037\036\036\037\037\003\003\025\030\030\037\031\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\033\033\025\025\025\025\025\025\033\033\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\013\027\027\027\027\015\015\015\015\015\015\015\015\015\030\013\013\033\033\033\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\036\037\037\036\037\037\003!\031\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////"    
"0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\015\015\015\015\015\015\015\033\033\033\033\033\033\033\033\033\033\033\033\015\015\015\015\015\033\033\033\033\033\025\036\025\025\025\025\025\025\025\025\025\025\007\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\033\025\033\025\025\033\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\013\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025111111\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\006\005\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\02511\004\016\016\016\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\003\003\003\003\003\003\003\005\006\003\033\033\033\033\033\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\003\010\010\014\014\005\006\005\006\005\006\005\006\005\006\005\006\005\006\005\006\003\003\005\006\003\003\003\003\014\014\014\003\003\003\033\003\003\003\003\010\005\006\005\006\005\006\003\003\003\007\010\007\007\007\033\003\004\003\003\033\033\033\0331\0251\0251\0331\0251\0251\0251\0251\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\021\033\003\003\003\004\003\003\003\005\006\003\007\003\010\003\003\011\011\011\011\011\011\011\011\011\011\003\003\007\007\007\003\003\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\005\003\006\013\014\013\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\005\007\006\007\005\006\003\005\006\003\003\025\025\025\025\025\025\025\025\025\025\030\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\02522\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\025\025\025\025\025\025\033\033\025\025\025\025\025\025\033\033\025\025\025\025\025\025\033\033\025\025\025\033\033\033\004\004\007\013\016\004\004\033\016\007\007\007\007\016\016\033\033\033\033\033\033\033\033\033\033\021\021\021\016\016\033\033\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033"    
"\003\003\003\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\016\016\016\016\016\016\016\016\016#####################################################\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\022\022\016\016\016\033\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\031\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\022\022\022\022\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025#\025\025\025\025\025\025\025\025#\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\025\025\025\025\025\025\025\025\003#####\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\033\033\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\003\012\012\012\012\012\012\012\012\012\012\012\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\012\012\012\012\012\012\012\033\012\012\033\015\015\015\015\015\015\015\015\015\015\015\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\015\015\015\015\015\015\015\033\015\015\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\027\030\030\027\027\027\033\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\027\033\027\027\027\027\027\027\027\027\027\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\033\033\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\033\033\033\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\003\022\022\022\022\022\022\022\022\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\016\016\022\022\022\022\022\022\022\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\033\033\033\033\033\022\022\022\022\022\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\022\022\022\022\022\022\033\033\033\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\022\022\025\025\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"    
"\025\036\036\036\033\036\036\033\033\033\033\033\036\036\036\036\025\025\025\025\033\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\031\031\031\033\033\033\033\031\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\003\003\003\003\003\003\003\003\003\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\022\022\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\016\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\033\033\033\033\022\022\022\022\022\003\003\003\003\003\003\003\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\003\003\003\003\003\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\022\022\022\022\022\022\022\022\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\022\022\022\022\022\022\022\022\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\033\033\033\033\033\033\033\033\033\033\033\033\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\033\033\033\033\033\033\022\022\022\022\022\022\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\036\036\010\033\033\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\022\022\022\022\022\022\022\022\022\022\025\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\031\031\031\031\031\031\031\031\031\022\022\022\022\003\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\031\031\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\037\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\036\036\036\031\003\003\003\003\003\003\003\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\011\011\011\011\011\011\011\011\011\011\031\025\025\036\036\025\033\033\033\033\033\033\033\033\033\031\031\031\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\037\037\031\031\003\003\021\003\003\003\003\036\033\033\033\033\033\033\033\033\033\033\021\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\036\036\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\037\036\036\036\036\036\036\031\031\033\011\011\011\011\011\011\011\011\011\011\003\003\003\003\025\037\037\025\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\003\003\025\033\033\033\033\033\033\033\033\033\036\036\037\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\036\036\036\036\036\037!\025\025\025\025\003\003\003\003\031\031\031\031\003\037\036\011\011\011\011\011\011\011\011\011\011\025\003\025\003\003\003\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\037\037\036!\031\036\003\003\003\003\003\003\036\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\033\025\033\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\003\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\037\037\037\036\036\036\036\036\036\031\031\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\036\036\037\037\033\025\025\025\025\025\025\025\025\033\033\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\033\031\031\025\037\037\036\037\037\037\037\033\033\037\037\033\033\037\037!\033\033\025\033\033\033\033\033\033\037\033\033\033\033\033\025\025\025\025\025\037\037\033\033\031\031\031\031\031\031\031\033\033\033\031\031\031\031\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\036\036\036\036\037\037\031\036\036\037\031\025\025\025\025\003\003\003\003\003\011\011\011\011\011\011\011\011\011\011\003\003\033\003\031\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\036\036\037\036\037\037\037\037\036\036\037\031\031\025\025\003\025\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\033\033\037\037\037\037\036\036\037\031\031\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\025\025\025\025\036\036\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\036\036\036\036\037\037\036\037\031\036\003\003\003\025\033\033\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\003\003\003\003\003\003\003\003\003\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\037\036\037\037\036\036\036\036\036\036!\031\025\003\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\036\036\036\037\037\036\036\036\036\037\036\036\036\036\031\033\033\033\033\011\011\011\011\011\011\011\011\011\011\022\022\003\003\003\016\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\036\036\036\036\036\037\031\031\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\033\033\025\033\033\025\025\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\037\037\037\033\037\037\033\033\036\036!\031\025\037\025\037\031\003\003\003\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\036\036\036\036\033\033\036\036\037\037\037\037\031\025\003\025\037\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\025\036\036\036\036\036\036\036\036\036\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\036\036\036\036\037\025\036\036\036\036\003\003\003\003\003\003\003\003\031\033\033\033\033\033\033\033\033\025\036\036\036\036\036\036\037\037\036\036\036\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\036\036\036\036\036\036\036\037\031\031\003\003\003\025\003\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\036\036\036\036\036\036\036\033\036\036\036\036\036\036\037\031\025\003\003\003\003\003\033\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\033\037\036\036\036\036\036\036\036\037\036\036\037\036\036\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\036\036\036\036\033\033\033\036\033\036\036\033\036\036\036\031\036\031\031\025\036\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\025\025\025\025\025\025\033\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\037\037\037\037\037\033\036\036\033\037\037\036\037\031\025\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\036\036\037\037\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\004\004\004\004\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033###############################################################################################################\033\003\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\021\021\021\021\021\021\021\021\021\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\003\003\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\031\031\031\031\031\003\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\031\031\031\031\031\003\003\003\003\003\016\016\016\016\030\030\030\030\003\016\033\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\022\022\022\022\022\022\022\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\036\025\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\037\033\033\033\033\033\033\033\036\036\036\036\030\030\030\030\030\030\030\030\030\030\030\030\030\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\030\030\003\030\031\033\033\033\033\033\033\033\033\033\033\033\037\037\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\030\030\030\030\033\030\030\030\030\030\030\030\033\030\030\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\033\033\016\031\036\003\021\021\021\021\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\033\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016!!\031\031\031\016\016\016!!!!!!\021\021\021\021\021\021\021\021\031\031\031\031\031\031\031\031\016\016\031\031\031\031\031\031\031\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\031\031\031\031\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\031\031\031\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\033\012\012\033\033\012\033\033\012\012\033\033\012\012\012\012\033\012\012\012\012\012\012\012\012\015\015\015\015\033\015\033\015\015\015\015\015\015\015\033\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\033\012\012\012\012\033\033\012\012\012\012\012\012\012\012\033\012\012\012\012\012\012\012\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\033\012\012\012\012\033\012\012\012\012\012\033\012\033\033\033\012\012\012\012\012\012\012\033\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\007\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\007\015\015\015\015\015\015\012\015\033\033\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011\011"    
"\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\016\016\016\016\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\016\016\016\016\016\016\016\016\031\016\016\016\016\016\016\016\016\016\016\016\016\016\016\031\016\016\003\003\003\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\031\031\031\031\031\033\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\015\015\015\015\015\015\015\015\015\015\025\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\036\036\036\036\036\036\036\033\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\036\033\033\036\036\036\036\036\036\036\033\036\036\033\036\036\036\036\036\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\031\031\031\031\031\031\031\030\030\030\030\030\030\030\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\025\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\031\031\031\031\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\004\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\033\025\025\025\025\033\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\022\022\022\022\022\022\022\022\022\031\031\031\031\031\031\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\012\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\031\031\031\036\031\031\031\030\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\003\003\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\022\022\022\004\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\016\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\025\025\033\025\033\033\025\033\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\033\025\033\025\033\033\033\033\033\033\025\033\033\033\033\025\033\025\033\025\033\025\025\025\033\025\025\033\025\033\033\025\033\025\033\025\033\025\033\025\033\025\025\033\025\033\033\025\025\025\025\033\025\025\025\025\025\025\025\033\025\025\025\025\033\025\025\025\025\033\025\033\025\025\025\025\025\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\025\025\025\033\025\025\025\025\025\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\007\007\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033"    
"\022\022\022\022\022\022\022\022\022\022\022\022\022\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016,,,,,,,,,,,,,,,,,,,,,,,,,,\016\016\016\016\016\016,,,,,,,,,,,,,,,,,,,,,,,,,,\016\016\016\016\016\016,,,,,,,,,,,,,,,,,,,,,,,,,,\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\013\013\013\013\013\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033"    
"\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\016\016\016\016\016\033\033\033\016\016\016\016\016\033\033\033\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\016\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\011\011\011\011\011\011\011\011\011\011\033\033\033\033\033\033"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025"    
"\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\025\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\021\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033"    
"\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\031\033\033\033\033\033\033\033\033\033\033\033\033\033\033\033\03300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\033\033";

constexpr std::size_t property_index(char32_t code_point)
{
    LEXY_PRECONDITION(code_point <= 0x10FFFF);
    auto high = (code_point >> 8) & 0xFFFF;
    auto low  = code_point & 0xFF;

    auto block_start = static_cast<unsigned char>(block_starts[high]);
    auto block_index = block_start * 256u + low;
    return static_cast<unsigned char>(blocks[block_index]);
}

constexpr lexy::code_point::general_category_t category[] = {lexy::code_point::Cc,lexy::code_point::Cc,lexy::code_point::Zs,lexy::code_point::Po,lexy::code_point::Sc,lexy::code_point::Ps,lexy::code_point::Pe,lexy::code_point::Sm,lexy::code_point::Pd,lexy::code_point::Nd,lexy::code_point::Lu,lexy::code_point::Sk,lexy::code_point::Pc,lexy::code_point::Ll,lexy::code_point::So,lexy::code_point::Lo,lexy::code_point::Pi,lexy::code_point::Cf,lexy::code_point::No,lexy::code_point::Po,lexy::code_point::Pf,lexy::code_point::Lo,lexy::code_point::Lt,lexy::code_point::Lm,lexy::code_point::Lm,lexy::code_point::Mn,lexy::code_point::Mn,lexy::code_point::Cn,lexy::code_point::Lm,lexy::code_point::Me,lexy::code_point::Mn,lexy::code_point::Mc,lexy::code_point::Lo,lexy::code_point::Mc,lexy::code_point::No,lexy::code_point::Nl,lexy::code_point::Mn,lexy::code_point::Cf,lexy::code_point::Zl,lexy::code_point::Zp,lexy::code_point::Sm,lexy::code_point::So,lexy::code_point::Nl,lexy::code_point::Nl,lexy::code_point::So,lexy::code_point::So,lexy::code_point::Lm,lexy::code_point::Cs,lexy::code_point::Co,lexy::code_point::Lo,lexy::code_point::Lm,};

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

constexpr std::uint_least8_t binary_properties[] = {0,1,1,0,0,0,0,0,0,64,108,0,64,116,0,116,0,0,0,64,0,100,100,116,100,64,84,0,20,0,68,68,68,64,64,100,100,2,1,1,96,96,108,116,12,20,4,0,0,4,68,};
} // namespace lexy::_unicode_db

constexpr lexy::code_point::general_category_t lexy::code_point::general_category() const
{
    if (!is_valid())
        return general_category_t::unassigned;

    auto idx = _unicode_db::property_index(_value);
    return _unicode_db::category[idx];
}
#endif

#endif // LEXY_CODE_POINT_HPP_INCLUDED



//=== encoding ===//
namespace lexy::_detail
{
template <typename Encoding>
constexpr std::size_t encode_code_point(code_point cp, typename Encoding::char_type* buffer,
                                        std::size_t size)
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
    {
        LEXY_PRECONDITION(cp.is_ascii());
        LEXY_PRECONDITION(size >= 1);

        *buffer = char(cp.value());
        return 1;
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
    {
        LEXY_PRECONDITION(cp.is_valid());

        // Taken from http://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html.
        if (cp.is_ascii())
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = LEXY_CHAR8_T(cp.value());
            return 1;
        }
        else if (cp.value() <= 0x07'FF)
        {
            LEXY_PRECONDITION(size >= 2);

            auto first  = (cp.value() >> 6) & 0x1F;
            auto second = (cp.value() >> 0) & 0x3F;

            buffer[0] = LEXY_CHAR8_T(0xC0 | first);
            buffer[1] = LEXY_CHAR8_T(0x80 | second);
            return 2;
        }
        else if (cp.value() <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 3);

            auto first  = (cp.value() >> 12) & 0x0F;
            auto second = (cp.value() >> 6) & 0x3F;
            auto third  = (cp.value() >> 0) & 0x3F;

            buffer[0] = LEXY_CHAR8_T(0xE0 | first);
            buffer[1] = LEXY_CHAR8_T(0x80 | second);
            buffer[2] = LEXY_CHAR8_T(0x80 | third);
            return 3;
        }
        else
        {
            LEXY_PRECONDITION(size >= 4);

            auto first  = (cp.value() >> 18) & 0x07;
            auto second = (cp.value() >> 12) & 0x3F;
            auto third  = (cp.value() >> 6) & 0x3F;
            auto fourth = (cp.value() >> 0) & 0x3F;

            buffer[0] = LEXY_CHAR8_T(0xF0 | first);
            buffer[1] = LEXY_CHAR8_T(0x80 | second);
            buffer[2] = LEXY_CHAR8_T(0x80 | third);
            buffer[3] = LEXY_CHAR8_T(0x80 | fourth);
            return 4;
        }
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
    {
        LEXY_PRECONDITION(cp.is_valid());

        if (cp.is_bmp())
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char16_t(cp.value());
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

            buffer[0] = char16_t(0xD800 + high_ten_bits);
            buffer[1] = char16_t(0xDC00 + low_ten_bits);
            return 2;
        }
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf32_encoding>)
    {
        LEXY_PRECONDITION(cp.is_valid());
        LEXY_PRECONDITION(size >= 1);

        *buffer = char32_t(cp.value());
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
    predicate_failure,
};

template <typename Reader>
struct cp_result
{
    lexy::code_point          cp;
    cp_error                  error;
    typename Reader::iterator end;
};

template <typename Reader>
constexpr cp_result<Reader> parse_code_point(Reader reader)
{
    if constexpr (std::is_same_v<typename Reader::encoding, lexy::ascii_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.position()};

        auto cur = reader.peek();
        reader.bump();

        auto cp = lexy::code_point(static_cast<char32_t>(cur));
        if (cp.is_ascii())
            return {cp, cp_error::success, reader.position()};
        else
            return {cp, cp_error::out_of_range, reader.position()};
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf8_encoding>)
    {
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

        auto first = reader.peek();
        if ((first & ~payload_lead1) == pattern_lead1)
        {
            // ASCII character.
            reader.bump();
            return {lexy::code_point(first), cp_error::success, reader.position()};
        }
        else if ((first & ~payload_cont) == pattern_cont)
        {
            return {{}, cp_error::leads_with_trailing, reader.position()};
        }
        else if ((first & ~payload_lead2) == pattern_lead2)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            auto result = char32_t(first & payload_lead2);
            result <<= 6;
            result |= char32_t(second & payload_cont);

            // C0 and C1 are overlong ASCII.
            if (first == 0xC0 || first == 0xC1)
                return {lexy::code_point(result), cp_error::overlong_sequence, reader.position()};
            else
                return {lexy::code_point(result), cp_error::success, reader.position()};
        }
        else if ((first & ~payload_lead3) == pattern_lead3)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            auto result = char32_t(first & payload_lead3);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);

            auto cp = lexy::code_point(result);
            if (cp.is_surrogate())
                return {cp, cp_error::surrogate, reader.position()};
            else if (first == 0xE0 && second < 0xA0)
                return {cp, cp_error::overlong_sequence, reader.position()};
            else
                return {cp, cp_error::success, reader.position()};
        }
        else if ((first & ~payload_lead4) == pattern_lead4)
        {
            reader.bump();

            auto second = reader.peek();
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            auto third = reader.peek();
            if ((third & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            auto fourth = reader.peek();
            if ((fourth & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            auto result = char32_t(first & payload_lead4);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);
            result <<= 6;
            result |= char32_t(fourth & payload_cont);

            auto cp = lexy::code_point(result);
            if (!cp.is_valid())
                return {cp, cp_error::out_of_range, reader.position()};
            else if (first == 0xF0 && second < 0x90)
                return {cp, cp_error::overlong_sequence, reader.position()};
            else
                return {cp, cp_error::success, reader.position()};
        }
        else // FE or FF
        {
            return {{}, cp_error::eof, reader.position()};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf16_encoding>)
    {
        constexpr auto payload1 = 0b0000'0011'1111'1111;
        constexpr auto payload2 = payload1;

        constexpr auto pattern1 = 0b110110 << 10;
        constexpr auto pattern2 = 0b110111 << 10;

        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.position()};

        auto first = char16_t(reader.peek());
        if ((first & ~payload1) == pattern1)
        {
            reader.bump();
            if (reader.peek() == Reader::encoding::eof())
                return {{}, cp_error::missing_trailing, reader.position()};

            auto second = char16_t(reader.peek());
            if ((second & ~payload2) != pattern2)
                return {{}, cp_error::missing_trailing, reader.position()};
            reader.bump();

            // We've got a valid code point.
            auto result = char32_t(first & payload1);
            result <<= 10;
            result |= char32_t(second & payload2);
            result |= 0x10000;
            return {lexy::code_point(result), cp_error::success, reader.position()};
        }
        else if ((first & ~payload2) == pattern2)
        {
            return {{}, cp_error::leads_with_trailing, reader.position()};
        }
        else
        {
            // Single code unit code point; always valid.
            reader.bump();
            return {lexy::code_point(first), cp_error::success, reader.position()};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf32_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.position()};

        auto cur = reader.peek();
        reader.bump();

        auto cp = lexy::code_point(cur);
        if (!cp.is_valid())
            return {cp, cp_error::out_of_range, reader.position()};
        else if (cp.is_surrogate())
            return {cp, cp_error::surrogate, reader.position()};
        else
            return {cp, cp_error::success, reader.position()};
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
        LEXY_PRECONDITION(false);
        break;
    case cp_error::eof:
        // We don't need to do anything to "recover" from EOF.
        break;

    case cp_error::leads_with_trailing:
        // Invalid code unit, consume to recover.
        LEXY_PRECONDITION(result.end == reader.position());
        reader.bump();
        break;

    case cp_error::missing_trailing:
    case cp_error::surrogate:
    case cp_error::out_of_range:
    case cp_error::overlong_sequence:
    case cp_error::predicate_failure:
        // Consume all the invalid code units to recover.
        reader.set_position(result.end);
        break;
    }
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_CODE_POINT_HPP_INCLUDED



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

    template <typename Production>
    class event_handler
    {
    public:
        template <typename Error>
        constexpr void on(match_handler& handler, parse_events::error, Error&&)
        {
            handler._failed = true;
        }

        template <typename Event, typename... Args>
        constexpr void on(match_handler&, Event, const Args&...)
        {}
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr bool get_result_void(bool rule_parse_result) &&
    {
        return rule_parse_result && !_failed;
    }

private:
    bool _failed;
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    auto reader = input.reader();
    return lexy::do_action<Production>(match_handler(), no_parse_state, reader);
}

template <typename Production, typename Input, typename State>
constexpr bool match(const Input& input, const State& state)
{
    auto reader = input.reader();
    return lexy::do_action<Production>(match_handler(), &state, reader);
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

template <typename Derived, typename BranchKind = branch_base>
struct token_base : _token_base, BranchKind
{
    using token_type = Derived;

    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr auto try_parse(const Context&, const Reader& reader)
        {
            lexy::token_parser_for<Derived, Reader> parser(reader);
            auto                                    result = parser.try_parse(reader);
            end                                            = parser.end;
            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::token{}, Derived{}, reader.position(), end);
            reader.set_position(end);
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
                parser.report_error(context, reader);

                if (begin != parser.end)
                    context.on(_ev::token{}, lexy::error_token_kind, reader.position(), parser.end);
                reader.set_position(parser.end);

                return false;
            }
        }

        context.on(_ev::token{}, Derived{}, begin, parser.end);
        reader.set_position(parser.end);

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

template <auto Kind, typename Token>
struct _tokk : token_base<_tokk<Kind, Token>>
{
    template <typename Reader>
    using tp = lexy::token_parser_for<Token, Reader>;
};

template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>>
{
    template <typename Reader>
    struct tp : lexy::token_parser_for<Token, Reader>
    {
        constexpr explicit tp(const Reader& reader) : lexy::token_parser_for<Token, Reader>(reader)
        {}

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            // Report a different error.
            auto err = lexy::error<Reader, Tag>(reader.position(), this->end);
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
        static constexpr auto max_recursion_depth = 0;
        static constexpr auto rule                = Rule{};
    };

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // We match a dummy production that only consists of the rule.
            auto success
                = lexy::do_action<_production>(lexy::match_handler(), lexy::no_parse_state, reader);
            end = reader.position();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::missing_token>(reader.position(), end);
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
template <char32_t Cp>
struct _cpl : token_base<_cpl<Cp>>
{
    template <typename Encoding>
    struct _literal_t
    {
        using char_type = typename Encoding::char_type;
        char_type   str[4];
        std::size_t length;

        constexpr _literal_t()
        : str{}, length(lexy::_detail::encode_code_point<Encoding>(lexy::code_point(Cp), str, 4))
        {}
    };
    template <typename Reader>
    static constexpr _literal_t<typename Reader::encoding> _literal
        = _literal_t<typename Reader::encoding>{};

    template <typename Reader,
              typename Indices = lexy::_detail::make_index_sequence<_literal<Reader>.length>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            constexpr auto str = _literal<Reader>.str;

            auto result
                // Compare each code unit, bump on success, cancel on failure.
                = ((reader.peek() == lexy::_char_to_int_type<typename Reader::encoding>(str[Idx])
                        ? (reader.bump(), true)
                        : false)
                   && ...);
            end = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            constexpr auto str = _literal<Reader>.str;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, this->end);
            auto err   = lexy::error<Reader, lexy::expected_literal>(begin, str, index);
            context.on(_ev::error{}, err);
        }
    };
};

template <typename Predicate>
struct _cp : token_base<_cp<Predicate>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;
        lexy::_detail::cp_error   ec;

        constexpr explicit tp(const Reader& reader)
        : end(reader.position()), ec(lexy::_detail::cp_error::success)
        {}

        constexpr bool try_parse(Reader reader)
        {
            using lexy::_detail::cp_error;

            // Parse one code point.
            auto result = lexy::_detail::parse_code_point(reader);
            end         = result.end;
            ec          = result.error;

            if (ec != cp_error::success)
                return false;

            // Check whether it matches the predicate.
            if constexpr (!std::is_void_v<Predicate>)
            {
                if (!Predicate()(result.cp))
                {
                    ec = cp_error::predicate_failure;
                    return false;
                }
            }
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using lexy::_detail::cp_error;

            if (ec == cp_error::predicate_failure)
            {
                constexpr auto name = lexy::_detail::type_name<Predicate>();

                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), name);
                context.on(_ev::error{}, err);
            }
            else
            {
                constexpr auto name = [] {
                    using encoding = typename Reader::encoding;
                    if constexpr (std::is_same_v<encoding, lexy::ascii_encoding>)
                        return "ASCII.code-point";
                    else if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
                        return "UTF-8.code-point";
                    else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
                        return "UTF-16.code-point";
                    else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
                        return "UTF-32.code-point";
                    else
                        return "code-point";
                }();

                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), name);
                context.on(_ev::error{}, err);
            }
        }
    };

    template <char32_t CodePoint>
    constexpr auto lit() const
    {
        static_assert(lexy::code_point(CodePoint).is_scalar());
        return _cpl<CodePoint>{};
    }

    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _cp<P>{};
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
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp<void>{};
} // namespace lexyd

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

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
        return _range_reader<Encoding>(_begin, _end);
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
        using encoding = Encoding;
        using iterator = const typename Encoding::char_type*;
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
    if constexpr (std::is_same_v<encoding, lexy::ascii_encoding> //
                  || std::is_same_v<encoding, lexy::default_encoding>)
    {
        auto count = 0u;
        for (char c : lexeme)
        {
            // If the character is in fact ASCII, visualize the code point.
            // Otherwise, visualize as byte.
            if (lexy::_is_ascii(c))
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
            if (auto result = lexy::_detail::parse_code_point(reader);
                result.error == lexy::_detail::cp_error::eof)
            {
                // No more code points in the lexeme, finish.
                break;
            }
            else if (result.error == lexy::_detail::cp_error::success)
            {
                // Consume and visualize.
                reader.set_position(result.end);
                out = visualize_to(out, result.cp, opts);
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
                    if constexpr (std::is_same_v<encoding, lexy::utf8_encoding>)
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




namespace lexy::_detail
{
template <typename Reader>
constexpr bool match_newline(Reader& reader)
{
    using encoding = typename Reader::encoding;

    if (reader.peek() == lexy::_char_to_int_type<encoding>('\n'))
    {
        reader.bump();
        return true;
    }
    else if (reader.peek() == lexy::_char_to_int_type<encoding>('\r'))
    {
        reader.bump();
        if (reader.peek() == lexy::_char_to_int_type<encoding>('\n'))
        {
            reader.bump();
            return true;
        }
    }

    return false;
}
} // namespace lexy::_detail

namespace lexyd
{
struct _nl : token_base<_nl>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = lexy::_detail::match_newline(reader);
            end         = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), "newline");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

namespace lexyd
{
struct _eol : token_base<_eol>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = (reader.peek() == Reader::encoding::eof())
                          || lexy::_detail::match_newline(reader);
            end = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), "EOL");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches the end of line (EOF or newline).
constexpr auto eol = _eol{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_eol> = lexy::eol_token_kind;
}

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED




namespace lexy_ext
{
// Fake token that counts code units without verification.
struct _unchecked_code_unit
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            if (reader.peek() == Reader::encoding::eof())
                return false;

            reader.bump();
            end = reader.position();
            return true;
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
            auto eol = _reader.position();
            for (auto reader = _reader; true; reader.bump())
            {
                eol = reader.position();
                if (reader.peek() == decltype(reader)::encoding::eof()
                    || lexy::try_match_token(TokenLine{}, reader))
                    break;
            }

            return {_reader.position(), eol};
        }

        /// The newline after the line, if there is any.
        constexpr lexy::lexeme_for<Input> newline() const
        {
            // Advance to EOl.
            for (auto reader = _reader; true; reader.bump())
            {
                auto pos = reader.position();
                if (reader.peek() == decltype(reader)::encoding::eof())
                    return {pos, pos};
                else if (lexy::try_match_token(TokenLine{}, reader))
                    return {pos, reader.position()};
            }

            return {}; // unreachable
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
            if (reader.position() == pos)
            {
                // We found the position of the error.
                break;
            }
            else if (lexy::token_parser_for<TokenLine, decltype(reader)> nl(reader);
                     nl.try_parse(reader))
            {
                LEXY_ASSERT(reader.position() != nl.end, "TokenLine must consume input");
                reader.bump();

                // Check whether our location points inside the newline.
                auto found = false;
                while (reader.position() != nl.end)
                {
                    found |= reader.position() == pos;
                    reader.bump();
                }

                // We pretend the location is at the beginning of the newline.
                if (found)
                    break;

                // We're at a new line.
                ++cur_line;
                cur_column = 1;
                line_start = reader;
            }
            else if (lexy::token_parser_for<TokenColumn, decltype(reader)> column(reader);
                     column.try_parse(reader))
            {
                LEXY_ASSERT(reader.position() != column.end, "TokenColumn must consume input");
                reader.bump();

                // Check whether our location points inside a column.
                auto found = false;
                while (reader.position() != column.end)
                {
                    found |= reader.position() == pos;
                    reader.bump();
                }

                // We pretend the location is at the beginning of the column.
                if (found)
                    break;

                // Next column.
                ++cur_column;
            }
            else if (reader.peek() == decltype(reader)::encoding::eof())
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

    template <typename Location, typename Production>
    void write_production_start(const Location& loc, Production)
    {
        _last_token.reset();
        if (_cur_depth <= _opts.max_tree_depth)
        {
            write_prefix(loc, prefix::event);

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
    }

    template <typename Location, typename Production, typename Reader>
    void write_token(const Location& loc, Production production, lexy::token_kind<TokenKind> kind,
                     lexy::lexeme<Reader> lexeme)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        if (_last_token.merge(production, kind))
        {
            _out = visualize_to(_out, lexeme, _opts | visualize_space);
        }
        else
        {
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

        _last_token.update(kind);
    }

    template <typename Location, typename Reader>
    void write_backtrack(const Location& loc, lexy::lexeme<Reader> lexeme)
    {
        _last_token.reset();
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
        _last_token.reset();
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
    }

    template <typename Location>
    void write_recovery_start(const Location& loc)
    {
        _last_token.reset();
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
    void write_debug(const Location& loc, const char* str)
    {
        _last_token.reset();
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
        _last_token.reset();

        if (_cur_depth <= _opts.max_tree_depth)
            write_prefix(loc, prefix::finish);
        --_cur_depth;
    }
    template <typename Location>
    void write_cancel(const Location& loc)
    {
        _last_token.reset();

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
        _out = _detail::write_format(_out, "%2zu:%3zu", loc.line_nr(), loc.column_nr());
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

    struct last_token_info
    {
        bool                        first_token;
        lexy::token_kind<TokenKind> kind;

        last_token_info() : first_token(true) {}

        template <typename Production>
        bool merge(Production, lexy::token_kind<TokenKind> new_kind) const
        {
            if (first_token || kind != new_kind)
                return false;

            // We can merge inside a token production or if it's an error token.
            return lexy::is_token_production<Production> || kind == lexy::error_token_kind;
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

    OutputIt              _out;
    visualization_options _opts;

    std::size_t     _cur_depth;
    last_token_info _last_token;
};
} // namespace lexy::_detail

namespace lexy
{
template <typename OutputIt, typename Input, typename TokenKind = void>
class trace_handler
{
    using location_finder = lexy_ext::input_location_finder<Input>;
    using location        = typename location_finder::location;

public:
    explicit trace_handler(OutputIt out, const Input& input,
                           visualization_options opts = {}) noexcept
    : _writer(out, opts), _locations(input), _anchor(_locations.beginning())
    {
        LEXY_PRECONDITION(opts.max_tree_depth <= visualization_options::max_tree_depth_limit);
    }

    template <typename Production>
    class event_handler
    {
        using iterator = typename lexy::input_reader<Input>::iterator;

    public:
        void on(trace_handler& handler, parse_events::production_start, iterator pos)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_production_start(loc, Production{});

            // All events for the production are after the initial event.
            _previous_anchor.emplace(handler._anchor);
            handler._anchor = loc;
        }
        void on(trace_handler& handler, parse_events::production_finish, iterator pos)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_finish(loc);
        }
        void on(trace_handler& handler, parse_events::production_cancel, iterator pos)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_cancel(loc);

            // We've backtracked, so we need to restore the anchor.
            handler._anchor = *_previous_anchor;
        }

        template <typename TK>
        void on(trace_handler& handler, parse_events::token, TK kind, iterator begin, iterator end)
        {
            auto loc = handler.find_location(begin);
            handler._writer.write_token(loc, Production{}, token_kind<TokenKind>(kind),
                                        lexeme_for<Input>(begin, end));
        }
        void on(trace_handler& handler, parse_events::backtracked, iterator begin, iterator end)
        {
            auto loc = handler.find_location(begin);
            handler._writer.write_backtrack(loc, lexeme_for<Input>(begin, end));
        }

        template <typename Error>
        void on(trace_handler& handler, parse_events::error, const Error& error)
        {
            auto loc = handler.find_location(error.position());
            handler._writer.write_error(loc, error);
        }

        void on(trace_handler& handler, parse_events::recovery_start, iterator pos)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_recovery_start(loc);
        }
        void on(trace_handler& handler, parse_events::recovery_finish, iterator pos)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_finish(loc);
        }
        void on(trace_handler& handler, parse_events::recovery_cancel, iterator pos)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_cancel(loc);
        }

        void on(trace_handler& handler, parse_events::debug, iterator pos, const char* str)
        {
            auto loc = handler.find_location(pos);
            handler._writer.write_debug(loc, str);
        }

    private:
        // The beginning of the previous production.
        // If the current production gets canceled, it needs to be restored.
        _detail::lazy_init<location> _previous_anchor;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr OutputIt get_result_void(bool) &&
    {
        return LEXY_MOV(_writer).finish();
    }

private:
    location find_location(typename lexy::input_reader<Input>::iterator pos)
    {
        return _locations.find(pos, _anchor);
    }

    _detail::trace_writer<OutputIt, TokenKind> _writer;

    location_finder _locations;
    location        _anchor;
};

template <typename Production, typename TokenKind = void, typename OutputIt, typename Input>
OutputIt trace_to(OutputIt out, const Input& input, visualization_options opts = {})
{
    auto reader = input.reader();
    return lexy::do_action<Production>(trace_handler<OutputIt, Input, TokenKind>(out, input, opts),
                                       no_parse_state, reader);
}

template <typename Production, typename TokenKind = void, typename OutputIt, typename Input,
          typename ParseState>
OutputIt trace_to(OutputIt out, const Input& input, const ParseState& state,
                  visualization_options opts = {})
{
    auto reader = input.reader();
    return lexy::do_action<Production>(trace_handler<OutputIt, Input, TokenKind>(out, input, opts),
                                       &state, reader);
}

template <typename Production, typename TokenKind = void, typename Input>
void trace(std::FILE* file, const Input& input, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, opts);
}
template <typename Production, typename TokenKind = void, typename Input, typename State>
void trace(std::FILE* file, const Input& input, const State& state, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, state, opts);
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

#ifndef LEXY_DETAIL_TRIE_HPP_INCLUDED
#define LEXY_DETAIL_TRIE_HPP_INCLUDED

#include <climits>




namespace lexy::_detail
{
template <typename CharT, std::size_t NodeCount>
struct _trie
{
    static_assert(NodeCount > 0);

    static auto _index_type()
    {
        // We need to store:
        // * A value for every node, which is the index of the string.
        //   There are at most NodeCount - 1 strings (root node plus one for every one character
        //   string). One value must be free for the invalid value.
        // * An index to a node in the range [0, NodeCount).
        // * An index to a transition, there are at most NodeCount - 1 transitions in a tree.
        // As such, we need values in [0, ..., NodeCount].
        if constexpr (NodeCount <= UCHAR_MAX)
            return static_cast<unsigned char>(0);
        else if constexpr (NodeCount <= USHRT_MAX)
            return static_cast<unsigned short>(0);
        else if constexpr (NodeCount <= UINT_MAX)
            return static_cast<unsigned int>(0);
        else
            return std::size_t(0);
    }
    using index_type = decltype(_index_type());

    static constexpr std::size_t invalid_value = NodeCount;

    LEXY_CONSTEVAL bool empty() const
    {
        return NodeCount == 1 && _node_value[0] == invalid_value;
    }
    LEXY_CONSTEVAL bool accepts_empty() const
    {
        return _node_value[0] != invalid_value;
    }

    // The index of the string.
    LEXY_CONSTEVAL std::size_t node_value(std::size_t node) const
    {
        return _node_value[node];
    }

    LEXY_CONSTEVAL std::size_t transition_count(std::size_t node) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        auto end   = _node_transition_idx[node];
        return std::size_t(end - begin);
    }

    template <typename Encoding>
    LEXY_CONSTEVAL auto transition_char(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : std::size_t(_node_transition_idx[node - 1]);
        return lexy::_char_to_int_type<Encoding>(_transition_char[begin + transition]);
    }
    LEXY_CONSTEVAL std::size_t transition_next(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : std::size_t(_node_transition_idx[node - 1]);
        return _transition_node[begin + transition];
    }

    // Arrays indexed by nodes.
    // The node has the transitions in the range [_node_transition_idx[node] - 1,
    // _node_transition_idx[node]].
    index_type _node_value[NodeCount];
    index_type _node_transition_idx[NodeCount];

    // Shared array for all transitions.
    index_type _transition_node[NodeCount == 1 ? 1 : NodeCount - 1];
    CharT      _transition_char[NodeCount == 1 ? 1 : NodeCount - 1];
};

template <typename CharT, typename... Strings, std::size_t... Idxs>
LEXY_CONSTEVAL auto _make_trie(lexy::_detail::index_sequence<Idxs...>)
{
    // We can estimate the number of nodes in the trie by adding all strings together.
    // This is the worst case where the strings don't share any nodes.
    // The plus one comes from the additional root node.
    constexpr auto node_count_upper_bound = (Strings::size + ... + 1);

    // We cannot construct the `_trie` directly as we don't know how many transitions each node has.
    // So we use this temporary representation using an adjacency matrix.
    struct builder_t
    {
        std::size_t node_count = 1;

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

                    cur_node = next_node;
                }
            }

            // A trie (alternative, symbol table) contains duplicate values.
            LEXY_PRECONDITION(node_value[cur_node] == std::size_t(-1));
            node_value[cur_node] = value;
        }
    };
    // We build the trie by inserting all strings.
    constexpr auto builder = [] {
        builder_t builder;
        (builder.insert(Idxs, Strings::template c_str<CharT>, Strings::size), ...);
        return builder;
    }();

    // Now we also now the exact number of nodes in the trie.
    _trie<CharT, builder.node_count> result{};
    using index_type = typename decltype(result)::index_type;

    // Translate the adjacency matrix representation into the actual trie representation.
    auto transition_idx = 0u;
    for (auto node = 0u; node != builder.node_count; ++node)
    {
        auto value = builder.node_value[node];
        result._node_value[node]
            = value == std::size_t(-1) ? result.invalid_value : index_type(value);

        for (auto next_node = node + 1; next_node != builder.node_count; ++next_node)
            if (auto c = builder.node_transition[node][next_node])
            {
                // We've found a transition, add it to the shared transition array.
                result._transition_node[transition_idx] = index_type(next_node);
                result._transition_char[transition_idx] = c;
                ++transition_idx;
            }

        // The node transition end at the current transition index.
        result._node_transition_idx[node] = index_type(transition_idx);
    }

    return result;
}

/// A trie containing the given strings.
template <typename CharT, typename... Strings>
constexpr auto trie
    = _make_trie<CharT, Strings...>(lexy::_detail::index_sequence_for<Strings...>{});

template <const auto& Trie, typename Reader>
struct trie_parser
{
    // 0 ... number-of-transitions-of-Node
    template <std::size_t Node>
    using transition_sequence = lexy::_detail::make_index_sequence<Trie.transition_count(Node)>;

    template <std::size_t Node, typename Transitions = transition_sequence<Node>>
    struct handle_node;
    template <std::size_t Node, std::size_t... Transitions>
    struct handle_node<Node, lexy::_detail::index_sequence<Transitions...>>
    {
        static constexpr std::size_t parse(Reader& reader)
        {
            using encoding = typename Reader::encoding;

            constexpr auto cur_value = Trie.node_value(Node);
            auto           cur_pos   = reader.position();

            // Compute a longer match.
            auto next_value = [&] {
                auto result = Trie.invalid_value;

                // Find a transition that would match.
                auto next_char = reader.peek();
                (void)((next_char == Trie.template transition_char<encoding>(Node, Transitions)
                        ? // We did find a transition that matches, consume the character and take.
                        reader.bump(),
                        result
                        = handle_node<Trie.transition_next(Node, Transitions)>::parse(reader),
                        true // short-circuit
                        : false)
                       || ...);

                return result;
            }();

            // We prefer to return a longer match.
            if (next_value != Trie.invalid_value)
                return next_value;

            // We don't have a longer match, undo everything consumed and return our match.
            reader.set_position(cur_pos);
            return cur_value;
        }
    };
    template <std::size_t Node>
    struct handle_node<Node, lexy::_detail::index_sequence<>>
    {
        static constexpr std::size_t parse(Reader&)
        {
            // We don't have any transitions, so we always return the current value.
            return Trie.node_value(Node);
        }
    };

    static constexpr std::size_t parse([[maybe_unused]] Reader& reader)
    {
        if constexpr (Trie.empty())
            return Trie.invalid_value;
        else
            // We start parsing at the root node.
            return handle_node<0>::parse(reader);
    }

    static constexpr auto try_match([[maybe_unused]] Reader& reader)
    {
        if constexpr (Trie.empty())
            return std::false_type{};
        else
            return parse(reader) != Trie.invalid_value;
    }
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TRIE_HPP_INCLUDED


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED






namespace lexyd
{
template <typename CharT, CharT... C>
struct _lit
: token_base<_lit<CharT, C...>,
             std::conditional_t<sizeof...(C) == 0, unconditional_branch_base, branch_base>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr auto try_parse(Reader reader)
        {
            if constexpr (sizeof...(C) == 0)
            {
                end = reader.position();
                return std::true_type{};
            }
            else
            {
                auto result
                    // Compare each code unit, bump on success, cancel on failure.
                    = ((reader.peek() == lexy::_char_to_int_type<typename Reader::encoding>(C)
                            ? (reader.bump(), true)
                            : false)
                       && ...);
                end = reader.position();
                return result;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using char_type    = typename Reader::encoding::char_type;
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<char_type>;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, this->end);
            auto err   = lexy::error<Reader, lexy::expected_literal>(begin, str, index);
            context.on(_ev::error{}, err);
        }
    };
};

template <auto C>
constexpr auto lit_c = _lit<std::decay_t<decltype(C)>, C>{};

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

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED



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

namespace lexy::_detail
{
template <typename... Tokens>
struct build_token_trie;

template <>
struct build_token_trie<>
{
    template <typename... Strings>
    static constexpr auto trie()
    {
        if constexpr (sizeof...(Strings) == 0)
            return lexy::_detail::trie<char>;
        else
            return lexy::_detail::trie<std::common_type_t<typename Strings::char_type...>,
                                       Strings...>;
    }

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = Templ<Tokens...>;
};

template <typename H, typename... T>
struct build_token_trie<H, T...>
{
    template <typename... Strings>
    static constexpr auto trie()
    {
        return build_token_trie<T...>::template trie<Strings...>();
    }

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens =
        typename build_token_trie<T...>::template other_tokens<Templ, Tokens..., H>;
};
template <typename CharT, CharT... C, typename... T>
struct build_token_trie<lexyd::_lit<CharT, C...>, T...>
{
    template <typename... Strings>
    static constexpr auto trie()
    {
        return build_token_trie<T...>::template trie<Strings..., type_string<CharT, C...>>();
    }

    template <template <typename...> typename Templ, typename... Tokens>
    using other_tokens = typename build_token_trie<T...>::template other_tokens<Templ, Tokens...>;
};
} // namespace lexy::_detail

namespace lexyd
{
template <typename... Tokens>
struct _malt : token_base<_malt<Tokens...>>
{
    template <typename Reader,
              typename Indices = lexy::_detail::make_index_sequence<sizeof...(Tokens)>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr auto try_parse([[maybe_unused]] const Reader& reader)
        {
            if constexpr (sizeof...(Tokens) == 0)
                return std::false_type{};
            else
            {
                auto result = false;
                auto impl   = [&](auto token, Reader local_reader) {
                    // Try to match the current token.
                    if (!lexy::try_match_token(token, local_reader))
                        return;

                    // Update end to longest match.
                    end    = lexy::_detail::max_range_end(reader.position(), end,
                                                       local_reader.position());
                    result = true;
                };

                // Need to try everything.
                (impl(Tokens{}, reader), ...);
                return result;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::exhausted_alternatives>(reader.position());
            context.on(_ev::error{}, err);
        }
    };
};

template <typename... Tokens>
struct _alt : token_base<_alt<Tokens...>>
{
    using _builder              = lexy::_detail::build_token_trie<typename Tokens::token_type...>;
    static constexpr auto _trie = _builder::trie();

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(const Reader& reader)
        {
            lexy::token_parser_for<typename _builder::template other_tokens<_malt>, Reader>
                manual_parser(reader);

            // We check the trie as a baseline.
            // This gives us a first end position.
            if (auto trie_reader = reader;
                lexy::_detail::trie_parser<_trie, Reader>::try_match(trie_reader))
            {
                end = trie_reader.position();

                if (trie_reader.peek() == Reader::encoding::eof())
                    // Exit early, there can't be a longer match.
                    return true;

                // Check the remaining tokens to see if we have a longer match.
                if (manual_parser.try_parse(reader))
                    end = lexy::_detail::max_range_end(reader.position(), end, manual_parser.end);

                return true;
            }
            else
            {
                // Check the remaining tokens only.
                auto result = manual_parser.try_parse(reader);
                end         = manual_parser.end;
                return result;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::exhausted_alternatives>(reader.position());
            context.on(_ev::error{}, err);
        }
    };
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




namespace lexyd
{
struct _any : token_base<_any, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            while (reader.peek() != Reader::encoding::eof())
                reader.bump();
            end = reader.position();
            return {};
        }
    };
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

#include <climits>



namespace lexy::_detail
{
class ascii_table_t
{
public:
    enum category
    {
        control,
        space,
        alpha,
        alpha_underscore,
        digit, // 0-9 only
        hex_lower,
        hex_upper,
        punct,

        _count,
    };
    static_assert(_count <= CHAR_BIT);

    LEXY_CONSTEVAL ascii_table_t() : _table{}
    {
        for (char c = 0x00; c <= 0x1F; ++c)
            insert(c, control);
        insert(0x7F, control);

        insert(' ', space);
        insert('\t', space);
        insert('\n', space);
        insert('\r', space);
        insert('\f', space);
        insert('\v', space);

        for (auto c = 'A'; c <= 'Z'; ++c)
        {
            insert(c, alpha);
            insert(c, alpha_underscore);
        }
        for (auto c = 'a'; c <= 'z'; ++c)
        {
            insert(c, alpha);
            insert(c, alpha_underscore);
        }
        insert('_', alpha_underscore);

        for (auto c = '0'; c <= '9'; ++c)
        {
            insert(c, digit);
            insert(c, hex_lower);
            insert(c, hex_upper);
        }
        for (auto c = 'A'; c <= 'F'; ++c)
            insert(c, hex_upper);
        for (auto c = 'a'; c <= 'f'; ++c)
            insert(c, hex_lower);

        insert('!', punct);
        insert('"', punct);
        insert('#', punct);
        insert('$', punct);
        insert('%', punct);
        insert('&', punct);
        insert('\'', punct);
        insert('(', punct);
        insert(')', punct);
        insert('*', punct);
        insert('+', punct);
        insert(',', punct);
        insert('-', punct);
        insert('.', punct);
        insert('/', punct);
        insert(':', punct);
        insert(';', punct);
        insert('<', punct);
        insert('=', punct);
        insert('>', punct);
        insert('?', punct);
        insert('@', punct);
        insert('[', punct);
        insert('\\', punct);
        insert(']', punct);
        insert('^', punct);
        insert('_', punct);
        insert('`', punct);
        insert('{', punct);
        insert('|', punct);
        insert('}', punct);
        insert('~', punct);
    }

    template <typename Encoding, category... Cats>
    constexpr bool contains(typename Encoding::int_type i) const
    {
        constexpr auto mask = ((1 << Cats) | ...);

        if (i < lexy::_char_to_int_type<Encoding>(0x00)
            || lexy::_char_to_int_type<Encoding>(0x7F) < i)
            return false;

        // NOLINTNEXTLINE: We've checked that we're positive in the condition above.
        auto index = static_cast<std::size_t>(i);
        return (_table[index] & mask) != 0;
    }

private:
    constexpr void insert(char c, category cat)
    {
        auto as_unsigned = static_cast<unsigned char>(c);
        LEXY_PRECONDITION(as_unsigned <= 0x7F);

        _table[as_unsigned] = static_cast<unsigned char>(_table[as_unsigned] | 1 << cat);
    }

    unsigned char _table[0x80];
};

inline constexpr ascii_table_t ascii_table = {};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ASCII_TABLE_HPP_INCLUDED





namespace lexyd::ascii
{
template <typename Derived>
struct _ascii : token_base<Derived>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            if (!Derived::template ascii_match<typename Reader::encoding>(reader.peek()))
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                      Derived::ascii_name());
            context.on(_ev::error{}, err);
        }
    };
};

//=== control ===//
struct _control : _ascii<_control>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.control";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::control>(i);
    }
};
inline constexpr auto control = _control{};

//=== whitespace ===//
struct _blank : _ascii<_blank>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.blank";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>(' ')
               || i == lexy::_char_to_int_type<Encoding>('\t');
    }
};
inline constexpr auto blank = _blank{};

struct _newline : _ascii<_newline>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.newline";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>('\n')
               || i == lexy::_char_to_int_type<Encoding>('\r');
    }
};
inline constexpr auto newline = _newline{};

struct _other_space : _ascii<_other_space>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.other-space";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return i == lexy::_char_to_int_type<Encoding>('\f')
               || i == lexy::_char_to_int_type<Encoding>('\v');
    }
};
inline constexpr auto other_space = _other_space{};

struct _space : _ascii<_space>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.space";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::space>(i);
    }
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : _ascii<_lower>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.lower";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('a') <= i
               && i <= lexy::_char_to_int_type<Encoding>('z');
    }
};
inline constexpr auto lower = _lower{};

struct _upper : _ascii<_upper>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.upper";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('A') <= i
               && i <= lexy::_char_to_int_type<Encoding>('Z');
    }
};
inline constexpr auto upper = _upper{};

struct _alpha : _ascii<_alpha>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.alpha";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::alpha>(i);
    }
};
inline constexpr auto alpha = _alpha{};

struct _alphau : _ascii<_alphau>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.alpha-underscore";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::alpha_underscore>(i);
    }
};
inline constexpr auto alpha_underscore = _alphau{};

//=== digit ===//
struct _digit : _ascii<_digit>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.digit";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::digit>(i);
    }
};
inline constexpr auto digit = _digit{};

struct _alnum : _ascii<_alnum>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.alpha-digit";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::alpha, ascii_table_t::digit>(i);
    }
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = _alnum{};

struct _word : _ascii<_word>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.word";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table
            .contains<Encoding, ascii_table_t::alpha_underscore, ascii_table_t::digit>(i);
    }
};
inline constexpr auto word                   = _word{};
inline constexpr auto alpha_digit_underscore = _word{};

//=== punct ===//
struct _punct : _ascii<_punct>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.punct";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        using namespace lexy::_detail;
        return ascii_table.contains<Encoding, ascii_table_t::punct>(i);
    }
};
inline constexpr auto punct = _punct{};

//=== categories ===//
struct _graph : _ascii<_graph>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.graph";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('\x21') <= i
               && i <= lexy::_char_to_int_type<Encoding>('\x7E');
    }
};
inline constexpr auto graph = _graph{};

struct _print : _ascii<_print>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII.print";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('\x20') <= i
               && i <= lexy::_char_to_int_type<Encoding>('\x7E');
    }
};
inline constexpr auto print = _print{};

struct _char : _ascii<_char>
{
    static LEXY_CONSTEVAL auto ascii_name()
    {
        return "ASCII";
    }

    template <typename Encoding>
    static constexpr bool ascii_match(typename Encoding::int_type i)
    {
        return lexy::_char_to_int_type<Encoding>('\x00') <= i
               && i <= lexy::_char_to_int_type<Encoding>('\x7F');
    }
};
inline constexpr auto character = _char{};
} // namespace lexyd::ascii

namespace lexyd::ascii
{
template <char... C>
struct _alt : token_base<_alt<C...>>
{
    static_assert(sizeof...(C) > 0);

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto cur = reader.peek();
            if (((cur != lexy::_char_to_int_type<typename Reader::encoding>(C)) && ...))
                return false;

            reader.bump();
            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            constexpr auto str = lexy::_detail::type_string<char, C...>::template c_str<char>;

            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), str);
            context.on(_ev::error{}, err);
        }
    };
};

template <typename CharT, CharT... C>
struct _one_of
{
    static_assert((std::is_same_v<CharT, char> && ... && lexy::_is_ascii(C)),
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED





namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom_impl
{
    static_assert(Endianness != lexy::encoding_endianness::bom,
                  "bom with BOM-endianness doesn't make sense");

    static constexpr auto name = "";

    using literal     = _lit<unsigned char>;
    using branch_base = unconditional_branch_base;
};
template <lexy::encoding_endianness DontCare>
struct _bom_impl<lexy::utf8_encoding, DontCare>
{
    static constexpr auto name = "BOM.UTF-8";

    using literal     = decltype(lit_b<0xEF, 0xBB, 0xBF>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-16-LE";

    using literal     = decltype(lit_b<0xFF, 0xFE>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf16_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-16-BE";

    using literal     = decltype(lit_b<0xFE, 0xFF>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::little>
{
    static constexpr auto name = "BOM.UTF-32-LE";

    using literal     = decltype(lit_b<0xFF, 0xFE, 0x00, 0x00>);
    using branch_base = lexyd::branch_base;
};
template <>
struct _bom_impl<lexy::utf32_encoding, lexy::encoding_endianness::big>
{
    static constexpr auto name = "BOM.UTF-32-BE";

    using literal     = decltype(lit_b<0x00, 0x00, 0xFE, 0xFF>);
    using branch_base = lexyd::branch_base;
};

template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom
: token_base<_bom<Encoding, Endianness>, typename _bom_impl<Encoding, Endianness>::branch_base>
{
    using _impl = _bom_impl<Encoding, Endianness>;

    template <typename Reader>
    struct tp : lexy::token_parser_for<typename _impl::literal, Reader>
    {
        constexpr explicit tp(const Reader& reader)
        : lexy::token_parser_for<typename _impl::literal, Reader>(reader)
        {}

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err
                = lexy::error<Reader, lexy::expected_char_class>(reader.position(), _impl::name);
            context.on(_ev::error{}, err);
        }
    };
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
    using p = NextParser;
};
template <typename H, typename... T>
struct _seq_impl<H, T...>
{
    // We parse Head and then seq<Tail...>.
    template <typename NextParser>
    using p = lexy::parser_for<H, lexy::parser_for<_seq_impl<T...>, NextParser>>;
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);

    template <typename NextParser>
    using p = lexy::parser_for<_seq_impl<R...>, NextParser>;
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
struct _br : _copy_base<Condition>
{
    static_assert(sizeof...(R) >= 0);

    template <typename NextParser>
    using _pc = lexy::parser_for<_seq_impl<R...>, NextParser>;

    // We parse Condition and then seq<R...>.
    // Condition's try_parse() checks the branch condition, which is what we want.
    template <typename Context, typename Reader>
    using bp = lexy::continuation_branch_parser<Condition, Context, Reader, _pc>;

    template <typename NextParser>
    using p = lexy::parser_for<_seq_impl<Condition, R...>, NextParser>;
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
struct _else : unconditional_branch_base
{
    template <typename NextParser>
    using p = NextParser;

    template <typename Context, typename Reader>
    using bp = lexy::unconditional_branch_parser<_else, Context, Reader>;
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





namespace lexyd
{
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
                lexy::token_parser_for<decltype(lexyd::token(Rule{})), Reader> parser(reader);
                parser.try_parse(reader);
                end = parser.end;
            }

            auto err = lexy::error<Reader, Tag>(begin, end);
            context.on(_ev::error{}, err);
            return false;
        }
    };
    template <typename Context, typename Reader>
    using bp = lexy::unconditional_branch_parser<_err, Context, Reader>;

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
            lexy::branch_parser_for<Branch, Context, Reader> branch{};
            if (branch.try_parse(context, reader))
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);

            // The branch wasn't taken, so we fail with the specific error by parsing Error.
            auto result = lexy::parser_for<Error, lexy::pattern_parser<>>::parse(context, reader);
            LEXY_ASSERT(!result, "error must not recover");

            return false;
        }
    };

    // As a branch we parse it exactly the same.
    template <typename Context, typename Reader>
    using bp = lexy::branch_parser_for<Branch, Context, Reader>;
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
    static_assert(!lexy::is_unconditional_branch_rule<Branch>);
    return _must_dsl<Branch>{};
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_TUPLE_HPP_INCLUDED
#define LEXY_DETAIL_TUPLE_HPP_INCLUDED




namespace lexy::_detail
{
template <std::size_t Idx, typename T>
struct _tuple_holder
{
    LEXY_EMPTY_MEMBER T value;
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
    constexpr auto get() noexcept -> element_type<N>&
    {
        // NOLINTNEXTLINE: this is fine.
        return static_cast<_tuple_holder<N, element_type<N>>&>(*this).value;
    }
    template <std::size_t N>
    constexpr auto get() const noexcept -> const element_type<N>&
    {
        // NOLINTNEXTLINE: this is fine.
        return static_cast<const _tuple_holder<N, element_type<N>>&>(*this).value;
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

    template <typename Context, typename Reader,
              typename Indices = lexy::_detail::make_index_sequence<sizeof...(R)>>
    struct bp;
    template <typename Context, typename Reader, std::size_t... Idx>
    struct bp<Context, Reader, lexy::_detail::index_sequence<Idx...>>
    {
        template <typename Rule>
        using rp = lexy::branch_parser_for<Rule, Context, Reader>;

        lexy::_detail::tuple<rp<R>...> r_parsers;
        std::size_t                    branch_idx;

        constexpr auto try_parse(Context& context, const Reader& reader)
            -> std::conditional_t<_any_unconditional, std::true_type, bool>
        {
            auto try_r = [&](std::size_t idx, auto& parser) {
                if (!parser.try_parse(context, reader))
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

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Need to call finish on the selected branch.
            auto result = false;
            (void)((Idx == branch_idx
                        ? (result
                           = r_parsers.template get<Idx>()
                                 .template finish<NextParser>(context, reader, LEXY_FWD(args)...),
                           true)
                        : false)
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
                if (!parser.try_parse(context, reader))
                    return false;

                // LEXY_FWD(args) will break MSVC builds targeting C++17.
                result = parser.template finish<NextParser>(context, reader,
                                                            static_cast<Args&&>(args)...);
                return true;
            };

            // Try to parse each branch in order.
            auto found_branch = (try_r(lexy::branch_parser_for<R, Context, Reader>{}) || ...);
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




namespace lexyd
{
struct _eof : token_base<_eof>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(const Reader& reader)
        {
            return reader.peek() == Reader::encoding::eof();
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(this->end, "EOF");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_eof> = lexy::eof_token_kind;
}

#endif // LEXY_DSL_EOF_HPP_INCLUDED


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
            auto result
                = lexy::parser_for<Rule, _continuation>::parse(context, reader, recovery_finished,
                                                               LEXY_FWD(args)...);
            if (!recovery_finished)
                context.on(_ev::recovery_cancel{}, reader.position());
            return result;
        }
    };
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
            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);

            while (true)
            {
                if (lexy::token_parser_for<Token, Reader> token(reader); token.try_parse(reader))
                {
                    // We've found it.
                    break;
                }
                else if (lexy::token_parser_for<decltype(get_limit()), Reader> limit(reader);
                         limit.try_parse(reader))
                {
                    // Haven't found it, recovery fails.
                    auto end = reader.position();
                    if (begin != end)
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
            if (begin != end)
                context.on(_ev::token{}, lexy::error_token_kind, begin, end);
            context.on(_ev::recovery_finish{}, end);

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

        auto l = (get_limit() / ... / tokens);
        return _find<Token, decltype(l)>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return eof;
        else
            return Limit{};
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
            lexy::branch_parser_for<decltype((R{} | ...)), Context, Reader> recovery{};
            while (!recovery.try_parse(context, reader))
            {
                if (lexy::token_parser_for<decltype(get_limit()), Reader> limit(reader);
                    limit.try_parse(reader))
                {
                    // We've failed to recover as we've reached the limit.
                    auto end = reader.position();
                    if (begin != end)
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
            if (begin != end)
                context.on(_ev::token{}, lexy::error_token_kind, begin, end);
            context.on(_ev::recovery_finish{}, end);

            // Finish with the rule that matched.
            return recovery.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if Token is found before any of R.
    template <typename... Tokens>
    constexpr auto limit(Tokens... tokens) const
    {
        static_assert(sizeof...(Tokens) > 0);
        static_assert((lexy::is_token_rule<Tokens> && ...));

        auto l = (get_limit() / ... / tokens);
        return _reco<decltype(l), R...>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return eof;
        else
            return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
constexpr auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    static_assert((lexy::is_branch_rule<Branches> && ...));
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
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (std::is_base_of_v<_recovery_base, Recover>)
                return lexy::parser_for<Recover, NextParser>::parse(context, reader,
                                                                    LEXY_FWD(args)...);
            else
                return lexy::parser_for<_recovery_wrapper<Recover>,
                                        NextParser>::parse(context, reader, LEXY_FWD(args)...);
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

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward branching behavior.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
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

    template <typename Context, typename Reader>
    using bp = lexy::unconditional_branch_parser<_break, Context, Reader>;
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
            lexy::branch_parser_for<Branch, Context, Reader> branch{};
            while (branch.try_parse(context, reader))
            {
                if (!branch.template finish<lexy::pattern_parser<>>(context, reader))
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



//=== implementation ===//
namespace lexy::_detail
{
template <typename Rule>
struct ws_production
{
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule                = lexy::dsl::loop(Rule{} | lexy::dsl::break_);
};

// A special handler for parsing whitespace.
// It only forwards errors to the context and ignores all other events.
template <typename Context>
class whitespace_handler
{
public:
    constexpr explicit whitespace_handler(Context& context) : _context(&context) {}

    template <typename Production>
    struct event_handler
    {
        static_assert(_detail::error<Production>,
                      "whitespace rule must not contain `dsl::p` or `dsl::recurse`;"
                      "use `dsl::inline_` instead");
    };
    template <typename Rule>
    class event_handler<ws_production<Rule>>
    {
    public:
        template <typename Error>
        constexpr void on(whitespace_handler& handler, parse_events::error ev, Error&& error)
        {
            handler._context->on(ev, LEXY_FWD(error));
        }

        template <typename Event, typename... Args>
        constexpr void on(whitespace_handler&, Event, const Args&...)
        {}
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr bool get_result_void(bool rule_parse_result) &&
    {
        return rule_parse_result;
    }

private:
    Context* _context;
};

template <typename Rule, typename NextParser>
struct manual_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto result = true;
        auto begin  = reader.position();
        if constexpr (lexy::is_token_rule<Rule>)
        {
            // Parsing a token repeatedly cannot fail, so we can optimize it.
            while (lexy::try_match_token(Rule{}, reader))
            {}
        }
        else
        {
            // Parse the rule using a special handler that only forwards errors.
            using production = ws_production<Rule>;
            result = lexy::do_action<production>(whitespace_handler(context), lexy::no_parse_state,
                                                 reader);
        }
        auto end = reader.position();

        if (result)
        {
            // Add a whitespace token node.
            if (begin != end)
                context.on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin, end);

            // And continue.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Add an error token node.
            if (begin != end)
                context.on(lexy::parse_events::token{}, lexy::error_token_kind, begin, end);

            // And cancel.
            return false;
        }
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
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if (context.control_block->enable_whitespace_skipping)
        {
            // Skip the appropriate whitespace.
            using rule = context_whitespace<Context>;
            return manual_ws_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Automatic whitespace skipping is disabled.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
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
    using p = lexy::_detail::manual_ws_parser<Rule, NextParser>;

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
    using p = lexy::_detail::automatic_ws_parser<NextParser>;

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

    template <typename Context, typename Reader,
              typename Whitespace = lexy::_detail::context_whitespace<Context>>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Temporary disable whitespace skipping to parse the rule.
            context.control_block->enable_whitespace_skipping = false;
            auto result                                       = rule.try_parse(context, reader);
            context.control_block->enable_whitespace_skipping = true;
            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule with whitespace skipping disabled.
            context.control_block->enable_whitespace_skipping = false;
            return rule.template finish<_pc<NextParser>>(context, reader, LEXY_FWD(args)...);
        }
    };

    // Optimization: if there is no whitespace rule, we just parse Rule directly.
    template <typename Context, typename Reader>
    struct bp<Context, Reader, void> : lexy::branch_parser_for<Rule, Context, Reader>
    {};

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
            {
                // No whitespace, just parse the rule.
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
        return rule + terminator();
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
        return _lstt<Terminator, Rule, Sep, decltype(recovery_rule())>{};
    }

    /// Matches `opt_list(rule, sep)` followed by terminator.
    /// The rule does not require a condition.
    template <typename Rule>
    constexpr auto opt_list(Rule) const
    {
        return _optt<Terminator, _lstt<Terminator, Rule, void, decltype(recovery_rule())>>{};
    }
    template <typename Rule, typename S>
    constexpr auto opt_list(Rule, S) const
    {
        return _optt<Terminator, _lstt<Terminator, Rule, S, decltype(recovery_rule())>>{};
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
template <typename Token>
struct _capt : _copy_base<Token>
{
    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr auto try_parse(Context&, const Reader& reader)
        {
            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, Token{}, begin, end);
            reader.set_position(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
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
struct _cap : _copy_base<Rule>
{
    template <typename NextParser, typename... PrevArgs>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           PrevArgs&&... prev_args, typename Reader::iterator begin,
                                           Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                     lexy::lexeme(reader, begin), LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward to the rule.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
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

/// Captures whatever the token matches as a lexeme; does not include trailing whitespace.
template <typename Token>
constexpr auto capture_token(Token)
{
    static_assert(lexy::is_token_rule<Token>);
    return _capt<Token>{};
}

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
    static_assert((lexy::is_branch_rule<R> && ...), "combination() requires a branch rule");
    static_assert((!lexy::is_unconditional_branch_rule<R> && ...),
                  "combination() does not support unconditional branches");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
constexpr auto partial_combination(R...)
{
    static_assert((lexy::is_branch_rule<R> && ...), "partial_combination() requires a branch rule");
    static_assert((!lexy::is_unconditional_branch_rule<R> && ...),
                  "partial_combination() does not support unconditional branches");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
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
            _ctx_counter<Id>::get(context) += Delta;
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

            _ctx_counter<Id>::get(context) += int(length) * Sign;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward to the rule.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
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
    template <typename Context, typename Reader>
    struct bp
    {
        constexpr bool try_parse(Context& context, const Reader&)
        {
            return _ctx_counter<Id>::get(context) == Value;
        }

        template <typename NextParser, typename... Args>
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
                                     _ctx_counter<Id>::get(context));
        }
    };
};

template <typename... Ids>
struct _ctx_ceq;
template <typename H, typename... T>
struct _ctx_ceq<H, T...> : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        constexpr bool try_parse(Context& context, const Reader&)
        {
            auto value = _ctx_counter<H>::get(context);
            return ((value == _ctx_counter<T>::get(context)) && ...);
        }

        template <typename NextParser, typename... Args>
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
            auto value = _ctx_counter<H>::get(context);
            if (((value != _ctx_counter<T>::get(context)) || ...))
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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
            _ctx_flag<Id>::get(context) = Value;
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
            auto& flag = _ctx_flag<Id>::get(context);
            flag       = !flag;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, bool Value>
struct _ctx_fis : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        constexpr bool try_parse(Context& context, const Reader&)
        {
            return _ctx_flag<Id>::get(context) == Value;
        }

        template <typename NextParser, typename... Args>
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
                                     _ctx_flag<Id>::get(context));
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






// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MINUS_HPP_INCLUDED
#define LEXY_DSL_MINUS_HPP_INCLUDED






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
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Token, Reader> token_parser;
        typename Reader::iterator             end;
        bool                                  minus_failure;

        constexpr explicit tp(const Reader& reader)
        : token_parser(reader), end(reader.position()), minus_failure(false)
        {}

        constexpr bool try_parse(const Reader& reader)
        {
            // Try to parse the token.
            if (!token_parser.try_parse(reader))
            {
                // It didn't match, so we fail.
                minus_failure = false;
                return false;
            }
            // We already remember the end to have it during error reporting as well.
            end = token_parser.end;

            // Check whether Except matches on the same input and we're then at EOF.
            if (auto partial = lexy::partial_reader(reader, token_parser.end);
                lexy::try_match_token(Except{}, partial)
                && partial.peek() == Reader::encoding::eof())
            {
                // Except did match, so we fail.
                minus_failure = true;
                return false;
            }

            // Success.
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (minus_failure)
            {
                auto err = lexy::error<Reader, lexy::minus_failure>(reader.position(), this->end);
                context.on(_ev::error{}, err);
            }
            else
            {
                // Delegate error to the actual token.
                token_parser.report_error(context, reader);
            }
        }
    };
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

namespace lexyd
{
template <typename Token>
struct _prefix : token_base<_prefix<Token>>
{
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Token, Reader> token;
        typename Reader::iterator             end;

        constexpr explicit tp(const Reader& reader) : token(reader), end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the token.
            if (!token.try_parse(reader))
                return false;
            reader.set_position(token.end);

            // Consume the rest of the input.
            lexy::try_match_token(lexy::dsl::any, reader);
            end = reader.position();

            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            // Only the token part can fail.
            token.report_error(context, reader);
        }
    };
};

template <typename Token>
constexpr auto prefix(Token)
{
    return _prefix<Token>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Token>
struct _contains : token_base<_contains<Token>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            while (true)
            {
                if (lexy::try_match_token(Token{}, reader))
                {
                    // We've found it.
                    break;
                }
                else if (reader.peek() == Reader::encoding::eof())
                {
                    // Haven't found it.
                    end = reader.position();
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            // Consume everything else.
            lexy::try_match_token(lexy::dsl::any, reader);

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            // Trigger an error by parsing the token at the end of the input.
            reader.set_position(end);

            LEXY_ASSERT(reader.peek() == Reader::encoding::eof(),
                        "forgot to set end in try_parse()");

            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            LEXY_ASSERT(!result, "token shouldn't have matched?!");
            parser.report_error(context, reader);
        }
    };
};

template <typename Token>
constexpr auto contains(Token)
{
    return _contains<Token>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED




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
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Leading, Reader> leading;
        typename Reader::iterator               end;

        constexpr explicit tp(const Reader& reader) : leading(reader), end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match Leading character.
            if (!leading.try_parse(reader))
                return false;
            reader.set_position(leading.end);
            end = leading.end;

            // Match zero or more trailing characters.
            while (lexy::try_match_token(Trailing{}, reader))
            {}

            end = reader.position();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            leading.report_error(context, reader);
        }
    };
};

template <typename Id, typename CharT, CharT... C>
struct _kw;

template <typename Leading, typename Trailing, typename... Reserved>
struct _id : branch_base
{
    template <typename Reader>
    constexpr static auto _is_reserved(const Reader& reader, typename Reader::iterator begin,
                                       typename Reader::iterator end)
    {
        if constexpr (sizeof...(Reserved) == 0)
        {
            (void)reader;
            (void)begin;
            (void)end;

            // No reserved patterns, never reserved.
            return std::false_type{};
        }
        else
        {
            auto id_reader = lexy::partial_reader(reader, begin, end);
            // Need to match any of the reserved tokens.
            return lexy::try_match_token((Reserved{} / ...), id_reader)
                   // And fully match it.
                   && id_reader.position() == end;
        }
    }

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
            if (_is_reserved(reader, begin, end))
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

    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr bool try_parse(Context&, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<decltype(pattern()), Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // We only succeed if it's not a reserved identifier.
            return !_is_reserved(reader, reader.position(), end);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end);
            reader.set_position(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };

    template <typename R>
    constexpr auto _make_reserve(R r) const
    {
        return lexyd::token(r);
    }
    template <typename Id, typename CharT, CharT... C>
    constexpr auto _make_reserve(_kw<Id, CharT, C...>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // We turn the keyword into a literal to be able to use a trie for matching.
        return _lit<CharT, C...>{};
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
    constexpr auto reserve_prefix(R... r) const
    {
        return reserve(prefix(_make_reserve(r))...);
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    constexpr auto reserve_containing(R... r) const
    {
        return reserve(contains(_make_reserve(r))...);
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

namespace lexy
{
template <typename Leading, typename Trailing>
constexpr auto token_kind_of<lexy::dsl::_idp<Leading, Trailing>> = lexy::identifier_token_kind;
}

//=== keyword ===//
namespace lexyd
{
template <typename Id, typename CharT, CharT... C>
struct _kw : token_base<_kw<Id, CharT, C...>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match the literal.
            if (!lexy::try_match_token(_lit<CharT, C...>{}, reader))
                return false;
            end = reader.position();

            // To qualify as a keyword, and not just the prefix of an identifier,
            // we must not have a trailing identifier character.
            return !lexy::try_match_token(Id{}.trailing_pattern(), reader);
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<
                typename Reader::encoding::char_type>;

            // Match the entire identifier.
            auto begin = reader.position();
            lexy::try_match_token(Id{}.pattern(), reader);
            auto end = reader.position();

            auto err = lexy::error<Reader, lexy::expected_keyword>(begin, end, str);
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
    return _kw<_id<L, T>, std::decay_t<decltype(C)>, C>{};
}
#endif

#define LEXY_KEYWORD(Str, Id)                                                                      \
    LEXY_NTTP_STRING(::lexyd::_keyword<std::decay_t<decltype(Id)>>::template get, Str) {}
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
            _ctx_id<Id, Reader>::get(context) = (args, ...);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
    template <typename Context, typename Reader>
    using bp = lexy::continuation_branch_parser<Identifier, Context, Reader, _pc>;
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

    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr bool try_parse(Context& context, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<_pattern, Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // The two lexemes need to be equal.
            auto lexeme = lexy::lexeme<Reader>(reader.position(), end);
            return lexy::_detail::equal_lexemes(_ctx_id<Id, Reader>::get(context), lexeme);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish parsing the token.
            context.on(_ev::token{}, lexy::identifier_token_kind, reader.position(), end);
            reader.set_position(end);
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
                if (!lexy::_detail::equal_lexemes(_ctx_id<Id, Reader>::get(context), lexeme))
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
            return lexy::parser_for<_capt<_pattern>, _cont<Args...>>::parse(context, reader,
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED






// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SYMBOL_HPP_INCLUDED
#define LEXY_DSL_SYMBOL_HPP_INCLUDED










namespace lexy
{
#define LEXY_SYMBOL(Str) LEXY_NTTP_STRING(::lexy::_detail::type_string, Str)

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
        return map<_detail::type_string<std::decay_t<decltype(C)>, C>>(LEXY_FWD(args)...);
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

        auto result = _detail::trie_parser<_trie::object, Reader>::parse(reader);
        if (result == _trie::object.invalid_value)
            return key_index();
        else
            return key_index(result);
    }

    constexpr const T& operator[](key_index idx) const noexcept
    {
        LEXY_PRECONDITION(idx);
        return _data[idx._value];
    }

private:
    struct _trie
    {
        static constexpr auto object = lexy::_detail::trie<char_type, Strings...>;
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
struct _sym : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator                         end;
        typename std::decay_t<decltype(Table)>::key_index symbol;

        constexpr bool try_parse(Context&, const Reader& reader)
        {
            // Try and parse the token.
            lexy::token_parser_for<Token, Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // Check whether this is a symbol.
            auto content = lexy::partial_reader(reader, end);
            symbol       = Table.try_parse(content);

            // We need to consume everything.
            if (content.position() != end)
                return false;

            // Only succeed if it is a symbol.
            return static_cast<bool>(symbol);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // We need to consume and report the token.
            context.on(_ev::token{}, Token{}, reader.position(), end);
            reader.set_position(end);

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
                auto content = lexy::partial_reader(reader, lexeme.begin(), lexeme.end());
                auto symbol  = Table.try_parse(content);
                if (!symbol || content.position() != lexeme.end())
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
            // Capture the token and continue with special continuation.
            return lexy::parser_for<_capt<Token>, _cont<Args...>>::parse(context, reader,
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
    template <typename Context, typename Reader>
    struct bp
    {
        typename std::decay_t<decltype(Table)>::key_index symbol;
        typename Reader::iterator                         end;

        constexpr bool try_parse(const Context&, Reader reader)
        {
            // Try to parse a symbol.
            symbol = Table.try_parse(reader);
            if (!symbol)
                return false;
            end = reader.position();

            // We had a symbol, but it must not be the prefix of a valid identifier.
            return !lexy::try_match_token(T{}, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // We need to consume and report the identifier pattern.
            context.on(_ev::token{}, _idp<L, T>{}, reader.position(), end);
            reader.set_position(end);

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
                auto end = symbol_reader.position();
                context.on(_ev::token{}, _idp<L, T>{}, begin, end);
                reader.set_position(end);

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
    template <typename Context, typename Reader>
    struct bp
    {
        typename std::decay_t<decltype(Table)>::key_index symbol;
        typename Reader::iterator                         end;

        constexpr bool try_parse(const Context&, Reader reader)
        {
            // Try to parse a symbol.
            symbol = Table.try_parse(reader);
            end    = reader.position();

            // Only succeed if it is a symbol.
            return static_cast<bool>(symbol);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // We need to consume and report the token.
            context.on(_ev::token{}, lexy::identifier_token_kind, reader.position(), end);
            reader.set_position(end);

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
            bp<Context, Reader> impl{};
            if (impl.try_parse(context, reader))
                return impl.template finish<NextParser>(context, reader, LEXY_FWD(args)...);

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
template <typename Close, typename Char, typename Limit, typename... Escapes>
struct _del : rule_base
{
    template <typename CloseParser, typename Context, typename Reader, typename Sink>
    static constexpr bool _loop(CloseParser& close, Context& context, Reader& reader, Sink& sink)
    {
        auto del_begin = reader.position();
        while (!close.try_parse(context, reader))
        {
            // Check for missing delimiter.
            if (lexy::branch_parser_for<Limit, Context, Reader> limit{};
                limit.try_parse(context, reader))
            {
                auto err
                    = lexy::error<Reader, lexy::missing_delimiter>(del_begin, reader.position());
                context.on(_ev::error{}, err);
                return false;
            }

            // Check for escape sequences.
            if ((Escapes::_try_parse(context, reader, sink) || ...))
                // We had an escape sequence, so do nothing in this iteration.
                continue;

            // Parse the next character.
            if (auto begin = reader.position(); Char::token_parse(context, reader))
            {
                // Pass it to the sink.
                sink(lexy::lexeme<Reader>(begin, reader.position()));
            }
            else
            {
                // Recover from it; this is always possible,.
                context.on(_ev::recovery_start{}, reader.position());

                if (begin == reader.position())
                {
                    // The character didn't consume anything, so we manually discard one code unit.
                    LEXY_ASSERT(reader.peek() != Reader::encoding::eof(),
                                "EOF should be checked before calling this");
                    reader.bump();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, reader.position());
                }

                context.on(_ev::recovery_finish{}, reader.position());
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
            auto sink = context.value_callback().sink();

            // Parse characters until we have the closing delimiter.
            lexy::branch_parser_for<Close, Context, Reader> close{};
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

template <typename Open, typename Close, typename Limit = lexyd::_eof>
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
    return _delim_dsl<Open, Close>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
constexpr auto delimited(Delim)
{
    static_assert(lexy::is_branch_rule<Delim>);
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
    template <typename Context, typename Reader, typename Sink>
    static constexpr bool _try_parse(Context& context, Reader& reader, Sink& sink)
    {
        auto begin = reader.position();

        // Check whether we're having the initial escape character.
        lexy::branch_parser_for<Escape, Context, Reader> token{};
        if (!token.try_parse(context, reader))
            // No need to call backtrack(), it's a token.
            return false;

        // We do, so consume it.
        // It's a token, so this can't fail.
        token.template finish<lexy::pattern_parser<>>(context, reader);

        // Try to parse the correct branch.
        auto try_parse_branch = [&](auto branch) {
            lexy::branch_parser_for<decltype(branch), Context, Reader> parser{};
            if (!parser.try_parse(context, reader))
                return false;

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

        return true;
    }

    /// Adds a generic escape rule.
    template <typename Branch>
    constexpr auto rule(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the branch.
    template <typename Branch>
    constexpr auto capture(Branch branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED






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
                end                    = reader.position();
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
                        end                    = reader.position();
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
                auto err = lexy::error<Reader, lexy::expected_char_class>(end, Base::name());
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
            {
                end = reader.position();
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
                        end = reader.position();
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
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end, Base::name());
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
            {
                end = reader.position();
                return false;
            }

            // Match each other digit after a separator.
            auto success = (((void)Idx, lexy::try_match_token(Sep{}, reader),
                             lexy::try_match_token(digit<Base>, reader))
                            && ...);
            end          = reader.position();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end, Base::name());
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
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end, Base::name());
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Branch, Context, Reader> branch{};
            if (branch.try_parse(context, reader))
                // We take the branch.
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            else
                // We don't take the branch.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// If the branch condition matches, matches the branch then.
template <typename Branch>
constexpr auto if_(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>, "if_() requires a branch condition");
    if constexpr (lexy::is_unconditional_branch_rule<Branch>)
        // Branch is always taken, so don't wrap in if_().
        return Branch{};
    else
        return _if<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_IF_HPP_INCLUDED

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
            auto result = typename IntParser::result_type(0);
            if (!IntParser::parse(result, begin, end))
            {
                // Raise error but recover.
                using tag = lexy::_detail::type_or<Tag, lexy::integer_overflow>;
                context.on(_ev::error{}, lexy::error<Reader, tag>(begin, end));
            }

            // Need to skip whitespace now as well.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)..., result);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr auto try_parse(Context&, const Reader& reader)
        {
            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, Token{}, begin, end);
            reader.set_position(end);

            return _pc<NextParser>::parse(context, reader, begin, end, LEXY_FWD(args)...);
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
            {
                auto recovery_begin = reader.position();
                context.on(_ev::recovery_start{}, recovery_begin);

                // To recover we try and skip additional digits.
                while (lexy::try_match_token(digit<typename IntParser::base>, reader))
                {}

                auto recovery_end = reader.position();
                if (begin == recovery_begin && recovery_begin == recovery_end)
                {
                    // We didn't get any digits; don't recover.
                    context.on(_ev::recovery_cancel{}, recovery_end);
                    return false;
                }
                else
                {
                    if (recovery_begin != recovery_end)
                        context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                   recovery_end);
                    context.on(_ev::recovery_finish{}, recovery_end);
                }
            }
            auto end = reader.position();

            return _pc<NextParser>::parse(context, reader, begin, end, LEXY_FWD(args)...);
        }
    };
};

/// Parses the digits matched by the rule into an integer type.
template <typename T, typename Base, typename Digits>
constexpr auto integer(Digits)
{
    static_assert(lexy::is_token_rule<Digits>);
    using parser = _integer_parser<T, Base, false>;
    return _int<Digits, parser, void>{};
}

template <typename T, typename Base>
constexpr auto integer(_digits<Base>)
{
    using parser = _integer_parser<T, Base, true>;
    return _int<_digits<Base>, parser, void>{};
}
template <typename T, typename Base, typename Sep>
constexpr auto integer(_digits_s<Base, Sep>)
{
    using parser = _integer_parser<T, Base, false>;
    return _int<_digits_s<Base, Sep>, parser, void>{};
}
template <typename T, typename Base>
constexpr auto integer(_digits_t<Base>)
{
    using parser = _integer_parser<T, Base, true>;
    return _int<_digits_t<Base>, parser, void>{};
}
template <typename T, typename Base, typename Sep>
constexpr auto integer(_digits_st<Base, Sep>)
{
    using parser = _integer_parser<T, Base, false>;
    return _int<_digits_st<Base, Sep>, parser, void>{};
}

template <typename T, typename Base, std::size_t N>
constexpr auto integer(_ndigits<N, Base>)
{
    using type
        = std::conditional_t<_ndigits_can_overflow<T, N, Base::radix>(), T, lexy::unbounded<T>>;
    using parser = _integer_parser<type, Base, true>;
    return _int<_ndigits<N, Base>, parser, void>{};
}
template <typename T, typename Base, std::size_t N, typename Sep>
constexpr auto integer(_ndigits_s<N, Base, Sep>)
{
    using type
        = std::conditional_t<_ndigits_can_overflow<T, N, Base::radix>(), T, lexy::unbounded<T>>;
    using parser = _integer_parser<type, Base, false>;
    return _int<_ndigits_s<N, Base, Sep>, parser, void>{};
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
    return _int<_ndigits<N, Base>, parser, lexy::invalid_code_point>{};
}();
} // namespace lexyd

#endif // LEXY_DSL_INTEGER_HPP_INCLUDED

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
            lexy::branch_parser_for<Branch, Context, Reader> branch{};
            if (branch.try_parse(context, reader))
                // We take the branch.
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            else
                // We don't take the branch and produce a nullopt.
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
constexpr auto opt(Rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "opt() requires a branch condition");
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
            lexy::branch_parser_for<Term, Context, Reader> term{};
            if (term.try_parse(context, reader))
                // We had the terminator, so produce nullopt.
                return term.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                        lexy::nullopt{});

            // We didn't have the terminator, so we parse the rule.
            using parser = lexy::parser_for<Rule, NextParser>;
            return parser::parse(context, reader, LEXY_FWD(args)...);
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
    static constexpr void report_trailing_error(Context&                  context, Reader&,
                                                typename Reader::iterator sep_begin,
                                                typename Reader::iterator sep_end)
    {
        auto err = lexy::error<Reader, Tag>(sep_begin, sep_end);
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
    static constexpr void report_trailing_error(Context&, Reader&, typename Reader::iterator,
                                                typename Reader::iterator)
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


namespace lexyd
{
template <typename Item, typename Sep>
struct _lst : _copy_base<Item>
{
    template <typename Context, typename Reader, typename Sink>
    static constexpr bool _loop(Context& context, Reader& reader, Sink& sink)
    {
        while (true)
        {
            // Parse a separator if necessary.
            [[maybe_unused]] auto sep_begin = reader.position();
            if constexpr (!std::is_void_v<Sep>)
            {
                lexy::branch_parser_for<typename Sep::rule, Context, Reader> sep{};
                if (!sep.try_parse(context, reader))
                    // We didn't have a separator, list is definitely finished.
                    break;

                if (!sep.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }
            [[maybe_unused]] auto sep_end = reader.position();

            // Parse the next item.
            if constexpr (lexy::is_branch_rule<Item>)
            {
                // It's a branch, so try parsing it to detect loop exit.
                lexy::branch_parser_for<Item, Context, Reader> item{};
                if (!item.try_parse(context, reader))
                {
                    // We don't have a next item, exit the loop.
                    // If necessary, we report a trailing separator.
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

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Item, Context, Reader> item;

        constexpr bool try_parse(Context& context, const Reader& reader)
        {
            // We parse a list if we can parse its first item.
            return item.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
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
    static constexpr bool _loop(_state initial_state, TermParser& term, Context& context,
                                Reader& reader, Sink& sink)
    {
        auto state = initial_state;

        [[maybe_unused]] auto sep_pos = reader.position();
        while (true)
        {
            switch (state)
            {
            case _state::terminator:
                if (term.try_parse(context, reader))
                    // We had the terminator, so the list is done.
                    return true;

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
                            lexy::branch_parser_for<Item, Context, Reader> item{};
                            if (item.try_parse(context, reader)
                                && item.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue after an item has been parsed.
                                state = _state::terminator;
                                break;
                            }
                            else
                            {
                                // Not an item, recover.
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
                    if (term.try_parse(context, reader))
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

            case _state::recovery:
                context.on(_ev::recovery_start{}, reader.position());
                while (true)
                {
                    // Recovery succeeds when we reach the next separator.
                    if constexpr (!std::is_void_v<Sep>)
                    {
                        sep_pos = reader.position();

                        lexy::branch_parser_for<typename Sep::rule, Context, Reader> sep{};
                        if (sep.try_parse(context, reader))
                        {
                            context.on(_ev::recovery_finish{}, reader.position());
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
                    }
                    // When we don't have a separator, but the item is a branch, we also succeed
                    // when we reach the next item.
                    //
                    // Note that we're doing this check only if we don't have a separator.
                    // If we do have one, the heuristic "end of the invalid item" is better than
                    // "beginning of the next one".
                    else if constexpr (lexy::is_branch_rule<Item>)
                    {
                        lexy::branch_parser_for<Item, Context, Reader> item{};
                        if (item.try_parse(context, reader))
                        {
                            context.on(_ev::recovery_finish{}, reader.position());
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
                    }

                    // At this point, we couldn't detect the next item.
                    // Recovery succeeds when we reach the terminator.
                    if (term.try_parse(context, reader))
                    {
                        // We're now done with the entire list.
                        context.on(_ev::recovery_finish{}, reader.position());
                        return true;
                    }

                    // At this point, we couldn't detect the next item or a terminator.
                    // Recovery fails when we reach the limit.
                    lexy::branch_parser_for<decltype(Recover{}.get_limit()), Context, Reader>
                        limit{};
                    if (limit.try_parse(context, reader))
                    {
                        // Recovery has failed, propagate error.
                        context.on(_ev::recovery_cancel{}, reader.position());
                        return false;
                    }

                    // Consume one code unit and try again.
                    auto begin = reader.position();
                    reader.bump();
                    auto end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                }
                break;
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
            lexy::branch_parser_for<Term, Context, Reader> term{};
            auto                                           sink = context.value_callback().sink();

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
struct _look : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        static constexpr bool try_parse(Context& context, Reader reader)
        {
            auto begin = reader.position();

            auto result = [&] {
                while (true)
                {
                    // Try to match Needle.
                    if (lexy::try_match_token(Needle{}, reader))
                        // We found it.
                        return true;
                    // Check whether we've reached the End.
                    else if (reader.peek() == Reader::encoding::eof()
                             || lexy::try_match_token(End{}, reader))
                        // We've failed.
                        return false;
                    else
                        // Try again.
                        reader.bump();
                }

                return false; // unreachable
            }();

            auto end = reader.position();

            // Report that we've backtracked.
            context.on(_ev::backtracked{}, begin, end);

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC static bool finish(Context& context, Reader& reader, Args&&... args)
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
            if (!bp<Context, Reader>::try_parse(context, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::error<Reader, tag>(reader.position());
                context.on(_ev::error{}, err);
                // But recover immediately, as we wouldn't have consumed anything either way.
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
    return _look<Needle, End, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED


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
    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward to the rule.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but add member tag.
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
template <typename Rule, typename Tag>
struct _peek : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        static constexpr bool try_parse(Context& context, Reader reader)
        {
            // We need to match the entire rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            auto begin  = reader.position();
            auto result = parser.try_parse(reader);
            auto end    = parser.end;

            // Report that we've backtracked.
            context.on(_ev::backtracked{}, begin, end);

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC static bool finish(Context& context, Reader& reader, Args&&... args)
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
            if (!bp<Context, Reader>::try_parse(context, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::peek_failure>;
                auto err  = lexy::error<Reader, tag>(reader.position());
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peek<Rule, Error> error = {};
};

template <typename Rule, typename Tag>
struct _peekn : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr bool try_parse(Context& context, Reader reader)
        {
            // We must not match the rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            auto begin  = reader.position();
            auto result = !parser.try_parse(reader);
            end         = parser.end;

            // Report that we've backtracked.
            context.on(_ev::backtracked{}, begin, end);

            return result;
        }

        template <typename NextParser, typename... Args>
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
            if (bp<Context, Reader> impl{}; !impl.try_parse(context, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::unexpected>;
                auto err  = lexy::error<Reader, tag>(reader.position(), impl.end);
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
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

/// Check if at this reader position, the rule would not match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek_not(Rule)
{
    return _peekn<Rule, void>{};
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
struct _prd : _copy_base<lexy::production_rule<Production>>
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

    template <typename Context, typename Reader>
    struct bp
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context&).sub_context(Production{}));
        using parser_t
            = lexy::branch_parser_for<lexy::production_rule<Production>, sub_context_t, Reader>;

        lexy::_detail::lazy_init<sub_context_t> sub_context;
        parser_t                                parser;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Create the new context.
            sub_context = {};
            sub_context.emplace(context.sub_context(Production{}));
            sub_context->on(_ev::production_start{}, reader.position());

            // Try and parse the production on the new context.
            auto result = parser.try_parse(*sub_context, reader);
            if (!result)
                sub_context->on(_ev::production_cancel{}, reader.position());

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the production.
            if (_finish_production(parser, *sub_context, reader))
            {
                sub_context->on(_ev::production_finish{}, reader.position());

                using continuation = lexy::_detail::context_finish_parser<NextParser>;
                return continuation::parse(context, reader, *sub_context, LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                sub_context->on(_ev::production_cancel{}, reader.position());
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

    template <typename Context, typename Reader>
    struct bp
    {
        static_assert(lexy::is_branch_rule<lexy::production_rule<Production>>);

        using impl = lexy::branch_parser_for<_prd<Production>, Context, Reader>;
        impl _impl;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            return _impl.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
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

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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

LEXY_PUNCT(hash_sign, "#");
LEXY_PUNCT(dollar_sign, "$");
LEXY_PUNCT(at_sign, "@");
LEXY_PUNCT(percent_sign, "%");
LEXY_PUNCT(equal_sign, "=");

#undef LEXY_PUNCT
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
    using p = lexy::_detail::final_parser;
};

/// Finishes parsing a production without considering subsequent rules.
constexpr auto return_ = _ret{};
} // namespace lexyd

#endif // LEXY_DSL_RETURN_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SCAN_HPP_INCLUDED
#define LEXY_DSL_SCAN_HPP_INCLUDED



// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_CALLBACK_FORWARD_HPP_INCLUDED
#define LEXY_CALLBACK_FORWARD_HPP_INCLUDED



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
template <>
struct _fwd<void>
{
    using return_type = void;

    constexpr void operator()() const {}
};

/// A callback that just forwards an existing object.
template <typename T>
constexpr auto forward = _fwd<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_FORWARD_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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




//=== rule forward declaration ===//
namespace lexyd
{
template <typename Production>
struct _prd;
template <typename Rule, typename Tag>
struct _peek;

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

    constexpr const auto& value() const& noexcept
    {
        return *_value;
    }
    constexpr auto&& value() && noexcept
    {
        return LEXY_MOV(*_value);
    }

    template <typename U = T>
    constexpr U value_or(U&& fallback) const& noexcept
    {
        return _value ? *_value : LEXY_FWD(fallback);
    }
    template <typename U = T>
    constexpr U value_or(U&& fallback) && noexcept
    {
        return _value ? LEXY_MOV(*_value) : LEXY_FWD(fallback);
    }

private:
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
    _detail::lazy_init<void> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};

template <typename T>
scan_result(T&&) -> scan_result<std::decay_t<T>>;
} // namespace lexy

//=== scanner implementation ===//
namespace lexy::_detail
{
template <typename Context>
using _value_callback_for = lexy::production_value_callback<
    typename Context::production,
    std::remove_pointer_t<decltype(LEXY_DECLVAL(decltype(Context::control_block))->parse_state)>>;

// The context used for a child production during scanning.
// It forwards all events but overrides the value callback.
template <typename Context, typename ValueCallback = _value_callback_for<Context>>
struct spc_child
{
    using production      = typename Context::production;
    using root_production = typename Context::root_production;
    using value_type      = typename ValueCallback::return_type;

    decltype(Context::handler)       handler;
    decltype(Context::control_block) control_block;
    _detail::lazy_init<value_type>   value;

    constexpr explicit spc_child(decltype(Context::control_block) cb) : control_block(cb) {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction child)
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context).sub_context(child));
        if constexpr (std::is_same_v<ValueCallback, void_value_callback>)
            return spc_child<sub_context_t, void_value_callback>(control_block);
        else
            return spc_child<sub_context_t>(control_block);
    }

    constexpr auto value_callback()
    {
        return ValueCallback(control_block->parse_state);
    }

    template <typename Event, typename... Args>
    constexpr void on(Event ev, Args&&... args)
    {
        handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};

// The context used for a top-level rule parsing during scanning.
// It forwards all events but overrids the value callback to construct a T.
template <typename T, typename Context>
struct spc
{
    using production      = typename Context::production;
    using root_production = typename Context::root_production;
    using value_type      = T;

    decltype(Context::handler)&      handler;
    decltype(Context::control_block) control_block;
    _detail::lazy_init<T>&           value;

    constexpr explicit spc(_detail::lazy_init<T>& value, Context& context)
    : handler(context.handler), control_block(context.control_block), value(value)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction child)
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context).sub_context(child));
        if constexpr (std::is_void_v<T>)
            return spc_child<sub_context_t, void_value_callback>(control_block);
        else
            return spc_child<sub_context_t>(control_block);
    }

    constexpr auto value_callback()
    {
        return lexy::construct<T>;
    }

    template <typename Event, typename... Args>
    constexpr void on(Event ev, Args&&... args)
    {
        handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};

template <typename Reader>
struct scanner_input
{
    Reader _impl;

    constexpr auto reader() const&
    {
        return _impl;
    }
};

// The common interface of all scanner types.
template <typename Derived, typename Reader>
class scanner
{
public:
    using encoding = typename Reader::encoding;

    constexpr scanner(const scanner&) noexcept = delete;
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

        _detail::spc context(result._value, static_cast<Derived&>(*this).context());

        using parser = lexy::parser_for<Rule, lexy::_detail::final_parser>;
        auto success = parser::parse(context, _reader);
        if (!success)
            _state = _state_failed;
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
        static_assert(lexy::is_branch_rule<Rule>);
        if (_state == _state_failed)
            return false;

        _detail::spc context(result._value, static_cast<Derived&>(*this).context());

        lexy::branch_parser_for<Rule, decltype(context), Reader> parser{};
        if (!parser.try_parse(context, _reader))
            return false; // branch wasn't token

        auto success = parser.template finish<lexy::_detail::final_parser>(context, _reader);
        if (!success)
            _state = _state_failed;
        return true; // branch was taken
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
        error_recovery_guard(const error_recovery_guard&) = delete;
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

        if (begin != end)
        {
            auto& context = static_cast<Derived&>(*this).context();
            context.on(parse_events::token{}, lexy::error_token_kind, begin, end);
        }

        return result;
    }

    template <typename Tag, typename... Args>
    constexpr void error(Tag, Args&&... args)
    {
        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::error{}, lexy::error<Reader, Tag>(LEXY_FWD(args)...));
    }

    template <typename Tag, typename... Args>
    constexpr void fatal_error(Tag tag, Args&&... args)
    {
        error(tag, LEXY_FWD(args)...);
        _state = _state_failed;
    }

    //=== convenience ===//
    template <typename Rule>
    constexpr bool peek(Rule)
    {
        static_assert(lexy::is_rule<Rule>);
        return branch(dsl::_peek<Rule, void>{});
    }

    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr auto parse(Rule rule)
    {
        scan_result<T> result;
        parse(result, rule);
        return result;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    constexpr auto parse(Production = {})
    {
        using value_type = typename lexy::production_value_callback<Production>::return_type;

        scan_result<value_type> result;
        parse(result, lexyd::_prd<Production>{});
        return result;
    }

    template <typename Production, typename T, typename = lexy::production_rule<Production>>
    constexpr bool branch(scan_result<T>& result, Production = {})
    {
        return branch(result, lexyd::_prd<Production>{});
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
    using production      = typename Context::production;
    using root_production = typename Context::root_production;

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
template <typename Context, typename Scanner, typename StatePtr>
using _detect_scan_state = decltype(Context::production::scan(LEXY_DECLVAL(Scanner&), *StatePtr()));

struct _scan : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::rule_scanner scanner(context, reader);
            lexy::scan_result  result = [&] {
                if constexpr (lexy::_detail::is_detected<
                                  _detect_scan_state, Context, decltype(scanner),
                                  decltype(context.control_block->parse_state)>)
                    return Context::production::scan(scanner, *context.control_block->parse_state);
                else
                    return Context::production::scan(scanner);
            }();
            reader.set_position(scanner.position());
            if (!result)
                return false;

            if constexpr (std::is_void_v<typename decltype(result)::value_type>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         LEXY_MOV(result).value());
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
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
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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

#ifndef LEXY_DSL_UNICODE_HPP_INCLUDED
#define LEXY_DSL_UNICODE_HPP_INCLUDED




#if LEXY_HAS_UNICODE_DATABASE
#    define LEXY_UNICODE_PROPERTY_PREDICATE(Prop)                                                  \
        constexpr bool operator()(lexy::code_point cp) const                                       \
        {                                                                                          \
            using namespace lexy::_unicode_db;                                                     \
            auto idx  = property_index(cp.value());                                                \
            auto mask = binary_properties[idx];                                                    \
                                                                                                   \
            return (mask & (1 << lexy::_unicode_db::Prop)) != 0;                                   \
        }
#else
#    define LEXY_UNICODE_PROPERTY_PREDICATE(Prop) bool operator()(lexy::code_point cp) const;
#endif

namespace lexyd::unicode
{
inline constexpr auto control = code_point.general_category<lexy::code_point::control>();

//=== whitespace ===//
struct _blank
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.blank";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        if (cp.value() == '\t')
            return true;
        return cp.general_category() == lexy::code_point::space_separator;
    }
};
inline constexpr auto blank = code_point.if_<_blank>();

struct _newline
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.newline";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // ASCII newlines.
        return cp.value() == '\n'
               || cp.value() == '\r'
               // NEL, PARAGRAPH SEPARATOR, LINE SEPARATOR
               || cp.value() == 0x85 || cp.value() == 0x2029 || cp.value() == 0x2028;
    }
};
inline constexpr auto newline = code_point.if_<_newline>();

struct _other_space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.other-space";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // The same as in ASCII.
        return cp.value() == '\f' || cp.value() == '\v';
    }
};
inline constexpr auto other_space = code_point.if_<_other_space>();

struct _space
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.whitespace";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(whitespace)
};
inline constexpr auto space = code_point.if_<_space>();

//=== alpha ===//
struct _lower
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.lowercase";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(lowercase)
};
inline constexpr auto lower = code_point.if_<_lower>();

struct _upper
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.uppercase";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(uppercase)
};
inline constexpr auto upper = code_point.if_<_upper>();

struct _alpha
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.alphabetic";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(alphabetic)
};
inline constexpr auto alpha = code_point.if_<_alpha>();

//=== digit ===//
inline constexpr auto digit = code_point.general_category<lexy::code_point::decimal_number>();

struct _alnum
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.alphabetic-decimal";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return _alpha{}(cp) || cp.general_category() == lexy::code_point::decimal_number;
    }
};
inline constexpr auto alnum       = code_point.if_<_alnum>();
inline constexpr auto alpha_digit = alnum;

struct _word
{
    struct _join
    {
        LEXY_UNICODE_PROPERTY_PREDICATE(join_control)
    };

    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.word";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return _alnum{}(cp) || cp.general_category() == lexy::code_point::mark
               || cp.general_category() == lexy::code_point::connector_punctuation || _join{}(cp);
    }
};
inline constexpr auto word = code_point.if_<_word>();

//=== categories ===//
struct _graph
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.graph";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // everything that isn't control, surrogate, unassigned, or space.
        return !cp.is_control() && !cp.is_surrogate()
               && cp.general_category() != lexy::code_point::unassigned && !_space{}(cp);
    }
};
inline constexpr auto graph = code_point.if_<_graph>();

struct _print
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.print";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        // blank or graph without control
        return !cp.is_control() && (_blank{}(cp) || _graph{}(cp));
    }
};
inline constexpr auto print = code_point.if_<_print>();

struct _char
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.character";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return cp.general_category() != lexy::code_point::unassigned;
    }
};
inline constexpr auto character = code_point.if_<_char>();
} // namespace lexyd::unicode

namespace lexyd::unicode
{
struct _xid_start
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.XID-start";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(xid_start)
};
inline constexpr auto xid_start = code_point.if_<_xid_start>();

struct _xid_start_underscore
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.XID-start-underscore";
    }

    LEXY_UNICODE_CONSTEXPR bool operator()(lexy::code_point cp) const
    {
        return cp.value() == '_' || _xid_start{}(cp);
    }
};
inline constexpr auto xid_start_underscore = code_point.if_<_xid_start_underscore>();

struct _xid_continue
{
    static LEXY_CONSTEVAL auto name()
    {
        return "code-point.XID-continue";
    }

    LEXY_UNICODE_PROPERTY_PREDICATE(xid_continue)
};
inline constexpr auto xid_continue = code_point.if_<_xid_continue>();
} // namespace lexyd::unicode

#undef LEXY_UNICODE_PROPERTY_PREDICATE

#endif // LEXY_DSL_UNICODE_HPP_INCLUDED

// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED




namespace lexyd
{
template <typename Condition>
struct _until_eof : token_base<_until_eof<Condition>, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            while (true)
            {
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

            end = reader.position();
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
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            while (true)
            {
                // Try to parse the condition.
                if (lexy::try_match_token(Condition{}, reader))
                {
                    // It did match, we're done at that end.
                    end = reader.position();
                    return true;
                }

                // Check whether we've reached the end of the input.
                // We need to do it after checking for condition, as the condition might just accept
                // EOF.
                if (reader.peek() == Reader::encoding::eof())
                {
                    // It did, so we did not succeed.
                    end = reader.position();
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

            reader.set_position(end);
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


// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BUFFER_HPP_INCLUDED
#define LEXY_INPUT_BUFFER_HPP_INCLUDED

#include <cstring>





namespace lexy
{
// The reader used by the buffer if it can use a sentinel.
template <typename Encoding>
class _br
{
public:
    using encoding = Encoding;
    using iterator = const typename Encoding::char_type*;

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

    void set_position(iterator new_pos) noexcept
    {
        _cur = new_pos;
    }

private:
    iterator _cur;
};

// We use aliases for the three encodings that can actually use it.
// (i.e. where char_type == int_type).
LEXY_INSTANTIATION_NEWTYPE(_bra, _br, lexy::ascii_encoding);
LEXY_INSTANTIATION_NEWTYPE(_br8, _br, lexy::utf8_encoding);
LEXY_INSTANTIATION_NEWTYPE(_br32, _br, lexy::utf32_encoding);

// Create the appropriate buffer reader.
template <typename Encoding>
constexpr auto _buffer_reader(const typename Encoding::char_type* data)
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
        return _bra(data);
    else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
        return _br8(data);
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
            return _buffer_reader<encoding>(_data);
        else
            return _range_reader<encoding>(_data, _data + _size);
    }

private:
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

//=== convenience typedefs ===//
template <typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_lexeme = lexeme_for<buffer<Encoding, MemoryResource>>;

template <typename Tag, typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_error = error_for<buffer<Encoding, MemoryResource>, Tag>;

template <typename Production, typename Encoding = default_encoding, typename MemoryResource = void>
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
// Advances the iterator to the beginning of the next code point.
template <typename Encoding, typename Iterator>
constexpr Iterator find_cp_boundary(Iterator cur, Iterator end)
{
    // [maybe_unused] c breaks MSVC builds targetting C++17
    auto is_cp_boundary = [](auto c) {
        (void)c;
        if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
            return (c & 0b1100'0000) != (0b10 << 6);
        else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
            return (c & 0b1111'1100'0000'0000) != (0b110111 << 10);
        else
            // This encoding doesn't have continuation code units, so everything is a boundary.
            return std::true_type{};
    };

    while (cur != end && !is_cp_boundary(*cur))
        ++cur;
    return cur;
}

// Split the context of the location into three parts: the one before underlined, the underlined
// one, and the one after. If underlined covers multiple lines, limit to the one of the context or
// the newline afterwards.
template <typename Location, typename Reader>
constexpr auto split_context(const Location& location, const lexy::lexeme<Reader>& underlined)
{
    using encoding     = typename Reader::encoding;
    const auto context = location.context();

    struct result_t
    {
        lexy::lexeme<Reader> before;
        lexy::lexeme<Reader> underlined;
        lexy::lexeme<Reader> after;
    } result;

    // The before part starts at the context and continues until the beginning of the underline.
    // We do not advance the beginning of the underline to the next code point boundary:
    // If the error occurs inside a code point, this should be visible.
    result.before = {context.begin(), underlined.begin()};

    // Check whether we have underlined.begin() > context.end().
    auto underline_after_end
        = lexy::_detail::min_range_end(context.begin(), underlined.begin(), context.end())
          == context.end();
    if (underline_after_end)
    {
        // The underlined part is inside the newline.
        auto newline = location.newline();
        LEXY_PRECONDITION(lexy::_detail::precedes(newline.begin(), underlined.begin()));

        auto underlined_end = [&] {
            if (underlined.empty())
            {
                // Our underlined part is empty, extend it to cover one code unit.
                // For simplicity, we extend it further so that it covers the entire newline.
                return newline.end();
            }
            else
            {
                // The end of the underlined part is either the end of the underline or the newline.
                // Due to the nature of newlines, we don't need to advance to the next code point
                // boundary.
                return lexy::_detail::min_range_end(underlined.begin(), underlined.end(),
                                                    newline.end());
            }
        }();

        // Use the trimmed/extended underline, and nothing comes after it.
        result.underlined = {underlined.begin(), underlined_end};
        result.after      = {underlined_end, underlined_end};
    }
    else
    {
        auto underlined_end = [&] {
            if (underlined.empty())
            {
                LEXY_ASSERT(underlined.end() != context.end(),
                            "we would have triggered underline_after_end");
                return find_cp_boundary<encoding>(lexy::_detail::next(underlined.end()),
                                                  context.end());
            }
            else
            {
                // Trim the underlined part, so it does not extend the context.
                auto trimmed = lexy::_detail::min_range_end(underlined.begin(), underlined.end(),
                                                            context.end());
                if (trimmed == context.end())
                    return trimmed;

                // If we haven't trimmed it, we need to advance it to the next code point boundary.
                // This also prevents an empty underline.
                // (We assume the context ends at a code point boundary, so don't need to trim it
                // above.)
                return find_cp_boundary<encoding>(underlined.end(), context.end());
            }
        }();

        // Use the trimmed/extended underline, everything that remains (if any) is part of the
        // context.
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
            // This is only possible if we have an error right at EOF.
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
        auto underlined = lexy::lexeme_for<Input>(error.position(), error.index() + 1);

        out = writer.write_annotation(out, annotation_kind::primary, location, underlined,
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
        auto underlined = lexy::lexeme_for<Input>(error.begin(), error.end());

        out = writer.write_annotation(out, annotation_kind::primary, location, underlined,
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected keyword '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
    {
        auto underlined = lexy::lexeme_for<Input>(error.position(), error.position());
        out = writer.write_annotation(out, annotation_kind::primary, location, underlined,
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
        auto underlined = lexy::lexeme_for<Input>(error.begin(), error.end());
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
            if (_count != 0)
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

