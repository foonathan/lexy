// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONDITION_HPP_INCLUDED
#define LEXY_DSL_CONDITION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
template <bool Expected, typename Input>
struct _expected_pattern_error
{
public:
    constexpr explicit _expected_pattern_error(typename Input::iterator pos) noexcept : _pos(pos) {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

private:
    typename Input::iterator _pos;
};

struct expected_pattern
{
    template <typename Input>
    using error = _expected_pattern_error<true, Input>;
};

struct unexpected_pattern
{
    template <typename Input>
    using error = _expected_pattern_error<false, Input>;
};
} // namespace lexy

namespace lexyd
{
template <typename Pattern, bool Expected>
struct _if : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Input>
        LEXY_DSL_FUNC bool match(Input& input)
        {
            auto copy = input;
            return Pattern::matcher::match(copy) == Expected;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            if (matcher::match(input))
                return NextParser::parse(context, input, LEXY_FWD(args)...);
            else
                return context.report_error(
                    lexy::_expected_pattern_error<Expected, Input>(input.cur()));
        }
    };
};

template <typename Pattern>
LEXY_CONSTEVAL auto if_(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _if<Pattern, true>{};
}

template <typename Pattern>
LEXY_CONSTEVAL auto unless(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _if<Pattern, false>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONDITION_HPP_INCLUDED

