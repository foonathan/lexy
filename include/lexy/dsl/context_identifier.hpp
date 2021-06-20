// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/identifier.hpp>

namespace lexy
{
struct different_identifier
{
    static LEXY_CONSTEVAL auto name()
    {
        return "different identifier";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Id>
struct _ctx_icreate : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(!Context::contains(Id{}));
            auto identifier_ctx = context.insert(Id{}, lexy::lexeme<Reader>());
            return NextParser::parse(identifier_ctx, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Identifier>
struct _ctx_icap : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename... Args>
        struct _cont
        {
            template <typename Context, typename Reader>
            LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args,
                                     lexy::lexeme<Reader> lexeme)
            {
                context.get(Id{}) = lexeme;
                return NextParser::parse(context, reader, LEXY_FWD(args)..., lexeme);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            return lexy::rule_parser<Identifier, _cont<Args...>>::try_parse(context, reader,
                                                                            LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return lexy::rule_parser<Identifier, _cont<Args...>>::parse(context, reader,
                                                                        LEXY_FWD(args)...);
        }
    };
};

template <typename Id, typename Identifier, typename Tag>
struct _ctx_irem : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            // Trie to parse the pattern of the identifier only: we don't need a value nor need to
            // check for reserved identifier, because it must match the one we've succesfully parsed
            // earlier.
            using engine = typename decltype(Identifier{}.pattern())::token_engine;
            auto begin   = reader.cur();
            if (auto ec = engine::match(reader); ec != typename engine::error_code())
                return lexy::rule_try_parse_result::backtracked;
            auto lexeme = lexy::lexeme(reader, begin);

            if (!lexy::_detail::equal_lexemes(context.get(Id{}), lexeme))
                return lexy::rule_try_parse_result::backtracked;

            context.token(lexy::identifier_token_kind, lexeme.begin(), lexeme.end());
            // Don't produce a value.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Again, parse pattern only.
            using pattern = decltype(Identifier{}.pattern());
            using engine  = typename decltype(Identifier{}.pattern())::token_engine;
            auto begin    = reader.cur();
            if (auto ec = engine::match(reader); ec != typename engine::error_code())
            {
                pattern::token_error(context, reader, ec, begin);
                return false;
            }
            auto lexeme = lexy::lexeme(reader, begin);

            // Check that the identifier is the same.
            if (!lexy::_detail::equal_lexemes(context.get(Id{}), lexeme))
            {
                using tag
                    = std::conditional_t<std::is_void_v<Tag>, lexy::different_identifier, Tag>;
                auto err = lexy::make_error<Reader, tag>(lexeme.begin(), lexeme.end());
                context.error(err);
                // We can trivially recover, as we still had a valid identifier.
            }

            context.token(lexy::identifier_token_kind, lexeme.begin(), lexeme.end());
            // Don't produce a value.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _ctx_irem<Id, Identifier, Error> error = {};
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, typename Identifier>
struct _ctx_identifier
{
    struct id
    {};

    constexpr auto create() const
    {
        return _ctx_icreate<id>{};
    }

    constexpr auto capture() const
    {
        return _ctx_icap<id, Identifier>{};
    }

    constexpr auto rematch() const
    {
        return _ctx_irem<id, Identifier, void>{};
    }
};

/// Declares a context variable that stores one instance of the given identifier.
template <typename Id, typename Leading, typename Trailing, typename... Reserved>
constexpr auto context_identifier(_id<Leading, Trailing, Reserved...>)
{
    return _ctx_identifier<Id, _id<Leading, Trailing, Reserved...>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED

