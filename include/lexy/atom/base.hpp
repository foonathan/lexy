// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ATOM_BASE_HPP_INCLUDED
#define LEXY_ATOM_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

#define LEXY_ATOM_FUNC LEXY_FORCE_INLINE static constexpr

// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct atom_base
{};
} // namespace lexyd

#if 0
class Atom : atom_base
{
    // Try to match and consume characters.
    // Returns true, if match succesful and leave input after the consumed characters.
    // Otherwise, return false and leaves input at the error position.
    template <typename Input>
    LEXY_ATOM_FUNC bool match(Input& input);

    // Returns an error object describing the error.
    // The input is in the state the match function left it, `pos` is the position before calling match.
    template <typename Input>
    LEXY_ATOM_FUNC auto error(const Input& input, typename Input::iterator pos);
};
#endif

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_atom = std::is_base_of_v<dsl::atom_base, T>;
} // namespace lexy

#endif // LEXY_ATOM_BASE_HPP_INCLUDED

