// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Branch>
struct _whl : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                using branch_parser
                    = lexy::rule_parser<Branch, lexy::context_discard_parser<Context>>;

                auto result = branch_parser::try_parse(context, reader);
                if (result == lexy::rule_try_parse_result::backtracked)
                    break;
                else if (result == lexy::rule_try_parse_result::canceled)
                    return false;
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches the branch rule as often as possible.
template <typename Rule>
constexpr auto while_(Rule)
{
    static_assert(lexy::is_branch<Rule>, "while() requires a branch condition");
    return _whl<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the rule at least once, then as often as possible.
template <typename Rule>
constexpr auto while_one(Rule rule)
{
    static_assert(lexy::is_branch<Rule>, "while_one() requires a branch condition");
    return rule >> while_(rule);
}
} // namespace lexyd

namespace lexyd
{
/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
constexpr auto do_while(Then then, Condition condition)
{
    if constexpr (lexy::is_branch<Then>)
        return then >> while_(condition >> then);
    else
        return then + while_(condition >> then);
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Rule, typename Recover>
struct _whlt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            while (true)
            {
                using term_parser = lexy::rule_parser<Term, NextParser>;
                if (auto result = term_parser::try_parse(context, reader, LEXY_FWD(args)...);
                    result != lexy::rule_try_parse_result::backtracked)
                {
                    // We had the terminator, and thus are done.
                    return static_cast<bool>(result);
                }

                using parser = lexy::rule_parser<Rule, lexy::context_discard_parser<Context>>;
                if (!parser::parse(context, reader))
                {
                    using recovery = lexy::rule_parser<Recover, NextParser>;
                    return recovery::parse(context, reader, LEXY_FWD(args)...);
                }
            }

            return false; // unreachable
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED

