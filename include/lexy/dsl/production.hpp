// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
/// Parses the rule of the production as if it were part of the current production.
template <typename Production>
constexpr auto inline_ = lexy::production_rule<Production>{};
} // namespace lexyd

namespace lexyd
{
template <typename BranchParser, typename Context, typename Reader>
constexpr bool _finish_production_branch(BranchParser& branch, Context& context, Reader& reader)
{
    return branch.finish(context, reader);
}

template <typename Production>
struct _prd : _copy_base<lexy::production_rule<Production>>
{
    template <typename NextParser>
    struct p
    {
        using impl = lexy::_detail::production_parser<Production>;

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Create a context for the production and parse the context there.
            auto sub_context = impl::get_sub_context(context, reader);
            using parser
                = lexy::parser_for<lexy::production_rule<Production>, lexy::_detail::final_parser>;
            if (parser::parse(sub_context, reader))
            {
                // Extract value and continue.
                return impl::template finish<NextParser>(context, reader, sub_context,
                                                         LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                impl::cancel_sub_context(sub_context, reader);
                return false;
            }
        }
    };

    template <typename Context, typename Reader>
    struct bp
    {
        using impl          = lexy::_detail::production_parser<Production>;
        using sub_context_t = typename impl::template sub_context_t<Context, Reader>;
        using parser_t
            = lexy::branch_parser_for<lexy::production_rule<Production>, sub_context_t, Reader>;

        lexy::_detail::lazy_init<sub_context_t> sub_context;
        parser_t                                parser;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Create the new context.
            sub_context = {};
            sub_context.emplace(impl::get_sub_context(context, reader));

            // Try and parse the production on the new context.
            auto result = parser.try_parse(*sub_context, reader);
            if (!result)
                impl::cancel_sub_context(*sub_context, reader);

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the production.
            if (parser.template finish<lexy::_detail::final_parser>(*sub_context, reader))
            {
                // Continue parsing with the result.
                return impl::template finish<NextParser>(context, reader, *sub_context,
                                                         LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                impl::cancel_sub_context(*sub_context, reader);
                return false;
            }
        }
    };
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};

template <typename Production>
struct _rec : rule_base
{
    template <typename NextParser>
    struct p : _prd<Production>::template p<NextParser>
    {};
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

