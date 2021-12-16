// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED

#include <lexy/_detail/code_point.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

//=== lit ===//
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
                    = ((reader.peek() == lexy::_detail::transcode_int<typename Reader::encoding>(C)
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
template <char32_t Cp>
struct _lcp : token_base<_lcp<Cp>>
{
    template <typename Encoding>
    static auto _string_impl()
    {
        using char_type = typename Encoding::char_type;

        constexpr struct data_t
        {
            char_type   str[4];
            std::size_t length;

            constexpr data_t()
            : str{},
              length(lexy::_detail::encode_code_point<Encoding>(lexy::code_point(Cp), str, 4))
            {}
        } data;

        if constexpr (data.length == 1)
            return lexy::_detail::type_string<char_type, data.str[0]>{};
        else if constexpr (data.length == 2)
            return lexy::_detail::type_string<char_type, data.str[0], data.str[1]>{};
        else if constexpr (data.length == 3)
            return lexy::_detail::type_string<char_type, data.str[0], data.str[1], data.str[2]>{};
        else
            return lexy::_detail::type_string<char_type, data.str[0], data.str[1], data.str[2],
                                              data.str[3]>{};
    }
    template <typename Encoding>
    using _string = decltype(_string_impl<Encoding>());

    template <typename Reader,
              typename Indices
              = lexy::_detail::make_index_sequence<_string<typename Reader::encoding>::size>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            using encoding     = typename Reader::encoding;
            constexpr auto str = _string<encoding>::template c_str<>;

            auto result
                // Compare each code unit, bump on success, cancel on failure.
                = ((reader.peek() == encoding::to_int_type(str[Idx]) ? (reader.bump(), true)
                                                                     : false)
                   && ...);
            end = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using encoding     = typename Reader::encoding;
            constexpr auto str = _string<encoding>::template c_str<>;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, this->end);
            auto err   = lexy::error<Reader, lexy::expected_literal>(begin, str, index);
            context.on(_ev::error{}, err);
        }
    };
};

template <char32_t CodePoint>
constexpr auto lit_cp = _lcp<CodePoint>{};
} // namespace lexyd

namespace lexy
{
template <char32_t Cp>
constexpr auto token_kind_of<lexy::dsl::_lcp<Cp>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED

