// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_OPTION_HPP_INCLUDED
#define LEXY_DSL_OPTION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexy
{
// An optional type is something that has the following:
// * a default constructors: this means we can actually construct it from our `nullopt`
// * a dereference operator: this means that it actually contains something else
// * a contextual conversion to bool: this means that it might be "false" (i.e. empty)
//
// This definition should work:
// * it excludes all default constructible types that are convertible to bool (e.g. integers...)
// * it includes pointers, which is ok
// * it includes `std::optional` and all non-std implementations of it
template <typename T>
using _detect_optional_like = decltype(T(), *LEXY_DECLVAL(T&), !LEXY_DECLVAL(const T&));

struct nullopt
{
    template <typename T, typename = _detect_optional_like<T>>
    constexpr operator T() const
    {
        return T();
    }
};
} // namespace lexy

namespace lexyd
{
struct _nullopt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
        }
    };
};

constexpr auto nullopt = _nullopt{};
} // namespace lexyd

namespace lexyd
{
template <typename Branch>
struct _opt : rule_base
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
                // Branch wasn't taken, continue anyway with nullopt.
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
            else
                // Return true/false depending on result.
                return static_cast<bool>(result);
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
LEXY_CONSTEVAL auto opt(Rule)
{
    static_assert(lexy::is_branch<Rule>, "opt() requires a branch condition");
    if constexpr (Rule::is_unconditional_branch)
        // Branch is always taken, so don't wrap in opt().
        return Rule{};
    else
        return _opt<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename R, typename Recover>
struct _optt : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Try to parse the terminator.
            using term_parser = lexy::rule_parser<Term, NextParser>;
            if (auto result
                = term_parser::try_parse(context, reader, LEXY_FWD(args)..., lexy::nullopt{});
                result != lexy::rule_try_parse_result::backtracked)
            {
                // We had the terminator, and thus created the empty optional.
                return static_cast<bool>(result);
            }

            // Parse the rule followed by the terminator.
            using parser = lexy::rule_parser<R, term_parser>;
            if (!parser::parse(context, reader, LEXY_FWD(args)...))
            {
                using recovery = lexy::rule_parser<Recover, NextParser>;
                return recovery::parse(context, reader, LEXY_FWD(args)...);
            }

            return true;
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_OPTION_HPP_INCLUDED

