// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
struct expected_newline
{
    template <typename Input>
    class error
    {
    public:
        constexpr explicit error(typename Input::iterator pos) noexcept : _pos(pos) {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

    private:
        typename Input::iterator _pos;
    };
};
} // namespace lexy

namespace lexyd
{
struct _nl : atom_base<_nl>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input)
    {
        if (auto cur = input.peek(); cur == Input::encoding::to_int_type('\n'))
        {
            input.bump();
            return true;
        }
        else if (cur == Input::encoding::to_int_type('\r'))
        {
            input.bump();
            if (input.peek() == Input::encoding::to_int_type('\n'))
            {
                input.bump();
                return true;
            }

            return false; // Single '\r' not allowed.
        }
        else
            return false;
    }

    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input&, typename Input::iterator pos)
    {
        return lexy::expected_newline::error<Input>(pos);
    }
};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED
