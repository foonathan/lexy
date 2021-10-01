// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PEEK_HPP_INCLUDED
#define LEXY_DSL_PEEK_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy
{
/// We've failed to match a peek.
struct peek_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "peek failure";
    }
};

/// We've failed to match a peek not.
struct unexpected
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Rule, typename Tag>
struct _peek : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        static constexpr bool try_parse(Context& context, Reader reader)
        {
            // We need to match the entire rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            auto begin  = reader.position();
            auto result = parser.try_parse(reader);
            auto end    = parser.end;

            // Report that we've backtracked.
            context.on(_ev::backtracked{}, begin, end);

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC static bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!bp<Context, Reader>::try_parse(context, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::peek_failure>;
                auto err  = lexy::error<Reader, tag>(reader.position());
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peek<Rule, Error> error = {};
};

template <typename Rule, typename Tag>
struct _peekn : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr bool try_parse(Context& context, Reader reader)
        {
            // We must not match the rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            auto begin  = reader.position();
            auto result = !parser.try_parse(reader);
            end         = parser.end;

            // Report that we've backtracked.
            context.on(_ev::backtracked{}, begin, end);

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (bp<Context, Reader> impl{}; !impl.try_parse(context, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::unexpected>;
                auto err  = lexy::error<Reader, tag>(reader.position(), impl.end);
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peekn<Rule, Error> error = {};
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek(Rule)
{
    return _peek<Rule, void>{};
}

/// Check if at this reader position, the rule would not match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek_not(Rule)
{
    return _peekn<Rule, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

