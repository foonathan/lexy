// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/trie.hpp>
#include <lexy/token.hpp>

namespace lexyd
{
struct _nl : token_base<_nl>
{
    static constexpr auto _trie
        = lexy::trie<char, LEXY_NTTP_STRING("\n"), LEXY_NTTP_STRING("\r\n")>;
    using token_engine = lexy::engine_trie<_trie>;

    static LEXY_CONSTEVAL auto token_kind()
    {
        return lexy::newline_token_kind;
    }

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "newline");
        context.error(err);
    }

    template <typename Whitespace>
    constexpr auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

namespace lexyd
{
struct _eol : token_base<_eol>
{
    struct token_engine : lexy::engine_matcher_base
    {
        using error_code = _nl::token_engine::error_code;

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            if (reader.eof())
                return error_code();
            else
                return _nl::token_engine::match(reader);
        }
    };

    static LEXY_CONSTEVAL auto token_kind()
    {
        return lexy::eol_token_kind;
    }

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&, token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::expected_char_class>(pos, "EOL");
        context.error(err);
    }

    template <typename Whitespace>
    constexpr auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Matches the end of line (EOF or newline).
constexpr auto eol = _eol{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED

