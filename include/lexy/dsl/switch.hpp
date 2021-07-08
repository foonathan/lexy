// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is partialject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SWITCH_HPP_INCLUDED
#define LEXY_DSL_SWITCH_HPP_INCLUDED

#include <lexy/dsl/any.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_SWITCH
#    define LEXY_DEPRECATED_SWITCH
#else
#    define LEXY_DEPRECATED_SWITCH                                                                 \
        [[deprecated("`dsl::switch()` has been replaced by `dsl::symbol()`")]]
#endif

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
template <typename Token, typename Value>
struct _switch_case : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename PartialReader, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, PartialReader& partial, Reader& reader,
                                     Args&&... args) -> lexy::rule_try_parse_result
        {
            if (lexy::engine_try_match<typename Token::token_engine>(partial) && partial.eof())
                return static_cast<lexy::rule_try_parse_result>(
                    lexy::rule_parser<Value, NextParser>::parse(context, reader,
                                                                LEXY_FWD(args)...));
            else
                return lexy::rule_try_parse_result::backtracked;
        }
    };
};

template <typename Value>
struct _switch_case<void, Value> : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    using parser = lexy::rule_parser<Value, NextParser>;
};

// Selects the appropriate case after the switch rule has been matched.
template <typename NextParser, typename... Cases>
struct _switch_select;
template <typename NextParser, typename Tag>
struct _switch_select<NextParser, Tag>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&...)
    {
        // We didn't match any of the switch cases, report an error.
        // save.cur() is the beginning of the switched value, reader.cur() at the end.
        using tag = lexy::_detail::type_or<Tag, lexy::exhausted_switch>;
        auto err  = lexy::make_error<Reader, tag>(save.cur(), reader.cur());
        context.on(_ev::error{}, err);
        return false;
    }
};
template <typename NextParser, typename H, typename... T>
struct _switch_select<NextParser, H, T...>
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Reader save, Args&&... args)
    {
        if constexpr (H::is_unconditional_branch)
        {
            (void)save;
            return lexy::rule_parser<H, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // We only want to read what the value has matched.
            auto partial = lexy::partial_reader(save, reader.cur());

            auto result = lexy::rule_parser<H, NextParser>::try_parse(context, partial, reader,
                                                                      LEXY_FWD(args)...);
            if (result == lexy::rule_try_parse_result::backtracked)
                // Try the next branch.
                return _switch_select<NextParser, T...>::parse(context, reader, save,
                                                               LEXY_FWD(args)...);
            else
                return static_cast<bool>(result);
        }
    }
};

template <typename Rule, typename Error, typename... Cases>
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
            using cont = _switch_select<NextParser, Cases..., Error>;
            return lexy::rule_parser<Rule, cont>::parse(context, reader, Reader(reader),
                                                        LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Adds a case to the switch.
    template <typename Token, typename Value>
    constexpr auto case_(_br<Token, Value>) const
    {
        static_assert(lexy::is_token_rule<Token>, "case condition must be a token");
        return _switch<Rule, Error, Cases..., _switch_case<Token, Value>>{};
    }

    /// Adds a default value to the switch.
    template <typename Default>
    constexpr auto default_(Default) const
    {
        return _switch<Rule, Error, Cases..., _switch_case<void, Default>>{};
    }

    /// Adds an error on the default case.
    template <typename Tag>
    static constexpr _switch<Rule, Tag, Cases...> error = {};

    LEXY_DEPRECATED_ERROR("replace `switch.error<Tag>()` by `switch.error<Tag>`")
    constexpr _switch operator()() const
    {
        return *this;
    }
};

/// Switches on the lexeme matched by the rule.
/// The first case that will match the entire pattern will be taken.
template <typename Rule>
LEXY_DEPRECATED_SWITCH constexpr auto switch_(Rule)
{
    return _switch<Rule, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SWITCH_HPP_INCLUDED

