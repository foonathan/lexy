// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED

#include <lexy/action/match.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct missing_token
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing token";
    }
};
} // namespace lexy

//=== token_base ===//
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

    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using token_engine = typename Derived::token_engine;

            auto begin = reader.position();
            if (!lexy::engine_try_match<token_engine>(reader))
            {
                context.on(_ev::backtracked{}, begin, reader.position());
                return lexy::rule_try_parse_result::backtracked;
            }
            auto end = reader.position();
            context.on(_ev::token{}, Derived{}, begin, end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return static_cast<lexy::rule_try_parse_result>(
                continuation::parse(context, reader, LEXY_FWD(args)...));
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using token_engine = typename Derived::token_engine;

            auto position = reader.position();
            if constexpr (lexy::engine_can_fail<token_engine, Reader>)
            {
                if (auto ec = token_engine::match(reader);
                    ec != typename token_engine::error_code())
                {
                    Derived::token_error(context, reader, ec, position);
                    return false;
                }
            }
            else
            {
                token_engine::match(reader);
            }
            context.on(_ev::token{}, Derived{}, position, reader.position());

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...);
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
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        Token::token_error(context, reader, ec, pos);
    }
};

template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>>
{
    using token_engine = typename Token::token_engine;

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, Tag>(pos, reader.position());
        context.on(_ev::error{}, err);
    }
};
} // namespace lexyd

namespace lexy
{
template <auto Kind, typename Token>
constexpr auto token_kind_of<lexy::dsl::_tokk<Kind, Token>> = Kind;
template <typename Tag, typename Token>
constexpr auto token_kind_of<lexy::dsl::_toke<Tag, Token>> = token_kind_of<Token>;
} // namespace lexy

//=== token rule ===//
namespace lexyd
{
template <typename Rule>
struct _token : token_base<_token<Rule>>
{
    struct _production
    {
        static constexpr auto rule = Rule{};
    };

    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            return lexy::do_action<_production>(lexy::match_handler(), reader) ? error_code()
                                                                               : error_code::error;
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::error<Reader, lexy::missing_token>(pos);
        context.on(_ev::error{}, err);
    }
};

/// Turns the arbitrary rule into a token by matching it without producing any values.
template <typename Rule>
constexpr auto token(Rule)
{
    if constexpr (lexy::is_token_rule<Rule>)
        return Rule{};
    else
        return _token<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TOKEN_HPP_INCLUDED

