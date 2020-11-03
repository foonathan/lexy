// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MINUS_HPP_INCLUDED
#define LEXY_DSL_MINUS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
/// We've matched the Except of a minus.
struct minus_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "minus failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Rule, typename Except>
struct _minus : rule_base
{
    static constexpr auto has_matcher = Rule::has_matcher;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto save = reader;

            // First check whether the original rule matched.
            if (!Rule::matcher::match(reader))
                return false;

            // Then match Except on the same input.
            if (auto partial = lexy::partial_reader(save, reader.cur());
                Except::matcher::match(partial) && partial.eof())
            {
                // It did, so we don't match after all.
                reader = LEXY_MOV(save);
                return false;
            }

            return true;
        }
    };

    template <typename NextParser>
    struct parser
    {
        struct _continuation
        {
            template <typename Handler, typename Reader, typename... Args>
            LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Reader save, Args&&... args)
                -> typename Handler::result_type
            {
                // At this point, we've matched the rule.
                // Check, whether Except matches as well on the same input.
                if (auto partial = lexy::partial_reader(save, reader.cur());
                    Except::matcher::match(partial) && partial.eof())
                {
                    // It did, so we don't match after all.
                    auto e
                        = lexy::make_error<Reader, lexy::minus_failure>(save.cur(), reader.cur());
                    return LEXY_MOV(handler).error(reader, e);
                }

                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // Parse the Rule, but remember the current input by copying the reader.
            return Rule::template parser<_continuation>::parse(handler, reader, Reader(reader),
                                                               LEXY_FWD(args)...);
        }
    };
};

/// Matches Rule unless Except matches on the input Rule matched.
template <typename Rule, typename Except>
LEXY_CONSTEVAL auto operator-(Rule, Except)
{
    static_assert(lexy::is_pattern<Except>);
    return _minus<Rule, Except>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED

