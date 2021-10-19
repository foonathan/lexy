// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/_detail/code_point.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
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
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp<void>{};
} // namespace lexyd

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

