// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEQUENCE_HPP_INCLUDED
#define LEXY_DSL_SEQUENCE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/engine/success.hpp>

namespace lexyd
{
template <typename NextParser, typename... R>
struct _seq_parser;
template <typename NextParser>
struct _seq_parser<NextParser>
{
    using type = NextParser;
};
template <typename NextParser, typename H, typename... T>
struct _seq_parser<NextParser, H, T...>
{
    using tail = typename _seq_parser<NextParser, T...>::type;
    using type = typename H::template parser<tail>;
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);
    static constexpr auto has_matcher = (R::has_matcher && ...);

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto reset = reader;
            if ((R::matcher::match(reader) && ...))
                return true;

            reader = LEXY_MOV(reset);
            return false;
        }
    };

    static constexpr bool is_branch = has_matcher;

    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            return matcher::match(reader);
        }

        template <typename NextParser, typename Handler, typename... Args>
        constexpr auto parse(Handler& handler, Reader& reader, Args&&... args)
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    using parser = typename _seq_parser<NextParser, R...>::type;
};
template <>
struct _seq<> : token_base<_seq<>>
{
    using token_engine = lexy::engine_success;
};

/// Matches the empty string, always succeeds.
constexpr auto success = _seq<>{};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator+(R, S)
{
    return _seq<R, S>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator+(_seq<R...>, S)
{
    if constexpr (sizeof...(R) == 0)
        return S{};
    else
        return _seq<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator+(R, _seq<S...>)
{
    if constexpr (sizeof...(S) == 0)
        return R{};
    else
        return _seq<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator+(_seq<R...>, _seq<S...>)
{
    if constexpr (sizeof...(R) == 0)
        return _seq<S...>{};
    else if constexpr (sizeof...(S) == 0)
        return _seq<R...>{};
    else
        return _seq<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEQUENCE_HPP_INCLUDED

