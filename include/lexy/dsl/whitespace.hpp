// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/token.hpp>

//=== implementation ===//
namespace lexy::_detail
{
struct tag_no_whitespace
{};

template <typename Rule>
struct ws_production
{
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule                = lexy::dsl::loop(Rule{} | lexy::dsl::break_);
};

template <typename Context>
struct whitespace_handler
{
    Context* parent;

    //=== events ===//
    template <typename Production>
    struct marker
    {
        constexpr void get_value() && {}
    };

    template <typename Production>
    constexpr bool get_action_result(bool parse_result, marker<Production>&&) &&
    {
        return parse_result;
    }

    template <typename Rule, typename Iterator>
    constexpr auto on(parse_events::production_start<ws_production<Rule>>, Iterator)
    {
        return marker<ws_production<Rule>>{};
    }
    template <typename Production, typename Iterator>
    constexpr auto on(parse_events::production_start<Production>, Iterator)
    {
        static_assert(_detail::error<Production>,
                      "whitespace rule must not contain `dsl::p` or `dsl::recurse`;"
                      "use `dsl::inline_` instead");
        return marker<Production>{};
    }

    template <typename Production, typename Iterator>
    constexpr auto on(marker<Production>, parse_events::list, Iterator)
    {
        return lexy::noop.sink();
    }

    template <typename Production, typename Error>
    constexpr void on(marker<Production>, parse_events::error ev, Error&& error)
    {
        parent->on(ev, LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr void on(const Args&...)
    {}
};

template <typename Rule, typename NextParser>
struct manual_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto result = true;
        auto begin  = reader.position();
        if constexpr (lexy::is_token_rule<Rule>)
        {
            // Parsing a token repeatedly cannot fail, so we can optimize it.
            while (lexy::try_match_token(Rule{}, reader))
            {}
        }
        else
        {
            // Parse the rule using a special handler that only forwards errors.
            using production = ws_production<Rule>;
            whitespace_handler<Context> ws_handler{&context};
            result = lexy::do_action<production>(LEXY_MOV(ws_handler), reader);
        }
        auto end = reader.position();

        if (result)
        {
            // Add a whitespace token node.
            if (begin != end)
                context.on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin, end);

            // And continue.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Add an error token node.
            if (begin != end)
                context.on(lexy::parse_events::token{}, lexy::error_token_kind, begin, end);

            // And cancel.
            return false;
        }
    }
};
template <typename NextParser>
struct manual_ws_parser<void, NextParser> : NextParser
{};

template <typename Context>
using context_whitespace
    = lexy::production_whitespace<typename Context::production, typename Context::root_production>;

template <typename NextParser>
struct automatic_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if constexpr (Context::contains(lexy::_detail::tag_no_whitespace{}))
        {
            // Automatic whitespace skipping is disabled.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Skip the appropriate whitespace.
            using rule = context_whitespace<Context>;
            return manual_ws_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
    }
};

} // namespace lexy::_detail

//=== whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsr : rule_base
{
    template <typename NextParser>
    using p = lexy::_detail::manual_ws_parser<Rule, NextParser>;

    template <typename R>
    friend constexpr auto operator|(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} | r)>{};
    }
    template <typename R>
    friend constexpr auto operator|(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r | Rule{})>{};
    }

    template <typename R>
    friend constexpr auto operator/(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} / r)>{};
    }
    template <typename R>
    friend constexpr auto operator/(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r / Rule{})>{};
    }
};

struct _ws : rule_base
{
    template <typename NextParser>
    using p = lexy::_detail::automatic_ws_parser<NextParser>;

    /// Overrides implicit whitespace detection.
    template <typename Rule>
    constexpr auto operator()(Rule) const
    {
        return _wsr<Rule>{};
    }
};

/// Matches whitespace.
constexpr auto whitespace = _ws{};
} // namespace lexyd

//=== no_whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsn : _copy_base<Rule>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename ParentContext, typename Id, typename T, typename Reader,
                  typename Context, typename... Args>
        LEXY_PARSER_FUNC static bool parse(lexy::_detail::parse_context_var<ParentContext, Id, T>&,
                                           Reader& reader, Context& context, Args&&... args)
        {
            static_assert(std::is_same_v<ParentContext, Context>                      //
                              && std::is_same_v<Id, lexy::_detail::tag_no_whitespace> //
                              && std::is_same_v<Id, T>,
                          "cannot create context variables inside `lexy::dsl::no_whitespace()`");

            // Continue with the normal context, after skipping whitespace.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader,
              typename Whitespace = lexy::_detail::context_whitespace<Context>>
    struct bp
    {
        using whitespace_context
            = lexy::_detail::parse_context_var<Context, lexy::_detail::tag_no_whitespace>;

        lexy::branch_parser_for<Rule, whitespace_context, Reader> rule;

        constexpr auto try_parse(Context& context, const Reader& reader)
        {
            // Create a context that doesn't allow whitespace.
            // This is essentially free, so we can do it twice.
            whitespace_context ws_context(context);

            // Try parse the rule in that context.
            return rule.try_parse(ws_context, reader);
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule with on another whitespace context.
            whitespace_context ws_context(context);
            return rule.template finish<_pc<NextParser>>(ws_context, reader, context,
                                                         LEXY_FWD(args)...);
        }
    };

    // Optimization: if there is no whitespace rule, we just parse Rule directly.
    template <typename Context, typename Reader>
    struct bp<Context, Reader, void> : lexy::branch_parser_for<Rule, Context, Reader>
    {};

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
            {
                // No whitespace, just parse the rule.
                return lexy::parser_for<Rule, NextParser>::parse(context, reader,
                                                                 LEXY_FWD(args)...);
            }
            else
            {
                // Parse the rule in the whitespace context.
                lexy::_detail::parse_context_var ws_context(context,
                                                            lexy::_detail::tag_no_whitespace{},
                                                            lexy::_detail::tag_no_whitespace{});

                using parser = lexy::parser_for<Rule, _pc<NextParser>>;
                return parser::parse(ws_context, reader, context, LEXY_FWD(args)...);
            }
        }
    };
};

/// Disables automatic skipping of whitespace for all tokens of the given rule.
template <typename Rule>
constexpr auto no_whitespace(Rule)
{
    if constexpr (lexy::is_token_rule<Rule>)
        return Rule{}; // Token already behaves that way.
    else
        return _wsn<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED

