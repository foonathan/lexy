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
    template <typename OtherCharT>
    LEXY_CONSTEVAL string_literal(const OtherCharT* str) : string{}
    {
        for (auto i = 0u; i != N; ++i)
            string[i] = CharT(str[i]);
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

    template <typename CharT>
    struct _lazy
    {
        static inline constexpr string_literal<N, CharT> str = Str.string;
    };

    template <typename CharT = char_type>
    static LEXY_CONSTEVAL auto get()
    {
        if constexpr (std::is_same_v<CharT, char_type>)
            return basic_string_view<CharT>(Str.string, Str.size());
        else
        {
            constexpr auto str = _lazy<CharT>::str;
            return basic_string_viewCharT > (str.string, str.size());
        }
    }
};

template <auto C>
struct type_char
{
    using char_type = std::decay_t<decltype(C)>;

    template <typename CharT>
    static constexpr auto c = C;

    template <typename CharT>
    static LEXY_CONSTEVAL auto get()
    {
        return basic_string_view<CharT>(&c<CharT>, 1);
    }
};
} // namespace lexy::_detail

#    define LEXY_NTTP_STRING(Str) ::lexy::_detail::type_string<::lexy::_detail::string_literal(Str)>

#else // string<Cs...> implementation

namespace lexy::_detail
{
template <typename CharT, CharT... Cs>
struct type_string
{
    using char_type = CharT;

    template <typename OtherCharT>
    struct _lazy
    {
        static inline constexpr OtherCharT str[] = {OtherCharT(Cs)...};
    };

    template <typename OtherCharT = char_type>
    static LEXY_CONSTEVAL auto get()
    {
        return basic_string_view<OtherCharT>(_lazy<OtherCharT>::str, sizeof...(Cs));
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
#        define LEXY_NTTP_STRING64(Str, I)                                                         \
            ::lexy::_detail::cat<LEXY_NTTP_STRING32(Str, I), LEXY_NTTP_STRING32(Str, I + 32)>
#        define LEXY_NTTP_STRING128(Str, I)                                                        \
            ::lexy::_detail::cat<LEXY_NTTP_STRING64(Str, I), LEXY_NTTP_STRING64(Str, I + 64)>
#        define LEXY_NTTP_STRING256(Str, I)                                                        \
            ::lexy::_detail::cat<LEXY_NTTP_STRING128(Str, I), LEXY_NTTP_STRING128(Str, I + 128)>

// instantiate with overflow check
#        define LEXY_NTTP_STRING(Str)                                                              \
            ::lexy::_detail::check_size<LEXY_NTTP_STRING256(Str, 0), LEXY_NTTP_STRING_LENGTH(Str), \
                                        256>::type

#    endif

#endif

#endif // LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

