// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_BASE_HPP_INCLUDED
#define LEXY_ACTION_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/lazy_init.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/grammar.hpp>

//=== parse_context ===//
namespace lexy::_detail
{
template <typename Handler, typename Production>
using handler_production_result = typename Handler::template production_result<Production>;

template <typename Handler, typename Production>
using handler_marker = typename Handler::template marker<Production>;

template <typename Handler, typename Production, typename RootProduction>
class parse_context
{
public:
    //=== parse context ===//
    using handler         = Handler;
    using production      = Production;
    using root_production = RootProduction;

    constexpr auto& production_context()
    {
        return *this;
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args) -> std::enable_if_t<
        !std::is_base_of_v<parse_events::_production_event, Event>,
        decltype(LEXY_DECLVAL(Handler&).on(LEXY_DECLVAL(handler_marker<Handler, Production> &&), ev,
                                           LEXY_FWD(args)...))>
    {
        return _handler->on(LEXY_MOV(_marker), ev, LEXY_FWD(args)...);
    }

    //=== context variables ===//
    template <typename Id>
    static LEXY_CONSTEVAL bool contains(Id)
    {
        return false;
    }

    template <typename Id>
    constexpr auto get(Id)
    {
        static_assert(lexy::_detail::error<Id>, "context does not contain a variable with that id");
        return nullptr;
    }

private:
    template <typename Iterator>
    constexpr explicit parse_context(Handler& handler, Iterator begin)
    : _handler(&handler), _marker(handler.on(parse_events::production_start<Production>{}, begin))
    {}

    template <typename ChildProduction, typename Iterator>
    constexpr auto on(parse_events::production_start<ChildProduction>, Iterator begin)
    {
        // If the new production is a token production, need to re-root it.
        using new_root = std::conditional_t<lexy::is_token_production<ChildProduction>,
                                            ChildProduction, root_production>;
        return parse_context<Handler, ChildProduction, new_root>(*_handler, begin);
    }

    template <typename Iterator, typename... Args>
    constexpr void on(parse_events::production_finish<Production> ev, Iterator end,
                      Args&&... args) &&
    {
        using result_t = handler_production_result<Handler, Production>;
        if constexpr (std::is_void_v<result_t>)
        {
            _handler->on(LEXY_MOV(_marker), ev, end, LEXY_FWD(args)...);
            _result.emplace();
        }
        else
        {
            _result.emplace(_handler->on(LEXY_MOV(_marker), ev, end, LEXY_FWD(args)...));
        }
    }

    template <typename Iterator>
    constexpr void on(parse_events::production_cancel<Production> ev, Iterator pos) &&
    {
        _handler->on(LEXY_MOV(_marker), ev, pos);
    }

    Handler*                                                           _handler;
    handler_marker<Handler, Production>                                _marker;
    _detail::lazy_init<handler_production_result<Handler, Production>> _result;

    template <typename, typename, typename>
    friend class parse_context;

    friend struct final_parser;
    template <typename, typename>
    friend struct production_parser;
    template <typename P, typename H, typename Reader>
    friend constexpr auto action_impl(H& handler, Reader& reader);
};
} // namespace lexy::_detail

//=== action_impl ===//
namespace lexy::_detail
{
struct final_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        using event = parse_events::production_finish<typename Context::production>;
        LEXY_MOV(context.production_context()).on(event{}, reader.cur(), LEXY_FWD(args)...);
        return true;
    }
};

template <typename Production, typename Context, typename Reader>
constexpr bool parse_production(Context& context, Reader& reader)
{
    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, final_parser>::parse(context, reader);
}

template <typename Production, typename Context, typename Reader>
constexpr auto try_parse_production(Context& context, Reader& reader)
{
    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, final_parser>::try_parse(context, reader);
}

template <typename Production, typename NextParser>
struct production_parser
{
    struct _continuation
    {
        template <typename Context, typename Reader, typename Handler, typename Root,
                  typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                 parse_context<Handler, Production, Root>& new_context,
                                 Args&&... args)
        {
            // Might need to skip whitespace, according to the original context.
            using continuation
                = std::conditional_t<lexy::is_token_production<Production>,
                                     lexy::whitespace_parser<Context, NextParser>, NextParser>;

            // Pass the produced value to the next parser.
            using result_t = handler_production_result<Handler, Production>;
            if constexpr (std::is_void_v<result_t>)
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            else
                return continuation::parse(context, reader, LEXY_FWD(args)...,
                                           LEXY_MOV(*new_context._result));
        }
    };
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto new_context
            = context.production_context().on(parse_events::production_start<Production>{},
                                              reader.cur());
        if (parse_production<Production>(new_context, reader))
        {
            // Extract the value and continue.
            return _continuation::parse(context, reader, new_context, LEXY_FWD(args)...);
        }
        else
        {
            // We had an error, cancel the production.
            LEXY_MOV(new_context).on(parse_events::production_cancel<Production>{}, reader.cur());
            return false;
        }
    }

    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
        -> lexy::rule_try_parse_result
    {
        auto new_context
            = context.production_context().on(parse_events::production_start<Production>{},
                                              reader.cur());
        if (auto result = try_parse_production<Production>(new_context, reader);
            result == lexy::rule_try_parse_result::ok)
        {
            // Extract the value and continue.
            return _continuation::parse(context, reader, new_context, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }
        else
        {
            // We had an error, cancel the production.
            LEXY_MOV(new_context).on(parse_events::production_cancel<Production>{}, reader.cur());
            return result;
        }
    }
};

template <typename Production, typename Handler, typename Reader>
constexpr auto action_impl(Handler& handler, Reader& reader)
{
    parse_context<Handler, Production, Production> context(handler, reader.cur());

    if (!parse_production<Production>(context, reader))
    {
        // We had an error, cancel the production.
        LEXY_MOV(context).on(parse_events::production_cancel<Production>{}, reader.cur());
    }

    return LEXY_MOV(context._result);
}
} // namespace lexy::_detail

#endif // LEXY_ACTION_BASE_HPP_INCLUDED

