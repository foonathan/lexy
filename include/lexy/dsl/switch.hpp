// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is partialject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SWITCH_HPP_INCLUDED
#define LEXY_DSL_SWITCH_HPP_INCLUDED

#include <lexy/dsl/any.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/error.hpp>

namespace lexy
{
struct exhausted_switch
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted switch";
    }
};
} // namespace lexy

namespace lexyd
{
// Continues with the rest of the input after we've handled the switch.
template <typename NextParser>
struct _switch_continue
{
    template <typename Handler, typename PartialReader, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, PartialReader&, Reader& reader, Args&&... args) ->
        typename Handler::result_type
    {
        // We now continue with the regular reader again.
        return NextParser::parse(handler, reader, LEXY_FWD(args)...);
    }
};

// Selects the appropriate case after the switch rule has been matched.
template <typename NextParser, typename... Cases>
struct _switch_select;
template <typename NextParser>
struct _switch_select<NextParser>
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Reader save, Args&&...) ->
        typename Handler::result_type
    {
        // We didn't match any of the switch cases, report an error.
        // save.cur() is the beginning of the switched value, reader.cur() at the end.
        auto e = lexy::make_error<Reader, lexy::exhausted_switch>(save.cur(), reader.cur());
        return LEXY_MOV(handler).error(reader, e);
    }
};
template <typename NextParser, typename H, typename... T>
struct _switch_select<NextParser, H, T...>
{
    template <typename Handler, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Reader save, Args&&... args) ->
        typename Handler::result_type
    {
        using cont      = _switch_continue<NextParser>;
        using as_branch = decltype(branch(H{}));

        // We only want to read what the value has matched.
        auto partial = lexy::partial_reader(save, reader.cur());

        if constexpr (lexy::is_pattern<H>)
        {
            // We need this to catch `dsl::success` on its own, otherwise, it will be treated as an
            // unconditional branch `else_ >> success`. This is not true, as it needs to consume the
            // entire pattern, but the branch logic doesn't know it.

            if (H::matcher::match(partial) && partial.eof())
                // We can continue directly, it's a pattern.
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            else
                return _switch_select<NextParser, T...>::parse(handler, reader, save,
                                                               LEXY_FWD(args)...);
        }
        else if constexpr (as_branch::is_unconditional)
        {
            // We take it directly.
            return as_branch::template then_parser<cont>::parse(handler, partial, reader,
                                                                LEXY_FWD(args)...);
        }
        else
        {
            if (as_branch::condition_matcher::match(partial) && partial.eof())
                // We have matched the entire value, continue with normal parsing.
                return as_branch::template then_parser<cont>::parse(handler, partial, reader,
                                                                    LEXY_FWD(args)...);
            else
                return _switch_select<NextParser, T...>::parse(handler, reader, save,
                                                               LEXY_FWD(args)...);
        }
    }
};

template <typename Rule, typename... Cases>
struct _switch : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            // We parse the rule using our special continuation.
            // To recover the old reader position, we create a copy.
            using cont = _switch_select<NextParser, Cases...>;
            return Rule::template parser<cont>::parse(handler, reader, Reader(reader),
                                                      LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Adds a case to the switch.
    template <typename Branch>
    LEXY_CONSTEVAL auto case_(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>, "switch case must be a branch");
        return _switch<Rule, Cases..., Branch>{};
    }

    /// Adds a default value to the switch.
    template <typename Default>
    LEXY_CONSTEVAL auto default_(Default def) const
    {
        return case_(else_ >> def);
    }

    /// Adds an error on the default case.
    template <typename Error>
    LEXY_CONSTEVAL auto error()
    {
        return default_(lexyd::error<Error>(any));
    }
};

/// Switches on the lexeme matched by the rule.
/// The first case that will match the entire pattern will be taken.
template <typename Rule>
LEXY_CONSTEVAL auto switch_(Rule)
{
    return _switch<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SWITCH_HPP_INCLUDED
