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
    static constexpr auto has_matcher = [] {
        if constexpr (std::is_void_v<Then>)
            return true;
        else
            return Then::has_matcher;
    }();

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto save = reader;
            while (Condition::matcher::match(reader))
            {
                if constexpr (!std::is_void_v<Then>)
                {
                    if (!Then::matcher::match(reader))
                    {
                        reader = LEXY_MOV(save);
                        return false;
                    }
                }
            }

            return true;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            _loop_handler<Handler> loop_handler{handler, false};
            while (true)
            {
                if (!Condition::matcher::match(reader))
                    break;

                if constexpr (!std::is_void_v<Then>)
                {
                    using continuation = _loop_iter_parser<Args...>;
                    auto result = Then::template parser<continuation>::parse(loop_handler, reader,
                                                                             LEXY_FWD(args)...);
                    if (!result)
                        return LEXY_MOV(result).error();
                }
            }

            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
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

namespace lexyd
{
template <typename Terminator, typename Rule>
struct _whlt : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            using branch = decltype(branch(Terminator()));
            _loop_handler<Handler> loop_handler{handler, false};
            while (true)
            {
                if (branch::condition_matcher::match(reader))
                    break;

                using continuation = _loop_iter_parser<Args...>;
                auto result = Rule::template parser<continuation>::parse(loop_handler, reader,
                                                                         LEXY_FWD(args)...);
                if (!result)
                    return LEXY_MOV(result).error();
            }

            return branch::template then_parser<NextParser>::parse(handler, reader,
                                                                   LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED

