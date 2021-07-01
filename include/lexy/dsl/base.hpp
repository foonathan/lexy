// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/lazy_init.hpp>
#include <lexy/engine/base.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/base.hpp>

#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

#ifdef LEXY_IGNORE_DEPRECATED_ERROR
#    define LEXY_DEPRECATED_ERROR(msg)
#else
#    define LEXY_DEPRECATED_ERROR(msg) [[deprecated(msg)]]
#endif

//=== whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsr;
template <>
struct _wsr<void> : rule_base
{
    template <typename NextParser>
    using parser = NextParser;
};
} // namespace lexyd

namespace lexy
{
struct _tag_no_whitespace
{};
struct _tag_whitespace
{};

template <typename Context>
using _ws_rule = std::conditional_t<
    // We need to disable whitespace if the context is already currently parsing whitespace.
    Context::contains(_tag_no_whitespace{}) || Context::contains(_tag_whitespace{}), void,
    lexy::production_whitespace<typename Context::production, typename Context::root>>;

template <typename Context, typename NextParser>
using whitespace_parser = rule_parser<lexy::dsl::_wsr<_ws_rule<Context>>, NextParser>;
} // namespace lexy

//=== parse_context ===//
namespace lexy
{
/// Stores contextual information for parsing the given production.
template <typename Production, typename Handler, typename HandlerState, typename Root = Production>
class parse_context
{
    static_assert(!lexy::is_token_production<Production> || std::is_same_v<Production, Root>,
                  "don't specify Root argument explicitly");

    template <typename ChildProduction, typename Iterator>
    using _parse_context_for = parse_context<
        ChildProduction, Handler,
        decltype(LEXY_DECLVAL(Handler&).start_production(ChildProduction{}, Iterator{})),
        // If it's a token we need to re-root it.
        std::conditional_t<lexy::is_token_production<ChildProduction>, ChildProduction, Root>>;

public:
    template <typename Iterator>
    constexpr explicit parse_context(Production p, Handler& handler, Iterator begin)
    : _handler(&handler), _state(_handler->start_production(p, begin))
    {}

    constexpr Handler& handler() const noexcept
    {
        return *_handler;
    }

    template <typename ChildProduction, typename Iterator>
    constexpr auto production_context(ChildProduction p, Iterator position) const
    {
        return _parse_context_for<ChildProduction, Iterator>(p, *_handler, position);
    }

    template <typename Id, typename T>
    constexpr auto insert(Id, T&& value)
    {
        return _stateful_context<parse_context, Id, std::decay_t<T>>(*this, LEXY_FWD(value));
    }

    template <typename Id>
    static LEXY_CONSTEVAL bool contains(Id)
    {
        return false;
    }

    template <typename Id>
    constexpr auto get(Id)
    {
        static_assert(lexy::_detail::error<Id>, "context does not contain a state with that id");
        return 0;
    }

    using production  = Production;
    using root        = Root;
    using return_type = typename Handler::template return_type_for<Production>;

    constexpr auto sink() const
    {
        return _handler->get_sink(Production{});
    }

    template <typename TokenKind, typename Iterator>
    constexpr void token(TokenKind kind, Iterator begin, Iterator end)
    {
        _handler->token(kind, begin, end);
    }

    template <typename... Args>
    constexpr void value(Args&&... args)
    {
        if constexpr (std::is_void_v<return_type>)
        {
            _handler->finish_production(Production{}, LEXY_MOV(_state), LEXY_FWD(args)...);
            _value.emplace();
        }
        else
        {
            auto value
                = _handler->finish_production(Production{}, LEXY_MOV(_state), LEXY_FWD(args)...);
            _value.emplace(LEXY_MOV(value));
        }
    }

    template <typename Error>
    constexpr void error(Error&& error)
    {
        _handler->error(Production{}, LEXY_MOV(_state), LEXY_FWD(error));
    }

    // Precondition: Either finish() or backtrack() must be called on every created context.
    constexpr auto finish() &&
    {
        if constexpr (!std::is_void_v<return_type>)
            return LEXY_MOV(*_value);
    }
    constexpr void backtrack() &&
    {
        _handler->backtrack_production(Production{}, LEXY_MOV(_state));
    }

private:
    template <typename Parent, typename Id, typename State>
    class _stateful_context
    {
    public:
        template <typename T>
        constexpr explicit _stateful_context(Parent& parent, T&& value)
        : _parent(&parent), _state(LEXY_FWD(value))
        {}

        constexpr Handler& handler() const noexcept
        {
            return _parent->handler();
        }

        template <typename ChildProduction, typename Iterator>
        constexpr auto production_context(ChildProduction p, Iterator position) const
        {
            static_assert(!contains(_tag_whitespace{}),
                          "whitespace rule must not contain dsl::p/dsl::recurse");
            return _parse_context_for<ChildProduction, Iterator>(p, _parent->handler(), position);
        }

        template <typename Id2, typename T>
        constexpr auto insert(Id2, T&& value)
        {
            return _stateful_context<_stateful_context, Id2, std::decay_t<T>>(*this,
                                                                              LEXY_FWD(value));
        }

        template <typename Id2>
        static LEXY_CONSTEVAL bool contains(Id2 id)
        {
            (void)id;
            if constexpr (std::is_same_v<Id, Id2>)
                return true;
            else
                return Parent::contains(id);
        }

        template <typename Id2>
        constexpr auto& get(Id2 id)
        {
            (void)id;
            if constexpr (std::is_same_v<Id2, Id>)
                return _state;
            else
                return _parent->get(id);
        }

        using production  = Production;
        using root        = Root;
        using return_type = typename Handler::template return_type_for<Production>;

        constexpr auto sink() const
        {
            return _parent->sink();
        }

        template <typename TokenKind, typename Iterator>
        constexpr void token(TokenKind kind, Iterator begin, Iterator end)
        {
            if constexpr (contains(_tag_whitespace{}))
                // Ignore tokens while skipping whitespace.
                return;

            _parent->token(kind, begin, end);
        }

        template <typename... Args>
        constexpr void value(Args&&... args)
        {
            _parent->value(LEXY_FWD(args)...);
        }

        template <typename Error>
        constexpr void error(Error&& error)
        {
            _parent->error(LEXY_FWD(error));
        }

        constexpr auto finish() &&
        {
            return LEXY_MOV(*_parent).finish();
        }
        constexpr void backtrack() &&
        {
            LEXY_MOV(*_parent).backtrack();
        }

    private:
        Parent*                 _parent;
        LEXY_EMPTY_MEMBER State _state;
    };

    lexy::_detail::lazy_init<return_type> _value;
    Handler*                              _handler;
    LEXY_EMPTY_MEMBER HandlerState        _state;
};

template <typename Production, typename Handler, typename Iterator>
parse_context(Production p, Handler& handler, Iterator position)
    -> parse_context<Production, Handler, decltype(handler.start_production(p, position))>;

/// A final parser that forwards all elements to the context.
struct context_value_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(Context& context, Reader&, Args&&... args)
    {
        context.value(LEXY_FWD(args)...);
        return true;
    }
};

/// A final parser that drops all arguments; creating an empty result.
template <typename Context>
struct context_discard_parser
{
    template <typename NewContext, typename Reader, typename... Args>
    LEXY_DSL_FUNC bool parse(NewContext&, Reader&, Args&&...)
    {
        static_assert(sizeof...(Args) == 0, "looped rule must not produce any values");
        static_assert(std::is_same_v<Context, NewContext>,
                      "looped rule cannot add state to the context");
        return true;
    }
};
} // namespace lexy

namespace lexy::_detail
{
template <typename Production, typename Rule = lexy::production_rule<Production>, typename Handler,
          typename Reader>
constexpr auto parse_impl(Handler& handler, Reader& reader)
{
    lexy::parse_context context(Production{}, handler, reader.cur());

    using return_type = typename decltype(context)::return_type;
    lexy::_detail::lazy_init<return_type> result;

    if (lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader))
    {
        if constexpr (std::is_void_v<return_type>)
        {
            LEXY_MOV(context).finish();
            result.emplace();
        }
        else
        {
            result.emplace(LEXY_MOV(context).finish());
        }
    }
    else
    {
        LEXY_MOV(context).backtrack();
    }

    return result;
}
} // namespace lexy::_detail

#endif // LEXY_DSL_BASE_HPP_INCLUDED

