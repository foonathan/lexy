// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

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
    template <typename Reader>
    LEXY_DSL_FUNC lexy::code_point _parse(Reader& reader)
    {
        using decoder_t = typename Reader::encoding::code_point_decoder;
        decoder_t decoder;

        switch (decoder.init(reader.peek()))
        {
        case 0:
            break;

        case 3:
            reader.bump();
            if (!decoder.next(reader.peek()))
                return {};
            // fallthrough
        case 2:
            reader.bump();
            if (!decoder.next(reader.peek()))
                return {};
            // fallthrough
        case 1:
            reader.bump();
            if (!decoder.next(reader.peek()))
                return {};
            break;

        case -1:
            return {};
        default:
            LEXY_PRECONDITION(false);
            return {};
        }

        // We only accept valid code points.
        auto cp = LEXY_MOV(decoder).finish();
        if (!cp.is_valid())
            return {};

        // Consume final code unit.
        // This has to be done after the validity check,
        // because for UTF-32, eof creates an invalid code point.
        reader.bump();
        return cp;
    }

    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            auto save = reader;
            if (auto result = _parse(reader); result.is_valid())
            {
                LEXY_PRECONDITION(result.is_scalar());
                return NextParser::parse(handler, reader, LEXY_FWD(args)..., result);
            }
            else
            {
                reader = LEXY_MOV(save);

                auto name = _cp_name<typename Reader::encoding>();
                auto e    = lexy::make_error<Reader, lexy::expected_char_class>(reader.cur(), name);
                return LEXY_MOV(handler).error(reader, e);
            }
        }
    };
};

struct _cp : atom_base<_cp>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        auto cp = _cp_cap::_parse(reader);
        LEXY_PRECONDITION(!cp.is_valid() || cp.is_scalar());
        return cp.is_valid();
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        auto name = _cp_name<typename Reader::encoding>();
        return lexy::make_error<Reader, lexy::expected_char_class>(pos, name);
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
