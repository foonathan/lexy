// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_IF_HPP_INCLUDED
#define LEXY_DSL_IF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Branch>
struct _if : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using branch_parser = lexy::rule_parser<Branch, NextParser>;

            auto result = branch_parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Branch wasn't taken, continue anyway.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                // Return true/false depending on result.
                return static_cast<bool>(result);
        }
    };
};

/// If the branch condition matches, matches the branch then.
template <typename Branch>
LEXY_CONSTEVAL auto if_(Branch)
{
    static_assert(lexy::is_branch<Branch>, "if_() requires a branch condition");
    if constexpr (Branch::is_unconditional_branch)
        // Branch is always taken, so don't wrap in if_().
        return Branch{};
    else
        return _if<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_IF_HPP_INCLUDED

