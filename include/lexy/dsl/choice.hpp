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
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator|(R r, S s)
{
    return _chc<decltype(branch(r)), decltype(branch(s))>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator|(_chc<R...>, S s)
{
    return _chc<R..., decltype(branch(s))>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator|(R r, _chc<S...>)
{
    return _chc<decltype(branch(r)), S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

