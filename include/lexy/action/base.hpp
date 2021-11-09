// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ACTION_BASE_HPP_INCLUDED
#define LEXY_ACTION_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/lazy_init.hpp>
#include <lexy/_detail/type_name.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/grammar.hpp>

//=== parse_context ===//
namespace lexy
{
namespace _detail
{
    struct final_parser;

    template <typename Production>
    struct production_parser;

    struct parse_context_var_base
    {
        const void*             id;
        parse_context_var_base* next;

        constexpr parse_context_var_base(const void* id) : id(id), next(nullptr) {}

        template <typename Context>
        constexpr void link(Context& context)
        {
            auto& cb = context.control_block();
            next     = cb.vars;
            cb.vars  = this;
        }

        template <typename Context>
        constexpr void unlink(Context& context)
        {
            auto& cb = context.control_block();
            cb.vars  = next;
        }
    };

    template <typename Id, typename T>
    struct parse_context_var : parse_context_var_base
    {
        static constexpr auto type_id = lexy::_detail::type_id<Id>();

        T value;

        explicit constexpr parse_context_var(T&& value)
        : parse_context_var_base(&type_id), value(LEXY_MOV(value))
        {}

        template <typename Context>
        static constexpr T& get(Context& context)
        {
            auto& cb = context.control_block();

            for (auto cur = cb.vars; cur; cur = cur->next)
                if (cur->id == &type_id)
                    return static_cast<parse_context_var*>(cur)->value;

            LEXY_ASSERT(false, "context variable hasn't been created");
            return LEXY_DECLVAL(T&);
        }
    };

    template <typename Handler>
    struct parse_context_control_block
    {
        Handler                 handler;
        parse_context_var_base* vars;
        int                     cur_depth, max_depth;
        bool                    enable_whitespace_skipping;

        constexpr parse_context_control_block(Handler&& handler, std::size_t max_depth)
        : handler(LEXY_MOV(handler)), vars(nullptr), cur_depth(0),
          max_depth(static_cast<int>(max_depth)), enable_whitespace_skipping(true)
        {}
    };
} // namespace _detail

template <typename Handler, typename Production>
using action_result_type
    = decltype(LEXY_DECLVAL(Handler &&)
                   .get_action_result(true,
                                      LEXY_DECLVAL(typename Handler::template marker<Production>)));

template <typename Handler, typename Production, typename RootProduction>
class _pc
{
    using handler_marker  = typename Handler::template marker<Production>;
    using control_block_t = _detail::parse_context_control_block<Handler>;

public:
    constexpr control_block_t& control_block()
    {
        return *_cb;
    }

    //=== parse context ===//
    using handler         = Handler;
    using production      = Production;
    using root_production = RootProduction;

    constexpr auto& production_context()
    {
        return *this;
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
        -> std::enable_if_t<!std::is_base_of_v<parse_events::_production_event, Event>,
                            decltype(LEXY_DECLVAL(Handler&).on(LEXY_DECLVAL(const handler_marker&),
                                                               ev, LEXY_FWD(args)...))>
    {
        return _cb->handler.on(_marker, ev, LEXY_FWD(args)...);
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
    constexpr explicit _pc(control_block_t* cb, Iterator begin)
    : _cb(cb), _marker(cb->handler.on(parse_events::production_start<Production>{}, begin))
    {}

    template <typename ChildProduction, typename Iterator>
    constexpr auto on(parse_events::production_start<ChildProduction>, Iterator begin)
    {
        // If the new production is a token production, need to re-root it.
        using new_root = std::conditional_t<lexy::is_token_production<ChildProduction>,
                                            ChildProduction, root_production>;
        return _pc<Handler, ChildProduction, new_root>(_cb, begin);
    }

    template <typename Iterator, typename... Args>
    constexpr void on(parse_events::production_finish<Production> ev, Iterator end, Args&&... args)
    {
        _cb->handler.on(_marker, ev, end, LEXY_FWD(args)...);
    }

    template <typename Iterator>
    constexpr void on(parse_events::production_cancel<Production> ev, Iterator pos)
    {
        _cb->handler.on(_marker, ev, pos);
        _cb = nullptr; // invalidate
    }

    control_block_t* _cb;
    handler_marker   _marker;

    template <typename, typename, typename>
    friend class _pc;

    friend struct _detail::final_parser;
    template <typename>
    friend struct _detail::production_parser;
    template <typename P, typename H, typename Reader>
    friend constexpr auto do_action(H&& handler, Reader& reader) -> action_result_type<H, P>;
};
} // namespace lexy

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
        using result_t = decltype(LEXY_MOV(sub_context._marker).get_value());
        if constexpr (std::is_void_v<result_t>)
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        else
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       LEXY_MOV(sub_context._marker).get_value());
    }
};
} // namespace lexy::_detail

namespace lexy
{
template <typename Production, typename Handler, typename Reader>
constexpr auto do_action(Handler&& handler, Reader& reader)
    -> action_result_type<Handler, Production>
{
    static_assert(!std::is_reference_v<Handler>, "need to move handler in");

    _detail::parse_context_control_block<Handler> control_block(LEXY_MOV(handler),
                                                                max_recursion_depth<Production>());
    _pc<Handler, Production, Production>          context(&control_block, reader.position());

    using parser = lexy::parser_for<lexy::production_rule<Production>, _detail::final_parser>;
    if (parser::parse(context, reader))
    {
        return LEXY_MOV(control_block.handler).get_action_result(true, LEXY_MOV(context._marker));
    }
    else
    {
        LEXY_MOV(context).on(parse_events::production_cancel<Production>{}, reader.position());
        return LEXY_MOV(control_block.handler).get_action_result(false, LEXY_MOV(context._marker));
    }
}
} // namespace lexy

#endif // LEXY_ACTION_BASE_HPP_INCLUDED

