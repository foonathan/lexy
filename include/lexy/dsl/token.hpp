// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/match.hpp>

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
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using token_engine = typename Derived::token_engine;
            using continuation = lexy::whitespace_parser<Context, NextParser>;

            auto position = reader.cur();
            if constexpr (lexy::engine_can_fail<token_engine, Reader>)
            {
                if (auto ec = token_engine::match(reader);
                    ec != typename token_engine::error_code())
                {
                    Derived::token_error(context, reader, ec, position);
                    return false;
                }

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
    static constexpr void token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator         pos)
    {
        Token::token_error(context, reader, ec, pos);
    }

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Kind;
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
        auto err = lexy::make_error<Reader, Tag>(pos, reader.cur());
        context.error(err);
    }

    static LEXY_CONSTEVAL auto token_kind()
    {
        return Token::token_kind();
    }

    LEXY_DEPRECATED_ERROR("replace `token.error<Tag>()` by `token.error<Tag>`")
    constexpr auto operator()() const
    {
        return *this;
    }
};
} // namespace lexyd

//=== token rule ===//
namespace lexyd
{
struct _token_dummy_production
{};

template <typename Rule>
struct _token : token_base<_token<Rule>>
{
    struct token_engine : lexy::engine_matcher_base
    {
        enum class error_code
        {
            error = 1,
        };

        template <typename Reader>
        static constexpr error_code match(Reader& reader)
        {
            auto                handler = lexy::_match_handler{};
            lexy::parse_context context(_token_dummy_production{}, handler, reader.cur());

            return lexy::rule_parser<Rule, lexy::context_value_parser>::parse(context, reader)
                       ? error_code()
                       : error_code::error;
        }
    };

    template <typename Context, typename Reader>
    static constexpr void token_error(Context& context, const Reader&,
                                      typename token_engine::error_code,
                                      typename Reader::iterator pos)
    {
        auto err = lexy::make_error<Reader, lexy::missing_token>(pos);
        context.error(err);
    }

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Turns the arbitrary rule into a token by matching it without producing any values.
template <typename Rule>
LEXY_CONSTEVAL auto token(Rule)
{
    if constexpr (lexy::is_token<Rule>)
        return Rule{};
    else
        return _token<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TOKEN_HPP_INCLUDED

