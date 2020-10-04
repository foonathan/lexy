// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <template <typename Reader> typename Lexeme, typename Rule, typename NextParser>
struct _cap_parser
{
    template <typename... PrevArgs>
    struct _continuation
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, typename Reader::iterator begin,
                                 PrevArgs&&... prev_args, Args&&... args) ->
            typename Handler::result_type
        {
            auto end = reader.cur();
            return NextParser::parse(handler, reader, LEXY_FWD(prev_args)...,
                                     Lexeme<Reader>(begin, end), LEXY_FWD(args)...);
        }
    };

    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
        typename Handler::result_type
    {
        using continuation = _continuation<Args...>;
        return Rule::template parser<continuation>::parse(handler, reader, reader.cur(),
                                                          LEXY_FWD(args)...);
    }
};

template <typename Rule>
struct _cap : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    using parser = _cap_parser<lexy::lexeme, Rule, NextParser>;
};

/// Captures whatever the rule matches as a lexeme.
template <typename Rule>
LEXY_CONSTEVAL auto capture(Rule)
{
    return _cap<Rule>{};
}

template <typename T, typename Rule>
struct _capa : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename>
    using _lexeme = T;

    template <typename NextParser>
    using parser = _cap_parser<_lexeme, Rule, NextParser>;
};

/// Captures whatever the rule matches as the specified type.
template <typename T, typename Rule>
LEXY_CONSTEVAL auto capture(Rule)
{
    return _capa<T, Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED
