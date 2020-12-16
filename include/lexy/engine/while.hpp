// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_WHILE_HPP_INCLUDED
#define LEXY_ENGINE_WHILE_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches `Matcher` as often as possible.
template <typename Matcher>
struct engine_while : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Matcher>);

    enum class error_code
    {
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (engine_try_match<Matcher>(reader))
        {}

        return error_code();
    }
};

template <typename Matcher, typename Reader>
inline constexpr bool engine_can_fail<engine_while<Matcher>, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_WHILE_HPP_INCLUDED

