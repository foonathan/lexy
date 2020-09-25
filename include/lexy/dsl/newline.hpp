// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _nl : atom_base<_nl>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        if (auto cur = reader.peek(); cur == Reader::encoding::to_int_type('\n'))
        {
            reader.bump();
            return true;
        }
        else if (cur == Reader::encoding::to_int_type('\r'))
        {
            reader.bump();
            if (reader.peek() == Reader::encoding::to_int_type('\n'))
            {
                reader.bump();
                return true;
            }

            return false; // Single '\r' not allowed.
        }
        else
            return false;
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        return lexy::expected_char_class::error<Reader>(pos, "newline");
    }
};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED
