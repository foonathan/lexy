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
#include <lexy/engine/while.hpp>

//=== implementation ===//
namespace lexy::_detail
{
struct tag_no_whitespace
{};

template <typename Rule>
struct ws_production
{
    static constexpr auto rule = lexy::dsl::loop(Rule{} | lexy::dsl::break_);
};

template <typename Context>
struct whitespace_handler
{
    Context* parent;

    //=== result ===//
    template <typename Production>
    using production_result = void;

    template <typename Production>
    constexpr bool get_result_value() && noexcept
    {
        return true;
    }
    template <typename Production>
    constexpr bool get_result_empty() && noexcept
    {
        return false;
    }

    //=== events ===//
    template <typename Production>
    struct marker
    {};

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
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto begin = reader.position();
        if constexpr (lexy::is_token_rule<Rule>)
        {
            // Parsing a token repeatedly cannot fail, so we can optimize it using an engine.
            using engine = lexy::engine_while<typename Rule::token_engine>;
            engine::match(reader);
        }
        else
        {
            // Parse the rule using a special handler that only forwards errors.
            using production = ws_production<Rule>;
            whitespace_handler<Context> ws_handler{&context};
            if (!lexy::do_action<production>(LEXY_MOV(ws_handler), reader))
                return false;
        }
        auto end = reader.position();

        // Add a whitespace token node.
        if (begin != end)
            context.on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin, end);

        return NextParser::parse(context, reader, LEXY_FWD(args)...);
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
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
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
    struct parser : lexy::_detail::manual_ws_parser<Rule, NextParser>
    {};

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
    struct parser : lexy::_detail::automatic_ws_parser<NextParser>
    {};

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
struct _wsn : rule_base
{
    static constexpr auto is_branch               = Rule::is_branch;
    static constexpr auto is_unconditional_branch = Rule::is_unconditional_branch;

    template <typename NextParser>
    struct parser
    {
        struct _cont
        {
            template <typename WsContext, typename Reader, typename Context, typename... Args>
            LEXY_DSL_FUNC bool parse(WsContext&, Reader& reader, Context& context, Args&&... args)
            {
                // Continue with the normal context, after skipping whitespace.
                return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                           LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
                // Optimization: no whitespace rule in the current context; do nothing special.
                return lexy::rule_parser<Rule, NextParser>::try_parse(context, reader,
                                                                      LEXY_FWD(args)...);

            // Parse the rule using the context that doesn't allow inner whitespace.
            lexy::_detail::parse_context_var ws_context(context, lexy::_detail::tag_no_whitespace{},
                                                        lexy::_detail::tag_no_whitespace{});
            return lexy::rule_parser<Rule, _cont>::try_parse(ws_context, reader, context,
                                                             LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
                // Optimization: no whitespace rule in the current context; do nothing special.
                return lexy::rule_parser<Rule, NextParser>::parse(context, reader,
                                                                  LEXY_FWD(args)...);

            // Parse the rule using the context that doesn't allow inner whitespace.
            lexy::_detail::parse_context_var ws_context(context, lexy::_detail::tag_no_whitespace{},
                                                        lexy::_detail::tag_no_whitespace{});
            return lexy::rule_parser<Rule, _cont>::parse(ws_context, reader, context,
                                                         LEXY_FWD(args)...);
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

