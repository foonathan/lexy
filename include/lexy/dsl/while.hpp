// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>

namespace lexyd
{
template <typename Condition, typename Then>
struct _while : rule_base
{
    static constexpr auto has_matcher = Then::has_matcher;

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

template <typename... R>
struct _while_chc : rule_base
{
    using _choice                     = _chc<R...>;
    static constexpr auto has_matcher = _choice::has_matcher;

    struct matcher
    {
        template <typename Rule, typename Reader>
        LEXY_DSL_FUNC int _match(Reader& reader)
        {
            using as_branch = decltype(branch(Rule{}));

            if (as_branch::condition_matcher::match(reader))
                return as_branch::then_matcher::match(reader) ? 1 : 0;
            else
                return -1;
        }

        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto save = reader;

            while (true)
            {
                int result = -1;
                // Try to match all branches in turn, until we found one where the result isn't
                // unmatched.
                (void)((result = _match<R>(reader), result == -1 ? false : true) || ...);

                if (result == -1)
                    // We no longer matched any condition, we are done.
                    break;
                else if (result == 0)
                {
                    // We had a match failure.
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

        struct _break : rule_base
        {
            static constexpr auto has_matcher = false;

            // We ignore the prescribed continuation and go to NextParser instead.
            template <typename>
            using parser = NextParser;
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // We parse a choice that continues to the continuation which jumps back here again.
            // However, if the choice didn't match, we parse _next_rule, which jumps to NextParser
            // instead.
            auto rule    = _choice{} | else_ >> _break{};
            using parser = typename decltype(rule)::template parser<_continuation<Args...>>;
            return parser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
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
template <typename... R>
LEXY_CONSTEVAL auto while_(_chc<R...>)
{
    return _while_chc<R...>{};
}

/// Matches the rule at least once, then as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_one(Rule rule)
{
    if constexpr (lexy::is_pattern<Rule>)
        return rule >> while_(rule);
    else
        return rule + while_(rule);
}

/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
LEXY_CONSTEVAL auto do_while(Then then, Condition condition)
{
    if constexpr (lexy::is_pattern<Then>)
        return then >> while_(condition >> then);
    else
        return then + while_(condition >> then);
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED
