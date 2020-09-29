// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/while.hpp>

namespace lexyd
{
template <typename Rule, typename Whitespace>
struct _ws : rule_base
{
    using _whitespace = decltype(while_(Whitespace{}));

    static constexpr auto has_matcher = Rule::has_matcher;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            _whitespace::matcher::match(reader);
            return Rule::matcher::match(reader);
        }
    };

    template <typename NextParser>
    using parser =
        typename _whitespace::template parser<typename Rule::template parser<NextParser>>;
};

/// Capturing a whitespaced rule doesn't capture the whitespace.
template <typename Rule, typename Whitespace>
LEXY_CONSTEVAL auto capture(_ws<Rule, Whitespace>)
{
    return _ws<decltype(capture(Rule{})), Whitespace>{};
}
template <typename T, typename Rule, typename Whitespace>
LEXY_CONSTEVAL auto capture(_ws<Rule, Whitespace>)
{
    return _ws<decltype(lexy::dsl::capture<T>(Rule{})), Whitespace>{};
}

/// Matches whitespace before parsing rule.
template <typename Rule, typename Whitespace>
LEXY_CONSTEVAL auto whitespaced(Rule, Whitespace)
{
    static_assert(lexy::is_pattern<Whitespace>, "whitespace must be a pattern");
    return _ws<Rule, Whitespace>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED

