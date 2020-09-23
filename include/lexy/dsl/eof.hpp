// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _eof : atom_base<_eof>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input)
    {
        return input.peek() == Input::encoding::eof();
    }

    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input&, typename Input::iterator pos)
    {
        return lexy::expected_char_class::error<Input>(pos, "EOF");
    }
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

#endif // LEXY_DSL_EOF_HPP_INCLUDED
