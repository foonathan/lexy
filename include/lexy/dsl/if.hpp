// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_IF_HPP_INCLUDED
#define LEXY_DSL_IF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Condition, typename Then>
struct _if : rule_base
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

/// If the branch condition matches, matches the branch then.
template <typename Branch>
LEXY_CONSTEVAL auto if_(Branch b)
{
    static_assert(lexy::is_branch_rule<Branch>, "if_() requires a branch condition");

    auto as_branch = branch(b);
    return _if<decltype(as_branch.condition()), decltype(as_branch.then())>{};
}
} // namespace lexyd

#endif // LEXY_DSL_IF_HPP_INCLUDED

