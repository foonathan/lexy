// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEQUENCE_HPP_INCLUDED
#define LEXY_DSL_SEQUENCE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename... R>
struct _seq_impl;
template <>
struct _seq_impl<>
{
    template <typename NextParser>
    struct parser : NextParser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto _try_parse(Context& context, Reader& reader, Reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return static_cast<lexy::rule_try_parse_result>(
                NextParser::parse(context, reader, LEXY_FWD(args)...));
        }
    };
};
template <typename H, typename... T>
struct _seq_impl<H, T...>
{
    template <typename NextParser>
    struct parser : lexy::rule_parser<H, lexy::rule_parser<_seq_impl<T...>, NextParser>>
    {
        // Called by another _seq_impl instantiation.
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto _try_parse(Context& context, Reader& reader, Reader save, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            // We can safely discard, token does not produce any values.
            using token_parser = lexy::rule_parser<H, lexy::context_discard_parser<Context>>;
            auto result        = token_parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::ok)
            {
                // Continue trying the branch.
                using continuation = lexy::rule_parser<_seq_impl<T...>, NextParser>;
                return continuation::_try_parse(context, reader, save, LEXY_FWD(args)...);
            }
            else if (result == lexy::rule_try_parse_result::backtracked)
            {
                // Backtrack.
                reader = LEXY_MOV(save);
                return lexy::rule_try_parse_result::backtracked;
            }
            else
            {
                // Canceled.
                return lexy::rule_try_parse_result::canceled;
            }
        }

        // Only needed in the first instantiation.
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto save = reader;
            return _try_parse(context, reader, save, LEXY_FWD(args)...);
        }
    };
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);

    static constexpr auto is_branch               = (lexy::is_token<R> && ...);
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    using parser = lexy::rule_parser<_seq_impl<R...>, NextParser>;
};

template <typename R, typename S>
constexpr auto operator+(R, S)
{
    return _seq<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator+(_seq<R...>, S)
{
    return _seq<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator+(R, _seq<S...>)
{
    return _seq<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator+(_seq<R...>, _seq<S...>)
{
    return _seq<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEQUENCE_HPP_INCLUDED

