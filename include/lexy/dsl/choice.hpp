// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CHOICE_HPP_INCLUDED
#define LEXY_DSL_CHOICE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct exhausted_choice
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted choice";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename NextParser, typename... R>
struct _chc_parser;
template <typename NextParser>
struct _chc_parser<NextParser>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context&, Reader&, Args&&...) -> lexy::rule_try_parse_result
    {
        // We couldn't match any alternative of the choice, so as a branch we backtrack.
        return lexy::rule_try_parse_result::backtracked;
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&...)
    {
        auto err = lexy::error<Reader, lexy::exhausted_choice>(reader.position());
        context.on(_ev::error{}, err);
        return false;
    }
};
template <typename NextParser, typename H, typename... T>
struct _chc_parser<NextParser, H, T...>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
        -> lexy::rule_try_parse_result
    {
        if constexpr (H::is_unconditional_branch)
        {
            // We know that we must take H, parse it and translate the result.
            if (lexy::rule_parser<H, NextParser>::parse(context, reader, LEXY_FWD(args)...))
                return lexy::rule_try_parse_result::ok;
            else
                return lexy::rule_try_parse_result::canceled;
        }
        else
        {
            // Try to parse H.
            auto result
                = lexy::rule_parser<H, NextParser>::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Try the next branch of the choice instead.
                return _chc_parser<NextParser, T...>::try_parse(context, reader, LEXY_FWD(args)...);
            else
                // We've taken H, return its result.
                return result;
        }
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if constexpr (H::is_unconditional_branch)
        {
            // We know that we must take H, so parse it.
            return lexy::rule_parser<H, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Try to parse H.
            auto result
                = lexy::rule_parser<H, NextParser>::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Try the next branch of the choice instead.
                return _chc_parser<NextParser, T...>::parse(context, reader, LEXY_FWD(args)...);
            else
                // We've taken H, return its translated result.
                return static_cast<bool>(result);
        }
    }
};

template <typename... R>
struct _chc : rule_base
{
    static constexpr auto _would_be_unconditional_branch = (R::is_unconditional_branch || ...);

    // We only make it a choice if it's not an unconditional branch;
    // this is almost surely a bug.
    static constexpr auto is_branch              = !_would_be_unconditional_branch;
    static constexpr auto is_unconditonal_branch = false;

    template <typename NextParser>
    using parser = _chc_parser<NextParser, R...>;
};

template <typename R, typename S>
constexpr auto operator|(R, S)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator|(_chc<R...>, S)
{
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator|(R, _chc<S...>)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

