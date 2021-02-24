// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Tag, typename Token>
struct _err : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&...)
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
            context.error(err);
            return false;
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
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (lexy::engine_peek<typename Token::token_engine>(reader) == Expected)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
            {
                auto err = lexy::make_error<Reader, Tag>(reader.cur());
                context.error(err);
                return false;
            }
        }
    };
};

template <bool Expected, typename Token>
struct _require_dsl
{
    template <typename Tag>
    static constexpr _require<Expected, Token, Tag> error = _require<Expected, Token, Tag>{};
};

/// Requires that lookahead will match a rule at a location.
template <typename Rule>
LEXY_CONSTEVAL auto require(Rule rule)
{
    auto t = token(rule);
    return _require_dsl<true, decltype(t)>{};
}

/// Requires that lookahead does not match a rule at a location.
template <typename Rule>
LEXY_CONSTEVAL auto prevent(Rule rule)
{
    auto t = token(rule);
    return _require_dsl<false, decltype(t)>{};
}

template <typename Tag, typename Rule>
LEXY_DEPRECATED_ERROR("replace `require<Tag>(rule)` by `require(rule).error<Tag>`")
LEXY_CONSTEVAL auto require(Rule rule)
{
    return require(rule).template error<Tag>;
}
template <typename Tag, typename Rule>
LEXY_DEPRECATED_ERROR("replace `prevent<Tag>(rule)` by `prevent(rule).error<Tag>`")
LEXY_CONSTEVAL auto prevent(Rule rule)
{
    return prevent(rule).template error<Tag>;
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED

