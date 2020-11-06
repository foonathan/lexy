// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED
#define LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/string_view.hpp>

#if LEXY_HAS_NTTP // string NTTP implementation

namespace lexy::_detail
{
template <std::size_t N, typename CharT>
struct string_literal
{
    CharT string[N];

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

template <auto Str>
struct type_string
{
    using char_type = std::decay_t<decltype(Str.string[0])>;

    static LEXY_CONSTEVAL auto get()
    {
        return basic_string_view<char_type>(Str.string, Str.size());
    }
};

template <auto C>
struct type_char
{
    using char_type = std::decay_t<decltype(C)>;

    static constexpr auto c = C;

    static LEXY_CONSTEVAL auto get()
    {
        return basic_string_view<char_type>(&c, 1);
    }
};
} // namespace lexy::_detail

#    define LEXY_NTTP_STRING(Str) ::lexy::_detail::type_string<::lexy::_detail::string_literal(Str)>

#else // string<Cs...> implementation

namespace lexy::_detail
{
template <auto... Cs>
struct type_string
{
    using char_type = decltype((Cs, ...));

    struct _lazy
    {
        static inline constexpr char_type str[] = {Cs...};
    };

    static LEXY_CONSTEVAL auto get()
    {
        return basic_string_view<char_type>(_lazy::str, sizeof...(Cs));
    }
};

template <auto C>
using type_char = type_string<C>;
} // namespace lexy::_detail

#    if defined(__GNUC__) // string<Cs...> literal implementation

#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wpedantic"
#        ifdef __clang__
#            pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#        endif

template <typename CharT, CharT... Cs>
constexpr ::lexy::_detail::type_string<Cs...> operator""_lexy_string_udl()
{
    return {};
}

#        define LEXY_NTTP_STRING(Str) decltype(Str##_lexy_string_udl)

#        pragma GCC diagnostic pop

#    else // string<Cs...> macro implementation

namespace lexy::string_detail
{
template <typename A, typename B>
struct cat_;
template <auto... C1, auto... C2>
struct cat_<string<C1...>, string<C2...>>
{
    using type = string<C1..., C2...>;
};
template <typename A, typename B>
using cat = typename cat_<A, B>::type;

template <typename T, std::size_t Size, std::size_t MaxSize>
struct check_size
{
    static_assert(Size <= MaxSize, "string out of range");
    using type = T;
};

} // namespace lexy::string_detail

#        define LEXY_NTTP_STRING_LENGTH(Str) (sizeof(Str) / sizeof(Str[0]) - 1)

// extract Ith character if not out of bounds
#        define LEXY_NTTP_STRING1(Str, I)                                                          \
            ::std::conditional_t<(I < LEXY_NTTP_STRING_LENGTH(Str)),                               \
                                 ::lexy::string_detail::string<(                                   \
                                     I >= LEXY_NTTP_STRING_LENGTH(Str) ? Str[0] : Str[I])>,        \
                                 ::lexy::string_detail::string<>>

// recursively split the string in two
#        define LEXY_NTTP_STRING2(Str, I)                                                          \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING1(Str, I), LEXY_NTTP_STRING1(Str, I + 1)>
#        define LEXY_NTTP_STRING4(Str, I)                                                          \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING2(Str, I), LEXY_NTTP_STRING2(Str, I + 2)>
#        define LEXY_NTTP_STRING8(Str, I)                                                          \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING4(Str, I), LEXY_NTTP_STRING4(Str, I + 4)>
#        define LEXY_NTTP_STRING16(Str, I)                                                         \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING8(Str, I), LEXY_NTTP_STRING8(Str, I + 8)>
#        define LEXY_NTTP_STRING32(Str, I)                                                         \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING16(Str, I), LEXY_NTTP_STRING16(Str, I + 16)>
#        define LEXY_NTTP_STRING64(Str, I)                                                         \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING32(Str, I), LEXY_NTTP_STRING32(Str, I + 32)>
#        define LEXY_NTTP_STRING128(Str, I)                                                        \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING64(Str, I), LEXY_NTTP_STRING64(Str, I + 64)>
#        define LEXY_NTTP_STRING256(Str, I)                                                        \
            ::lexy::string_detail::cat<LEXY_NTTP_STRING128(Str, I),                                \
                                       LEXY_NTTP_STRING128(Str, I + 128)>

// instantiate with overflow check
#        define LEXY_NTTP_STRING(Str)                                                              \
            ::lexy::string_detail::check_size<LEXY_NTTP_STRING256(Str, 0),                         \
                                              LEXY_NTTP_STRING_LENGTH(Str), 256>::type

#    endif

#endif

#endif // LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

