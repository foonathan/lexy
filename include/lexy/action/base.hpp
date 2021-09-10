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
        decltype(LEXY_DECLVAL(Handler&).on(LEXY_DECLVAL(const handler_marker<Handler, Production>&),
                                           ev, LEXY_FWD(args)...))>
    {
        LEXY_ASSERT(_handler, "using already finished context");
        return _handler->on(_marker, ev, LEXY_FWD(args)...);
    }

    //=== context variables ===//
    template <typename Id>
    static LEXY_CONSTEVAL bool contains(Id)
    {
        return false;
    }

    template <typename Id>
    constexpr auto get(Id) const
    {
        static_assert(lexy::_detail::error<Id>, "context does not contain a variable with that id");
        return nullptr;
    }

#ifndef _MSC_VER
private:
    // MSVC is a bad compiler and should feel bad.
    // Or at least fix their friend declarations.
#endif

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

        _handler = nullptr; // invalidate
    }

    template <typename Iterator>
    constexpr void on(parse_events::production_cancel<Production> ev, Iterator pos) &&
    {
        _handler->on(LEXY_MOV(_marker), ev, pos);
        _handler = nullptr; // invalidate
    }

    Handler*                                                  _handler;
    handler_marker<Handler, Production>                       _marker;
    lazy_init<handler_production_result<Handler, Production>> _result;

    template <typename, typename, typename>
    friend class parse_context;

    friend struct final_parser;
    template <typename>
    friend struct production_parser;
    template <typename P, typename H, typename Reader>
    friend constexpr auto action_impl(H& handler, Reader& reader)
        -> lazy_init<handler_production_result<H, P>>;
};
} // namespace lexy::_detail

//=== do_action ===//
namespace lexy::_detail
{
struct final_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        using event = parse_events::production_finish<typename Context::production>;
        LEXY_MOV(context.production_context()).on(event{}, reader.position(), LEXY_FWD(args)...);
        return true;
    }
};

template <typename Production>
struct production_parser
{
    template <typename Context, typename Reader>
    static constexpr auto get_sub_context(Context& context, const Reader& reader)
    {
        return context.production_context().on(parse_events::production_start<Production>{},
                                               reader.position());
    }
    template <typename Context, typename Reader>
    using sub_context_t = decltype(get_sub_context(LEXY_DECLVAL(Context&), LEXY_DECLVAL(Reader)));

    template <typename SubContext, typename Reader>
    static constexpr void cancel_sub_context(SubContext& sub_context, const Reader& reader)
    {
        LEXY_MOV(sub_context).on(parse_events::production_cancel<Production>{}, reader.position());
    }

    template <typename NextParser, typename Context, typename Reader, typename SubContext,
              typename... Args>
    LEXY_PARSER_FUNC static bool finish(Context& context, Reader& reader, SubContext& sub_context,
                                        Args&&... args)
    {
        // Might need to skip whitespace, according to the original context.
        using continuation
            = std::conditional_t<lexy::is_token_production<Production>,
                                 lexy::whitespace_parser<Context, NextParser>, NextParser>;

        // Pass the produced value to the next parser.
        using result_t = handler_production_result<typename Context::handler, Production>;
        if constexpr (std::is_void_v<result_t>)
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        else
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       LEXY_MOV(*sub_context._result));
    }
};

template <typename Production, typename Handler, typename Reader>
constexpr auto action_impl(Handler& handler, Reader& reader)
    -> lazy_init<handler_production_result<Handler, Production>>
{
    parse_context<Handler, Production, Production> context(handler, reader.position());

    using parser = lexy::parser_for<lexy::production_rule<Production>, final_parser>;
    if (!parser::parse(context, reader))
    {
        // We had an error, cancel the production.
        LEXY_ASSERT(!context._result, "result must be empty on cancel");
        LEXY_MOV(context).on(parse_events::production_cancel<Production>{}, reader.position());
    }

    return LEXY_MOV(context._result);
}
} // namespace lexy::_detail

namespace lexy
{
template <typename Production, typename Handler, typename Reader>
constexpr auto do_action(Handler&& handler, Reader& reader)
{
    static_assert(!std::is_reference_v<Handler>, "need to move handler in");
    auto result = _detail::action_impl<Production>(handler, reader);
    if (result)
    {
        using result_t = _detail::handler_production_result<Handler, Production>;
        if constexpr (std::is_void_v<result_t>)
            return LEXY_MOV(handler).template get_result_value<Production>();
        else
            return LEXY_MOV(handler).template get_result_value<Production>(LEXY_MOV(*result));
    }
    else
    {
        return LEXY_MOV(handler).template get_result_empty<Production>();
    }
}
} // namespace lexy

#endif // LEXY_ACTION_BASE_HPP_INCLUDED

