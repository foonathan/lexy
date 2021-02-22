// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
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
    template <typename Context, typename PartialReader, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, PartialReader&, Reader& reader, Args&&... args)
    {
        // We now continue with the regular reader again.
        return NextParser::parse(context, reader, LEXY_FWD(args)...);
    }
};

// Selects the appropriate case after the switch rule has been matched.
template <typename NextParser, typename... Cases>
struct _switch_select;
template <typename NextParser>
struct _switch_select<NextParser>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&...)
    {
        // We didn't match any of the switch cases, report an error.
        // save.cur() is the beginning of the switched value, reader.cur() at the end.
        auto err = lexy::make_error<Reader, lexy::exhausted_switch>(save.cur(), reader.cur());
        context.error(err);
        return false;
    }
};
template <typename NextParser, typename H, typename... T>
struct _switch_select<NextParser, H, T...>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&... args)
    {
        using cont = _switch_continue<NextParser>;

        // We only want to read what the value has matched.
        auto partial         = lexy::partial_reader(save, reader.cur());
        using branch_matcher = lexy::branch_matcher<H, decltype(partial)>;

        if constexpr (branch_matcher::is_unconditional)
        {
            return lexy::rule_parser<H, cont>::parse(context, partial, reader, LEXY_FWD(args)...);
        }
        else
        {
            branch_matcher branch{};
            if (branch.match(partial) && partial.eof())
                return branch.template parse<cont>(context, partial, reader, LEXY_FWD(args)...);
            else
                return _switch_select<NextParser, T...>::parse(context, reader, save,
                                                               LEXY_FWD(args)...);
        }
    }
};

template <typename Rule, typename... Cases>
struct _switch : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // We parse the rule using our special continuation.
            // To recover the old reader position, we create a copy.
            using cont = _switch_select<NextParser, Cases...>;
            return lexy::rule_parser<Rule, cont>::parse(context, reader, Reader(reader),
                                                        LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Adds a case to the switch.
    template <typename Branch>
    LEXY_CONSTEVAL auto case_(Branch) const
    {
        static_assert(lexy::is_branch<Branch>, "switch case must be a branch");
        return _switch<Rule, Cases..., Branch>{};
    }

    /// Adds a default value to the switch.
    template <typename Default>
    LEXY_CONSTEVAL auto default_(Default def) const
    {
        return case_(else_ >> def);
    }

    /// Adds an error on the default case.
    template <typename Tag>
    static constexpr _switch<Rule, Cases..., _err<Tag, _any>> error = {};

    LEXY_DEPRECATED_ERROR("replace `switch.error<Tag>()` by `switch.error<Tag>`")
    constexpr _switch operator()() const
    {
        return *this;
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

