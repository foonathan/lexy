// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_ID_HPP_INCLUDED
#define LEXY_PATTERN_ID_HPP_INCLUDED

#include <lexy/pattern/base.hpp>

namespace lexyd
{
template <auto Id>
struct _id : pattern_base
{
    static_assert(Id > 0);

    static constexpr auto max_capture_count = 0;

    template <typename Context, typename Input>
    LEXY_PATTERN_FUNC bool match(Context& context, Input&)
    {
        context._id = int(Id);
        return true;
    }
};

/// Matches with the specified id.
template <auto Id>
constexpr auto id = _id<Id>{};
} // namespace lexyd

#endif // LEXY_PATTERN_ID_HPP_INCLUDED

