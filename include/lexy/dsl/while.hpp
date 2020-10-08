// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Condition, typename Then>
struct _while : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto save = reader;
            while (Condition::matcher::match(reader))
            {
                if (!Then::matcher::match(reader))
                {
                    reader = LEXY_MOV(save);
                    return false;
                }
            }

            return true;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _continuation
        {
            template <typename Handler, typename Reader, typename... Extra>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args, Extra&&...)
                -> typename Handler::result_type
            {
                static_assert(sizeof...(Extra) == 0, "while_() must not create values");

                // After we've parsed then, we try again.
                return parser::parse(handler, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (Condition::matcher::match(reader))
                // Try another iteration.
                return Then::template parser<_continuation<Args...>>::parse(handler, reader,
                                                                            LEXY_FWD(args)...);
            else
                // Continue with next parser.
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
};
template <typename Condition>
struct _while<Condition, void> : atom_base<_while<Condition, void>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        while (Condition::matcher::match(reader))
        {
        }
        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC void error(const Reader&, typename Reader::iterator);
};

/// Matches the pattern branch rule as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_(Rule rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "while() requires a branch condition");

    if constexpr (lexy::is_pattern<Rule>)
        return _while<Rule, void>{};
    else
    {
        auto as_branch = branch(rule);
        return _while<decltype(as_branch.condition()), decltype(as_branch.then())>{};
    }
}

/// Matches the rule at least once, then as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_one(Rule rule)
{
    return rule + while_(rule);
}

/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
LEXY_CONSTEVAL auto do_while(Then then, Condition condition)
{
    return then + while_(condition >> then);
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED
