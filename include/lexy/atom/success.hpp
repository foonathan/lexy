// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ATOM_SUCCESS_HPP_INCLUDED
#define LEXY_ATOM_SUCCESS_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/atom/base.hpp>

namespace lexyd
{
struct _suc : atom_base
{
    template <typename Input>
    LEXY_ATOM_FUNC bool match(Input&)
    {
        return true;
    }

    template <typename Input>
    LEXY_ATOM_FUNC auto error(const Input&, typename Input::iterator)
    {
        LEXY_PRECONDITION(false);
        return nullptr;
    }
};

/// Matches the empty string, always succeeds.
constexpr auto success = _suc{};
} // namespace lexyd

#endif // LEXY_ATOM_SUCCESS_HPP_INCLUDED

