// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

#if 0
class Atom : atom_base
{
    // Try to match and consume characters.
    // Returns true, if match succesful and leave input after the consumed characters.
    // Otherwise, return false and leaves input at the error position.
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input& input);

    // Returns an error object describing the error.
    // The input is in the state the match function left it, `pos` is the position before calling match.
    template <typename Input>
    LEXY_DSL_FUNC auto error(const Input& input, typename Input::iterator pos);
};
#endif

// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct _atom_base
{};
template <typename Atom>
struct atom_base : _atom_base
{
    struct pattern
    {
        static constexpr std::size_t max_capture_count = 0;

        template <typename Context, typename Input>
        LEXY_DSL_FUNC bool match(Context&, Input& input)
        {
            auto reset = input;
            if (Atom::match(input))
                return true;

            input = LEXY_MOV(reset);
            return false;
        }
    };
};
} // namespace lexyd

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_atom = std::is_base_of_v<dsl::_atom_base, T>;
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED
