// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
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
template <typename Branch>
struct _whl : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            while (true)
            {
                lexy::branch_matcher<Branch, Reader> branch{};
                if (!branch.match(reader))
                    break;

                auto result
                    = branch.template parse<lexy::context_discard_parser>(context, reader,
                                                                          LEXY_FWD(args)...);
                if (result.has_error())
                    return LEXY_MOV(result);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename... R>
struct _whlc : rule_base
{
    using _choice = _chc<R...>;

    template <typename NextParser>
    using parser = lexy::rule_parser<decltype(loop(_choice{} | else_ >> break_)), NextParser>;
};

/// Matches the branch rule as often as possible.
template <typename Rule>
LEXY_CONSTEVAL auto while_(Rule)
{
    static_assert(lexy::is_branch<Rule>, "while() requires a branch condition");
    return _whl<Rule>{};
}
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
    if constexpr (lexy::is_branch<Rule>)
        return rule >> while_(rule);
    else
        return rule + while_(rule);
}
} // namespace lexyd

namespace lexyd
{
/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
LEXY_CONSTEVAL auto do_while(Then then, Condition condition)
{
    if constexpr (lexy::is_branch<Then>)
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
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            lexy::branch_matcher<Terminator, Reader> term;
            while (!term.match(reader))
            {
                using parser = lexy::rule_parser<Rule, lexy::context_discard_parser>;
                auto result  = parser::parse(context, reader, LEXY_FWD(args)...);
                if (result.has_error())
                    return LEXY_MOV(result);
            }

            return term.template parse<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_WHILE_HPP_INCLUDED

