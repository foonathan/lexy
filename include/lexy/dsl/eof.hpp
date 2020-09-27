// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
struct _eof : atom_base<_eof>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        return reader.peek() == Reader::encoding::eof();
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        return lexy::expected_char_class::error<Reader>(pos, "EOF");
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        static_assert(lexy::is_pattern<Whitespace>, "whitespace must be a pattern");
        return ws + _eof{};
    }
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

#endif // LEXY_DSL_EOF_HPP_INCLUDED
