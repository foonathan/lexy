// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONDITION_HPP_INCLUDED
#define LEXY_DSL_CONDITION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Pattern, bool Expected>
struct _if : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto copy = reader;
            return Pattern::matcher::match(copy) == Expected;
        }
    };

    // As a parser, we don't actually do anything.
    template <typename NextParser>
    using parser = NextParser;
};

template <typename Pattern, bool Expected>
struct _if_dsl
{
    template <typename Rule>
    friend LEXY_CONSTEVAL auto operator>>(_if_dsl, Rule rule)
    {
        return _if<Pattern, Expected>{} >> rule;
    }
};

/// Check if at this reader position, Pattern would match, but don't actually consume any characters
/// if it does.
/// Only used as a branch condition.
template <typename Pattern>
LEXY_CONSTEVAL auto if_(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _if_dsl<Pattern, true>{};
}

/// Check if at this reader position, Pattern would not match, but don't actually consume any
/// characters if it does.
/// Only used as a branch condition.
template <typename Pattern>
LEXY_CONSTEVAL auto unless(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _if_dsl<Pattern, false>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Pattern>
struct _not : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            // We match if we didn't match the pattern.
            // In other words:
            // If we return true, we haven't matched Pattern and haven't consumed any reader.
            // If we return false, we have matched Pattern and have consumed it.
            return !Pattern::matcher::match(reader);
        }
    };

    // As parser, we don't actually do anything.
    template <typename NextParser>
    using parser = NextParser;
};

template <typename Pattern>
struct _not_dsl
{
    template <typename Rule>
    friend LEXY_CONSTEVAL auto operator>>(_not_dsl, Rule rule)
    {
        return _not<Pattern>{} >> rule;
    }
};

/// Check that Pattern doesn't match.
/// This is used for something like `opt(!pattern >> rule)`, which is equivalent to `pattern |
/// rule`.
template <typename Pattern>
LEXY_CONSTEVAL auto operator!(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _not_dsl<Pattern>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONDITION_HPP_INCLUDED

