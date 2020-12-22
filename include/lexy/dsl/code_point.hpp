// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
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

struct _cp_cap : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto save = reader;

            lexy::engine_cp_auto::error_code ec{};
            auto                             result = lexy::engine_cp_auto::parse(ec, reader);
            if (ec == lexy::engine_cp_auto::error_code())
            {
                LEXY_PRECONDITION(result.is_scalar());
                return NextParser::parse(handler, reader, LEXY_FWD(args)..., result);
            }
            else
            {
                reader = LEXY_MOV(save);

                auto name = _cp_name<typename Reader::encoding>();
                auto e    = lexy::make_error<Reader, lexy::expected_char_class>(reader.cur(), name);
                return LEXY_MOV(handler).error(e);
            }
        }
    };
};

struct _cp : token_base<_cp>
{
    using token_engine = lexy::engine_cp_auto;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto name = _cp_name<typename Reader::encoding>();
        auto err  = lexy::make_error<Reader, lexy::expected_char_class>(pos, name);
        return LEXY_MOV(handler).error(err);
    }

    LEXY_CONSTEVAL auto capture() const
    {
        return _cp_cap{};
    }
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp{};
} // namespace lexyd

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED
