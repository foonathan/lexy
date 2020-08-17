// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
#define LEXY_DSL_ALTERNATIVE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
struct exhausted_alternatives
{
    template <typename Input>
    class error
    {
    public:
        constexpr explicit error(typename Input::iterator pos) noexcept : _pos(pos) {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

    private:
        typename Input::iterator _pos;
    };
};
} // namespace lexy

namespace lexyd
{
template <typename... P>
struct _alt : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Input>
        LEXY_DSL_FUNC bool match(Input& input)
        {
            return (P::matcher::match(input) || ...);
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
                    lexy::exhausted_alternatives::error<Input>(input.cur()));
        }
    };

    //=== dsl ===//
    template <typename Other>
    friend LEXY_CONSTEVAL auto operator/(_alt<P...>, Other)
    {
        static_assert(lexy::is_pattern<Other>);
        return _alt<P..., Other>{};
    }
    template <typename Other>
    friend LEXY_CONSTEVAL auto operator/(Other, _alt<P...>)
    {
        static_assert(lexy::is_pattern<Other>);
        return _alt<Other, P...>{};
    }
};

template <typename P1, typename P2>
LEXY_CONSTEVAL auto operator/(P1, P2)
{
    static_assert(lexy::is_pattern<P1> && lexy::is_pattern<P2>);
    return _alt<P1, P2>{};
}
template <typename... P, typename... Q>
LEXY_CONSTEVAL auto operator/(_alt<P...>, _alt<Q...>)
{
    return _alt<P..., Q...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
