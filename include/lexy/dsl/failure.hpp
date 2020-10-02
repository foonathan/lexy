// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_FAILURE_HPP_INCLUDED
#define LEXY_DSL_FAILURE_HPP_INCLUDED

#include <lexy/_detail/type_name.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/condition.hpp>
#include <lexy/dsl/option.hpp>

namespace lexyd
{
template <typename Tag>
struct _fail : atom_base<_fail<Tag>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader&)
    {
        return false;
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        return typename lexy::error<Reader, Tag>(pos);
    }
};

/// Matches nothing, produces a failure with the given tag.
template <typename Tag>
constexpr auto failure = _fail<Tag>{};
} // namespace lexyd

namespace lexyd
{
/// Requires that lookahead will match a pattern at a location.
template <typename Tag, typename Pattern>
LEXY_CONSTEVAL auto require(Pattern pattern)
{
    // If we don't get the pattern, we create a failure.
    // Otherwise, we match the empty string.
    return opt(unless(pattern) >> failure<Tag>);
}

/// Requires that lookahead does not match a pattern at a location.
template <typename Tag, typename Pattern>
LEXY_CONSTEVAL auto prevent(Pattern pattern)
{
    // Same as above, but we don't want to match the pattern.
    return opt(if_(pattern) >> failure<Tag>);
}
} // namespace lexyd

#endif // LEXY_DSL_FAILURE_HPP_INCLUDED
