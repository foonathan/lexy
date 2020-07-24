// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_CAPTURE_HPP_INCLUDED
#define LEXY_PATTERN_CAPTURE_HPP_INCLUDED

#include <lexy/pattern/base.hpp>

namespace lexyd
{
template <typename Pattern>
struct _cap : pattern_base
{
    static constexpr auto max_capture_count = 1 + Pattern::max_capture_count;

    template <typename Context, typename Input>
    LEXY_PATTERN_FUNC bool match(Context& context, Input& input)
    {
        auto idx = context._capture_count++;

        auto begin = input.cur();
        if (!Pattern::match(context, input))
        {
            context._capture_count--; // Undo the increment.
            return false;
        }
        auto end = input.cur();

        context._captures[idx] = {begin, end};
        return true;
    }
};

/// Captures whatever the pattern matches as a lexeme.
template <typename Pattern>
LEXY_CONSTEVAL auto capture(Pattern p)
{
    return _cap<decltype(pattern(p))>{};
}
} // namespace lexyd

#endif // LEXY_PATTERN_CAPTURE_HPP_INCLUDED
