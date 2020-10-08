// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Condition, typename Then>
struct _opt : rule_base
{
    static constexpr auto has_matcher = Then::has_matcher;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            if (auto reset = reader; Condition::matcher::match(reader))
            {
                if (Then::matcher::match(reader))
                    return true;
                else
                {
                    reader = LEXY_MOV(reset);
                    return false;
                }
            }
            else
            {
                reader = LEXY_MOV(reset);
                return true;
            }
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (auto result = Condition::matcher::match(reader))
                return Then::template parser<NextParser>::parse(handler, reader, LEXY_FWD(args)...);
            else
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
};
template <typename Condition>
struct _opt<Condition, void> : atom_base<_opt<Condition, void>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        Condition::matcher::match(reader);
        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC void error(const Reader&, typename Reader::iterator);
};

/// Matches the rule or nothing.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule rule)
{
    static_assert(lexy::is_branch_rule<Rule>, "opt() requires a branch condition");

    if constexpr (lexy::is_pattern<Rule>)
        return _opt<Rule, void>{};
    else
    {
        auto as_branch = branch(rule);
        return _opt<decltype(as_branch.condition()), decltype(as_branch.then())>{};
    }
}
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED
