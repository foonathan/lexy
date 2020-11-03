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
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted alternatives";
    }
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
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            return (P::matcher::match(reader) || ...);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (matcher::match(reader))
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            else
            {
                auto e = lexy::make_error<Reader, lexy::exhausted_alternatives>(reader.cur());
                return LEXY_MOV(handler).error(reader, e);
            }
        }
    };
};

template <typename R, typename S>
LEXY_CONSTEVAL auto operator/(R, S)
{
    static_assert(lexy::is_pattern<R>);
    static_assert(lexy::is_pattern<S>);
    return _alt<R, S>{};
}
template <typename... R, typename S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, S)
{
    static_assert(lexy::is_pattern<S>);
    return _alt<R..., S>{};
}
template <typename R, typename... S>
LEXY_CONSTEVAL auto operator/(R, _alt<S...>)
{
    static_assert(lexy::is_pattern<R>);
    return _alt<R, S...>{};
}
template <typename... R, typename... S>
LEXY_CONSTEVAL auto operator/(_alt<R...>, _alt<S...>)
{
    return _alt<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ALTERNATIVE_HPP_INCLUDED
