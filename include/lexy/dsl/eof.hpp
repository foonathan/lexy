// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/eof.hpp>
#include <lexy/token.hpp>

namespace lexyd
{
struct _eof : token_base<_eof>
{
    using token_engine = lexy::engine_eof;

    static LEXY_CONSTEVAL auto token_kind()
    {
        return lexy::eof_token_kind;
    }

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::expected_char_class>(pos, "EOF");
        context.on(_ev::error{}, err);
    }
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

#endif // LEXY_DSL_EOF_HPP_INCLUDED

