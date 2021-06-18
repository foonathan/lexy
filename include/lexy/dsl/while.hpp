// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHILE_HPP_INCLUDED
#define LEXY_DSL_WHILE_HPP_INCLUDED

#warning "include <lexy/dsl/loop.hpp> instead"
#include <lexy/dsl/loop.hpp>

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

