// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
// TODO: make public
template <typename Token>
struct _capt : _copy_base<Token>
{
    template <typename Context, typename Reader>
    struct bp
    {
        lexy::token_parser_for<Token, Reader> token;

        constexpr auto try_parse(Context&, const Reader& reader)
        {
            return token.try_parse(reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, Token{}, begin, token.end);
            reader.set_position(token.end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, token.end));
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Context, Reader> impl{};
            if (!impl.try_parse(context, reader))
            {
                impl.token.report_error(context, reader);
                return false;
            }

            return impl.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Rule>
struct _cap : _copy_base<Rule>
{
    template <typename NextParser, typename... PrevArgs>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           PrevArgs&&... prev_args, typename Reader::iterator begin,
                                           Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(prev_args)...,
                                     lexy::lexeme(reader, begin), LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Forward to the rule.
            return rule.try_parse(context, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using continuation = _pc<NextParser, Args...>;
            return rule.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                      reader.position());
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using parser = lexy::parser_for<Rule, _pc<NextParser, Args...>>;
            return parser::parse(context, reader, LEXY_FWD(args)..., reader.position());
        }
    };
};

/// Captures whatever the rule matches as a lexeme.
template <typename Rule>
constexpr auto capture(Rule)
{
    return _cap<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED

