// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/engine/base.hpp>
#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/result.hpp>

#define LEXY_DSL_FUNC LEXY_FORCE_INLINE static constexpr

//=== rule ===//
#if 0
struct Rule : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename ... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args)
            -> typename Context::result_type
        {
            if (/* match reader */)
                return NextParser::parse(context, reader, LEXY_FWD(args)..., /* rule arguments */);
            else
                return LEXY_MOV(context).error(/* error */);
        }
    };
};
#endif

namespace lexyd
{
struct rule_base
{
    static constexpr auto is_branch = false;
};

struct branch_base : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    using parser = NextParser;
};

struct _token_base : rule_base
{};
} // namespace lexyd

namespace lexy
{
// We use a shorthand namespace to decrease symbol size.
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;
template <typename T>
constexpr bool is_token = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_branch = [] {
    if constexpr (is_rule<T>)
        return T::is_branch;
    else
        return false;
}();

template <typename Rule, typename NextParser>
using rule_parser = typename Rule::template parser<NextParser>;
template <typename Branch, typename Reader>
using branch_matcher = typename Branch::template branch_matcher<Reader>;
} // namespace lexy

namespace lexy
{
struct _no_context_state
{};

/// Stores contextual information for parsing.
template <typename Input, typename Handler>
class parse_context
{
public:
    template <typename... Args>
    constexpr parse_context(const Input& input, Args&&... args)
    : _input(&input), _handler(LEXY_FWD(args)...)
    {}

    parse_context(const parse_context&) = delete;
    parse_context& operator=(const parse_context&) = delete;

    constexpr const Input& input() const noexcept
    {
        return *_input;
    }

    constexpr Handler& handler() noexcept
    {
        return _handler;
    }

private:
    const Input* _input;
    Handler      _handler;
};

/// Stores contextual information for parsing the given production.
template <typename Production, typename Input, typename Handler>
class production_context
{
    using _iterator = typename input_reader<Input>::iterator;

    static constexpr auto _start_production(Handler& handler, _iterator pos)
    {
        if constexpr (std::is_void_v<decltype(handler.start_production(Production{}, pos))>)
        {
            handler.start_production(Production{}, pos);
            return _no_context_state{};
        }
        else
        {
            return handler.start_production(Production{}, pos);
        }
    }

    using _handler_state_t = decltype(_start_production(LEXY_DECLVAL(Handler&), _iterator()));

public:
    //=== constructors ===//
    constexpr explicit production_context(parse_context<Input, Handler>& context, Production,
                                          _iterator                      current_position)
    : _ctx(&context), _pos(current_position),
      _handler_state(_start_production(_ctx->handler(), current_position))
    {}

    template <typename ParentProd>
    constexpr explicit production_context(production_context<ParentProd, Input, Handler>& context,
                                          Production p, _iterator current_position)
    : production_context(context.root(), p, current_position)
    {}

    production_context(production_context&&) = default;
    production_context& operator=(production_context&&) = default;

    production_context(const production_context&) = delete;
    production_context& operator=(const production_context&) = delete;

    //=== context ===//
    constexpr parse_context<Input, Handler>& root() noexcept
    {
        return *_ctx;
    }

    //=== handler interface ===//
    using production  = Production;
    using result_type = typename Handler::template result_type_for<Production>;

    constexpr auto sink() const
    {
        return _ctx->handler().sink(Production{});
    }

    template <typename Error>
    constexpr result_type error(Error&& error) &&
    {
        lexy::error_context<Production, Input> err_context(_ctx->input(), _pos);
        return _ctx->handler().error(LEXY_MOV(err_context), LEXY_FWD(error));
    }

    template <typename... Args>
    constexpr result_type value(Args&&... args) &&
    {
        if constexpr (std::is_same_v<_handler_state_t, _no_context_state>)
            return _ctx->handler().finish_production(Production{}, LEXY_FWD(args)...);
        else
            return _ctx->handler().finish_production(LEXY_MOV(_handler_state), Production{},
                                                     LEXY_FWD(args)...);
    }

private:
    parse_context<Input, Handler>*     _ctx;
    _iterator                          _pos;
    LEXY_EMPTY_MEMBER _handler_state_t _handler_state;
};

template <typename Production, typename Input, typename Handler, typename Iter>
production_context(parse_context<Input, Handler>&, Production, Iter)
    -> production_context<Production, Input, Handler>;
template <typename Parent, typename Input, typename Handler, typename Production, typename Iter>
production_context(production_context<Parent, Input, Handler>&, Production, Iter)
    -> production_context<Production, Input, Handler>;

/// A final parser that forwards all elements to the context.
struct context_value_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Context& context, Reader&, Args&&... args) ->
        typename Context::result_type
    {
        return LEXY_MOV(context).value(LEXY_FWD(args)...);
    }
};

/// A final parser that drops all arguments; creating an empty result.
struct context_discard_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(Context&, Reader&, Args&&...)
    {
        return typename Context::result_type(lexy::result_empty);
    }
};

} // namespace lexy

namespace lexyd
{
template <typename Derived>
struct token_base : _token_base
{
    static constexpr auto is_branch = true;

    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            return lexy::engine_try_match<typename Derived::token_engine>(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            using token_engine = typename Derived::token_engine;
            if constexpr (lexy::engine_can_fail<token_engine, Reader>)
            {
                auto position = reader.cur();
                if (auto ec = token_engine::match(reader);
                    ec == typename token_engine::error_code())
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                    return Derived::token_error(context, reader, ec, position);
            }
            else
            {
                token_engine::match(reader);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    //=== dsl ===//
    template <typename Tag>
    LEXY_CONSTEVAL auto error() const;
};
} // namespace lexyd

#endif // LEXY_DSL_BASE_HPP_INCLUDED

