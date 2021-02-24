// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <typename Rule>
struct _cap : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        template <typename... PrevArgs>
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, PrevArgs&&... prev_args,
                                     typename Reader::iterator begin, Args&&... args)
            {
                return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                         lexy::lexeme(reader, begin), LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            auto begin = reader.cur();

            using continuation = _continuation<Args...>;
            return lexy::rule_parser<Rule, continuation>::try_parse(context, reader,
                                                                    LEXY_FWD(args)..., begin);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.cur();

            using continuation = _continuation<Args...>;
            return lexy::rule_parser<Rule, continuation>::parse(context, reader, LEXY_FWD(args)...,
                                                                begin);
        }
    };

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

