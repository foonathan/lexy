// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <template <typename Reader> typename Lexeme, typename NextParser, typename... PrevArgs>
struct _cap_cont
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, PrevArgs&&... prev_args,
                             typename Reader::iterator begin, Args&&... args) ->
        typename Context::result_type
    {
        auto end = reader.cur();
        return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                 Lexeme<typename Reader::canonical_reader>(begin, end),
                                 LEXY_FWD(args)...);
    }
};

template <template <typename Reader> typename Lexeme, typename Rule, typename NextParser>
struct _cap_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
        typename Context::result_type
    {
        using continuation = _cap_cont<Lexeme, NextParser, Args...>;
        return lexy::rule_parser<Rule, continuation>::parse(context, reader, LEXY_FWD(args)...,
                                                            reader.cur());
    }
};

template <typename Rule>
struct _cap : rule_base
{
    static constexpr auto is_branch = lexy::is_branch<Rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Rule, Reader> _impl;
        typename Reader::iterator          _begin{};

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            _begin = reader.cur();
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            using continuation = _cap_cont<lexy::lexeme, NextParser, Args...>;
            return _impl.template parse<continuation>(context, reader, LEXY_FWD(args)..., _begin);
        }
    };

    template <typename NextParser>
    using parser = _cap_parser<lexy::lexeme, Rule, NextParser>;

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Captures whatever the rule matches as a lexeme.
template <typename Rule>
LEXY_CONSTEVAL auto capture(Rule)
{
    return _cap<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED

