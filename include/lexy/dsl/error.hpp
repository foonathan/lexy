// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>>
{
    using token_engine = typename Token::token_engine;

    template <typename Handler, typename Reader>
    static constexpr auto token_error(Handler& handler, const Reader& reader,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, Tag>(pos, reader.cur());
        return LEXY_MOV(handler).error(err);
    }
};

template <typename Derived>
template <typename Tag>
LEXY_CONSTEVAL auto token_base<Derived>::error() const
{
    return _toke<Tag, Derived>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Tag, typename Token>
struct _err : rule_base
{
    static constexpr auto has_matcher = false;

    static constexpr auto is_branch = true;

    template <typename Reader>
    struct branch_matcher
    {
        static constexpr auto is_unconditional = true;

        constexpr bool match(Reader&)
        {
            return true;
        }

        template <typename NextParser, typename Handler, typename... Args>
        constexpr auto parse(Handler& handler, Reader& reader, Args&&... args)
        {
            return parser<NextParser>::parse(handler, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&...) ->
            typename Handler::result_type
        {
            auto begin = reader.cur();
            auto end   = reader.cur();
            if constexpr (!std::is_same_v<Token, void>)
            {
                auto copy = reader;
                Token::token_engine::match(copy);
                end = copy.cur();
            }

            auto err = lexy::make_error<Reader, Tag>(begin, end);
            return LEXY_MOV(handler).error(err);
        }
    };

    /// Adds a rule whose match will be part of the error location.
    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        auto t = token(rule);
        return _err<Tag, decltype(t)>{};
    }
};

/// Matches nothing, produces an error with the given tag.
template <typename Tag>
constexpr auto error = _err<Tag, void>{};
} // namespace lexyd

namespace lexyd
{
template <bool Expected, typename Token, typename Tag>
struct _require : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (lexy::engine_peek<typename Token::token_engine>(reader) == Expected)
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            else
            {
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                return LEXY_MOV(handler).error(err);
            }
        }
    };
};

/// Requires that lookahead will match a rule at a location.
template <typename Tag, typename Rule>
LEXY_CONSTEVAL auto require(Rule rule)
{
    auto t = token(rule);
    return _require<true, decltype(t), Tag>{};
}

/// Requires that lookahead does not match a rule at a location.
template <typename Tag, typename Rule>
LEXY_CONSTEVAL auto prevent(Rule rule)
{
    auto t = token(rule);
    return _require<false, decltype(t), Tag>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED
