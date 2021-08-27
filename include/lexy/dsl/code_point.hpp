// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/code_point.hpp>

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
            auto err  = lexy::error<Reader, lexy::expected_char_class>(pos, name);
            context.on(_ev::error{}, err);
        }
        else
        {
            auto name = _cp_name<typename Reader::encoding>();
            auto err  = lexy::error<Reader, lexy::expected_char_class>(pos, name);
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

