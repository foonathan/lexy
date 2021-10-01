// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
struct _eof : token_base<_eof>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(const Reader& reader)
        {
            return reader.peek() == Reader::encoding::eof();
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(this->end, "EOF");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_eof> = lexy::eof_token_kind;
}

#endif // LEXY_DSL_EOF_HPP_INCLUDED

