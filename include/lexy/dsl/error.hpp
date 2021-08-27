// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
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

            auto err = lexy::error<Reader, Tag>(begin, end);
            context.on(_ev::error{}, err);
            return false;
        }
    };

    /// Adds a rule whose match will be part of the error location.
    template <typename Rule>
    constexpr auto operator()(Rule rule) const
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
template <typename Branch, typename Error>
struct _must : rule_base
{
    static constexpr auto is_branch               = true;
    static constexpr auto is_unconditional_branch = false;

    template <typename NextParser>
    struct parser : lexy::rule_parser<Branch, NextParser>
    {
        // inherit try_parse() from Branch

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using parser = lexy::rule_parser<Branch, NextParser>;
            auto result  = parser::try_parse(context, reader, LEXY_FWD(args)...);
            if (result != lexy::rule_try_parse_result::backtracked)
                return static_cast<bool>(result);
            else
                return lexy::rule_parser<Error, NextParser>::parse(context, reader,
                                                                   LEXY_FWD(args)...);
        }
    };
};

template <typename Branch>
struct _must_dsl
{
    template <typename Tag>
    struct _err : _must<Branch, lexyd::_err<Tag, void>>
    {
        template <typename Rule>
        constexpr auto operator()(Rule rule) const
        {
            auto err = lexyd::error<Tag>(rule);
            return _must<Branch, decltype(err)>{};
        }
    };

    template <typename Tag>
    static constexpr _err<Tag> error = _err<Tag>{};
};

/// Tries to parse `Branch` and raises a specific error on failure.
/// It can still be used as a branch rule; then behaves exactly like `Branch.`
template <typename Branch>
constexpr auto must(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    static_assert(!Branch::is_unconditional_branch);
    return _must_dsl<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED

