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
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted choice";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename... R>
struct _chc_matcher
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader&)
    {
        return false;
    }
};
template <typename H, typename... T>
struct _chc_matcher<H, T...>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        using as_branch = decltype(branch(H{}));

        if (auto save = reader; as_branch::condition_matcher::match(reader))
        {
            if (as_branch::then_matcher::match(reader))
                return true;
            else
            {
                reader = LEXY_MOV(save);
                return false;
            }
        }

        return _chc_matcher<T...>::match(reader);
    }
};

template <typename NextParser, typename... R>
struct _chc_parser;
template <typename NextParser>
struct _chc_parser<NextParser>
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&...) ->
        typename Handler::result_type
    {
        auto e = lexy::make_error<Reader, lexy::exhausted_choice>(reader.cur());
        return LEXY_MOV(handler).error(reader, e);
    }
};
template <typename NextParser, typename H, typename... T>
struct _chc_parser<NextParser, H, T...>
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
        typename Handler::result_type
    {
        using as_branch = decltype(branch(H{}));
        if constexpr (as_branch::is_unconditional)
            return as_branch::template then_parser<NextParser>::parse(handler, reader,
                                                                      LEXY_FWD(args)...);
        else
        {
            if (as_branch::condition_matcher::match(reader))
                return as_branch::template then_parser<NextParser>::parse(handler, reader,
                                                                          LEXY_FWD(args)...);
            else
                return _chc_parser<NextParser, T...>::parse(handler, reader, LEXY_FWD(args)...);
        }
    }
};

template <typename... R>
struct _chc : rule_base
{
    static constexpr auto has_matcher = (decltype(branch(R()).then())::has_matcher&&...);

    using matcher = _chc_matcher<R...>;

    template <typename NextParser>
    using parser = _chc_parser<NextParser, R...>;
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator|(R, S)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R, S>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator|(_chc<R...>, S)
{
    static_assert(lexy::is_branch_rule<S>, "choice requires a branch condition");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator|(R, _chc<S...>)
{
    static_assert(lexy::is_branch_rule<R>, "choice requires a branch condition");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

