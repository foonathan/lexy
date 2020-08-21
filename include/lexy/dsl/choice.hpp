// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CHOICE_HPP_INCLUDED
#define LEXY_DSL_CHOICE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexy
{
struct exhausted_choice
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
template <typename NextParser, typename... B>
struct _chc_parser;
template <typename NextParser>
struct _chc_parser<NextParser>
{
    template <typename Context, typename Input, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&...) ->
        typename Context::result_type
    {
        return context.report_error(input, lexy::exhausted_choice::error<Input>(input.cur()));
    }
};
template <typename NextParser, typename H, typename... T>
struct _chc_parser<NextParser, H, T...>
{
    template <typename Context, typename Input, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
        typename Context::result_type
    {
        if constexpr (H::is_unconditional)
            return H::template then_parser<NextParser>::parse(context, input, LEXY_FWD(args)...);
        else
        {
            if (H::condition_matcher::match(input))
                return H::template then_parser<NextParser>::parse(context, input,
                                                                  LEXY_FWD(args)...);
            else
                return _chc_parser<NextParser, T...>::parse(context, input, LEXY_FWD(args)...);
        }
    }
};

template <typename... B>
struct _chc : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = _chc_parser<NextParser, B...>;

    //=== dsl ===//
    template <typename Other>
    friend LEXY_CONSTEVAL auto operator|(_chc<B...>, Other other)
    {
        auto b = branch(other);
        return _chc<B..., decltype(b)>{};
    }
    template <typename Other>
    friend LEXY_CONSTEVAL auto operator|(Other other, _chc<B...>)
    {
        auto b = branch(other);
        return _chc<decltype(b), B...>{};
    }
};

template <typename R1, typename R2>
LEXY_CONSTEVAL auto operator|(R1 r1, R2 r2)
{
    auto b1 = branch(r1);
    auto b2 = branch(r2);
    return _chc<decltype(b1), decltype(b2)>{};
}
template <typename... B, typename... C>
LEXY_CONSTEVAL auto operator|(_chc<B...>, _chc<C...>)
{
    return _chc<B..., C...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

