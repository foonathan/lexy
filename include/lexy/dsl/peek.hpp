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
template <typename Engine, typename Tag>
struct _peek : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (!lexy::engine_peek<Engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!lexy::engine_peek<Engine>(reader))
            {
                using tag = std::conditional_t<std::is_void_v<Tag>, lexy::peek_failure, Tag>;
                auto err  = lexy::make_error<Reader, tag>(reader.cur());
                context.error(err);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peek<Engine, Error> error = {};
};

template <typename Engine, typename Tag>
struct _peekn : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if (lexy::engine_peek<Engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto copy = reader;
            if (auto begin = copy.cur(); lexy::engine_try_match<Engine>(copy))
            {
                using tag = std::conditional_t<std::is_void_v<Tag>, lexy::unexpected, Tag>;
                auto err  = lexy::make_error<Reader, tag>(begin, copy.cur());
                context.error(err);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peekn<Engine, Error> error = {};
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek(Rule rule)
{
    using token = decltype(token(rule));
    return _peek<typename token::token_engine, void>{};
}

/// Check if at this reader position, the rule would not match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek_not(Rule rule)
{
    using token = decltype(token(rule));
    return _peekn<typename token::token_engine, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

