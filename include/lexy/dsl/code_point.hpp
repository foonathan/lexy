// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/_detail/code_point.hpp>
#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <char32_t Cp>
struct _cpl : token_base<_cpl<Cp>>
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

namespace lexy
{
template <char32_t Cp>
constexpr auto token_kind_of<lexy::dsl::_cpl<Cp>> = lexy::literal_token_kind;

// The void-version without predicate logically matches any input (modulo encoding errors, of
// course).
template <>
inline constexpr auto token_kind_of<lexy::dsl::_cp<void>> = lexy::any_token_kind;
} // namespace lexy

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

