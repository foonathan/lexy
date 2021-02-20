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

#ifdef LEXY_IGNORE_DEPRECATED_ERROR
#    define LEXY_DEPRECATED_ERROR(msg)
#else
#    define LEXY_DEPRECATED_ERROR(msg) [[deprecated(msg)]]
#endif

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

namespace lexy
{
struct unknown_token_kind
{};
} // namespace lexy

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
struct _whitespace_tag
{};

template <typename Context>
using _ws_rule = std::conditional_t<
    // We need to disable whitespace if the context is already currently parsing whitespace.
    Context::contains(_whitespace_tag{}), void,
    lexy::production_whitespace<typename Context::production, typename Context::root>>;

template <typename Context, typename NextParser>
using whitespace_parser = rule_parser<lexy::dsl::_wsr<_ws_rule<Context>>, NextParser>;
} // namespace lexy

//=== token ===//
namespace lexyd
{
template <typename Tag, typename Token>
struct _toke;
template <auto Kind, typename Token>
struct _tokk;

template <typename Derived>
struct token_base : _token_base
{
    using token_type = Derived;

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Derived{};
    }

    static constexpr auto is_branch = true;

    template <typename Reader>
    struct branch_matcher
    {
        typename Reader::iterator _begin = {};

        static constexpr auto is_unconditional = false;

        constexpr bool match(Reader& reader)
        {
            _begin = reader.cur();
            return lexy::engine_try_match<typename Derived::token_engine>(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            context.token(Derived::token_kind(), _begin, reader.cur());
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
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
            using continuation = lexy::whitespace_parser<Context, NextParser>;

            auto position = reader.cur();
            if constexpr (lexy::engine_can_fail<token_engine, Reader>)
            {
                if (auto ec = token_engine::match(reader);
                    ec != typename token_engine::error_code())
                    return Derived::token_error(context, reader, ec, position);

                context.token(Derived::token_kind(), position, reader.cur());
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                token_engine::match(reader);
                context.token(Derived::token_kind(), position, reader.cur());
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _toke<Tag, Derived> error = _toke<Tag, Derived>{};

    template <auto Kind>
    static constexpr _tokk<Kind, Derived> kind = _tokk<Kind, Derived>{};
};

template <auto Kind, typename Token>
struct _tokk : token_base<_tokk<Kind, Token>>
{
    using token_engine = typename Token::token_engine;

    template <typename Context, typename Reader>
    static constexpr auto token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        return Token::token_error(context, reader, ec, pos);
    }

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Kind;
    }
};
} // namespace lexyd

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
    using result_type = typename Handler::template result_type_for<Production>;

    constexpr auto sink() const
    {
        return _handler->get_sink(Production{});
    }

    template <typename Iterator>
    constexpr void whitespace(Iterator begin, Iterator end)
    {
        _handler->whitespace(begin, end);
    }

    template <typename TokenKind, typename Iterator>
    constexpr void token(TokenKind kind, Iterator begin, Iterator end)
    {
        _handler->token(kind, begin, end);
    }

    template <typename... Args>
    constexpr result_type value(Args&&... args) &&
    {
        return _handler->finish_production(Production{}, LEXY_MOV(_state), LEXY_FWD(args)...);
    }

    template <typename Error>
    constexpr result_type error(Error&& error) &&
    {
        return _handler->error(Production{}, LEXY_MOV(_state), LEXY_FWD(error));
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
        using result_type = typename Handler::template result_type_for<Production>;

        constexpr auto sink() const
        {
            return _parent->sink();
        }

        template <typename Iterator>
        constexpr void whitespace(Iterator begin, Iterator end)
        {
            _parent->whitespace(begin, end);
        }

        template <typename TokenKind, typename Iterator>
        constexpr void token(TokenKind kind, Iterator begin, Iterator end)
        {
            _parent->token(kind, begin, end);
        }

        template <typename... Args>
        constexpr result_type value(Args&&... args) &&
        {
            return LEXY_MOV(*_parent).value(LEXY_FWD(args)...);
        }

        template <typename Error>
        constexpr result_type error(Error&& error) &&
        {
            return LEXY_MOV(*_parent).error(LEXY_FWD(error));
        }

    private:
        Parent*                 _parent;
        LEXY_EMPTY_MEMBER State _state;
    };

    Handler*                       _handler;
    LEXY_EMPTY_MEMBER HandlerState _state;
};

template <typename Production, typename Handler, typename Iterator>
parse_context(Production p, Handler& handler, Iterator position)
    -> parse_context<Production, Handler, decltype(handler.start_production(p, position))>;

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
template <typename Context>
struct context_discard_parser
{
    template <typename NewContext, typename Reader, typename... Args>
    LEXY_DSL_FUNC auto parse(NewContext&, Reader&, Args&&...)
    {
        static_assert(sizeof...(Args) == 0, "looped rule must not produce any values");
        static_assert(std::is_same_v<Context, NewContext>,
                      "looped rule cannot add state to the context");
        return typename Context::result_type(lexy::result_empty);
    }
};
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED

