// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_LEXEME_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_LEXEME_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <typename Id>
struct _ctx_lcreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Add the lexeme to the context.
            auto lex_ctx = context.insert(Id{}, lexy::lexeme<Reader>());
            return NextParser::parse(lex_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Rule>
struct _ctx_lcapture : rule_base
{
    template <typename NextParser>
    struct parser
    {
        struct _cont
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args)
            {
                context.get(Id{}) = lexy::lexeme(reader, begin);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont>::parse(context, reader, reader.cur(),
                                                         LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Tag, typename Rule>
struct _ctx_lrequire : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _cont
        {
            template <typename Context, typename Reader, typename... RuleArgs>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader,
                                     typename Reader::iterator begin, Args&&... args, RuleArgs&&...)
            {
                auto lhs = context.get(Id{});
                auto rhs = lexy::lexeme(reader, begin);

                if (lexy::_detail::equal_lexemes(lhs, rhs))
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                else
                {
                    auto err = lexy::make_error<Reader, Tag>(rhs.begin(), rhs.end());
                    context.error(err);
                    return false;
                }
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Rule, _cont<Args...>>::parse(context, reader, reader.cur(),
                                                                  LEXY_FWD(args)...);
        }
    };
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, typename Rule>
struct _ctx_lexeme_require
{
    template <typename Tag>
    static constexpr _ctx_lrequire<Id, Tag, Rule> error = {};
};

template <typename Id>
struct _ctx_lexeme
{
    struct id
    {};

    constexpr auto create() const
    {
        return _ctx_lcreate<id>{};
    }

    template <typename Rule>
    constexpr auto capture(Rule) const
    {
        return _ctx_lcapture<id, Rule>{};
    }

    template <typename Rule>
    constexpr auto require(Rule) const
    {
        return _ctx_lexeme_require<id, Rule>{};
    }

    template <typename Tag, typename Rule>
    LEXY_DEPRECATED_ERROR(
        "replace `lexeme.require<tag>(rule)` by `lexeme.require(rule).error<tag>`")
    constexpr auto require(Rule rule) const
    {
        return require(rule).template error<Tag>;
    }
};

/// Declares a lexeme.
template <typename Id>
constexpr auto context_lexeme = _ctx_lexeme<Id>{};
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_LEXEME_HPP_INCLUDED

