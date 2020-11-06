// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/loop.hpp>

namespace lexyd
{
template <typename Condition, typename Then>
struct _whl : rule_base
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

    struct _rule : rule_base
    {
        static constexpr auto has_matcher = false;

        template <typename NextParser>
        struct parser
        {
            template <typename Handler, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto parse(_loop_handler<Handler>& handler, Reader& reader,
                                     Args&&... args) -> typename _loop_handler<Handler>::result_type
            {
                if (!Condition::matcher::match(reader))
                    return LEXY_MOV(handler).break_();

                return Then::template parser<NextParser>::parse(handler, reader, LEXY_FWD(args)...);
            }
        };
    };

    template <typename NextParser>
    using parser = typename _loop<_rule>::template parser<NextParser>;
};

template <typename Pattern>
struct _whl<Pattern, void> : atom_base<_whl<Pattern, void>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        while (Pattern::matcher::match(reader))
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
    {
        (void)rule;
        return _whl<Rule, void>{};
    }
    else
    {
        auto b = branch(rule);
        return _whl<decltype(b.condition()), decltype(b.then())>{};
    }
}
} // namespace lexyd

namespace lexyd
{
template <typename... R>
struct _whlc : rule_base
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
    using parser =
        typename decltype(loop(_choice{} | else_ >> break_))::template parser<NextParser>;
};

/// Matches the choice as often as possible.
template <typename... R>
LEXY_CONSTEVAL auto while_(_chc<R...>)
{
    return _whlc<R...>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the rule at least once, then as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_one(Rule rule)
{
    if constexpr (lexy::is_branch_rule<Rule>)
        return rule >> while_(rule);
    else
        return rule + while_(rule);
}

/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
LEXY_CONSTEVAL auto do_while(Then then, Condition condition)
{
    if constexpr (lexy::is_branch_rule<Then>)
        return then >> while_(condition >> then);
    else
        return then + while_(condition >> then);
}
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED

