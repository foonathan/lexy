// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/engine/while.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_WHITESPACE
#    define LEXY_DEPRECATED_WHITESPACE
#else
#    define LEXY_DEPRECATED_WHITESPACE                                                             \
        [[deprecated("operator[] has been deprecated; use dsl::whitespace(ws) instead.")]]
#endif

namespace lexyd
{
template <typename Rule>
struct _wsr : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (lexy::is_token<Rule>)
            {
                // Parsing a token repeatedly cannot fail, so we can optimize it using an engine.
                using engine = lexy::engine_while<typename Rule::token_engine>;

                auto begin = reader.cur();
                engine::match(reader);
                auto end = reader.cur();

                context.token(Rule::token_kind(), begin, end);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // We need to mark the context with the tag to prevent infinite recursion.
                auto ws_context = context.insert(lexy::_whitespace_tag{}, lexy::_whitespace_tag{});

                // We can then parse the rule repeatedly using the special context.
                using loop_parser
                    = lexy::rule_parser<decltype(loop(Rule{} | break_)),
                                        lexy::context_discard_parser<decltype(ws_context)>>;
                if (!loop_parser::parse(ws_context, reader))
                    return false;

                // And continue with normal parsing.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    template <typename R>
    friend LEXY_CONSTEVAL auto operator|(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} | r)>{};
    }
    template <typename R>
    friend LEXY_CONSTEVAL auto operator|(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r | Rule{})>{};
    }

    template <typename R>
    friend LEXY_CONSTEVAL auto operator/(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} / r)>{};
    }
    template <typename R>
    friend LEXY_CONSTEVAL auto operator/(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r / Rule{})>{};
    }
};

struct _ws : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    /// Overrides implicit whitespace detection.
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule) const
    {
        return _wsr<Rule>{};
    }
};

/// Matches whitespace.
constexpr auto whitespace = _ws{};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _wsn : rule_base
{
    static constexpr auto is_branch = lexy::is_branch<Rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Rule, Reader> _impl;

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr auto parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the remainder of the branch using the context that doesn't allow inner
            // whitespace.
            auto ws_context    = context.insert(lexy::_whitespace_tag{}, lexy::_whitespace_tag{});
            using continuation = typename parser<NextParser>::_cont;
            return _impl.template parse<continuation>(ws_context, reader, context,
                                                      LEXY_FWD(args)...);
        }
    };

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
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the rule using the context that doesn't allow inner whitespace.
            auto ws_context = context.insert(lexy::_whitespace_tag{}, lexy::_whitespace_tag{});
            return lexy::rule_parser<Rule, _cont>::parse(ws_context, reader, context,
                                                         LEXY_FWD(args)...);
        }
    };
};

/// Disables automatic skipping of whitespace for all tokens of the given rule.
template <typename Rule>
LEXY_CONSTEVAL auto no_whitespace(Rule)
{
    if constexpr (lexy::is_token<Rule>)
        return Rule{}; // Token already behaves that way.
    else
        return _wsn<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule, typename Whitespace>
struct _wsd : decltype(loop(token(Whitespace{}) | break_) + Rule{})
{
    static constexpr bool is_branch = lexy::is_branch<Rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Rule, Reader> _impl;

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            using ws = decltype(token(loop(Whitespace{} | break_)));
            lexy::engine_try_match<typename ws::token_engine>(reader);
            return _impl.match(reader);
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return _impl.template parse<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Tag>
    LEXY_CONSTEVAL auto error() const
    {
        static_assert(lexy::is_token<Rule>);
        return Rule{}.template error<Tag>();
    }
};

/// Matches whitespace before parsing rule.
template <typename Rule, typename Whitespace>
LEXY_DEPRECATED_WHITESPACE LEXY_CONSTEVAL auto whitespaced(Rule, Whitespace)
{
    return _wsd<Rule, Whitespace>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED

