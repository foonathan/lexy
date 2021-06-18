// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOP_HPP_INCLUDED
#define LEXY_DSL_LOOP_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
struct _break : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader&, Args&&...)
        {
            static_assert(sizeof...(Args) == 0, "looped rule must not add any values");

            // We set loop break on the member with the specified id.
            context.get(_break{}).loop_break = true;
            return true;
        }
    };
};

/// Exits a loop().
constexpr auto break_ = _break{};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _loop : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            struct flag
            {
                bool loop_break = false;
            };

            auto loop_context = context.insert(_break{}, flag{});
            while (!loop_context.get(_break{}).loop_break)
            {
                using parser
                    = lexy::rule_parser<Rule, lexy::context_discard_parser<decltype(loop_context)>>;
                if (!parser::parse(loop_context, reader))
                    return false;
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeatedly matches the rule until a break rule matches.
template <typename Rule>
constexpr auto loop(Rule)
{
    return _loop<Rule>{};
}
} // namespace lexyd

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

#endif // LEXY_DSL_LOOP_HPP_INCLUDED

