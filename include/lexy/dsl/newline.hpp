// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

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

