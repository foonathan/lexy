// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SUCCESS_HPP_INCLUDED
#define LEXY_DSL_SUCCESS_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _suc : atom_base<_suc>
{
    template <typename Input>
    LEXY_DSL_FUNC bool match(Input&)
    {
        return true;
    }

    template <typename Input>
    LEXY_DSL_FUNC void error(const Input&, typename Input::iterator);
};

/// Matches the empty string, always succeeds.
constexpr auto success = _suc{};
} // namespace lexyd

#endif // LEXY_DSL_SUCCESS_HPP_INCLUDED
