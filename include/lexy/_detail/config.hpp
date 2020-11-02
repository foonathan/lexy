// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_CONFIG_HPP_INCLUDED
#define LEXY_DETAIL_CONFIG_HPP_INCLUDED

#include <cstddef>
#include <type_traits>

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
#    if __cpp_nontype_template_parameter_class
#        define LEXY_HAS_NTTP 1
#    else
#        define LEXY_HAS_NTTP 0
#    endif
#endif

#if LEXY_HAS_NTTP
#    define LEXY_NTTP(T) T
#    define LEXY_NTTP_TYPE_OF(Nttp) decltype(Nttp)
#else
#    define LEXY_NTTP(T) const T&
#    define LEXY_NTTP_TYPE_OF(Nttp) std::remove_cv_t<std::remove_reference_t<decltype(Nttp)>>
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

#else

namespace lexy
{
using _char8_t = char;
} // namespace lexy

#    define LEXY_CHAR8_T ::lexy::_char8_t

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
#    else
#        error "unknown endianness"
#    endif
#endif

//=== force inline ===//
#ifndef LEXY_FORCE_INLINE
#    if defined(__has_cpp_attribute) && __has_cpp_attribute(gnu::always_inline)
#        define LEXY_FORCE_INLINE [[gnu::always_inline]]
#    else
#        define LEXY_FORCE_INLINE inline
#    endif
#endif

//=== empty_member ===//
#ifndef LEXY_EMPTY_MEMBER
#    if defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
#        define LEXY_HAS_EMPTY_MEMBER 1
#        define LEXY_EMPTY_MEMBER [[no_unique_address]]
#    else
#        define LEXY_HAS_EMPTY_MEMBER 0
#        define LEXY_EMPTY_MEMBER
#    endif
#endif

#endif // LEXY_DETAIL_CONFIG_HPP_INCLUDED

