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

template <typename Derived, typename BranchKind = branch_base>
struct token_base : _token_base, BranchKind
{
    using token_type = Derived;

    template <typename Context, typename Reader>
    struct bp
    {
        typename Reader::iterator end;

        constexpr auto try_parse(const Context&, const Reader& reader)
        {
            lexy::token_parser_for<Derived, Reader> parser(reader);
            auto                                    result = parser.try_parse(reader);
            end                                            = parser.end;
            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::token{}, Derived{}, reader.position(), end);
            reader.set_position(end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    LEXY_PARSER_FUNC static bool token_parse(Context& context, Reader& reader)
    {
        auto                                    begin = reader.position();
        lexy::token_parser_for<Derived, Reader> parser(reader);

        using try_parse_result = decltype(parser.try_parse(reader));
        if constexpr (std::is_same_v<try_parse_result, std::true_type>)
        {
            parser.try_parse(reader);
        }
        else
        {
            if (!parser.try_parse(reader))
            {
                parser.report_error(context, reader);

                if (begin != parser.end)
                    context.on(_ev::token{}, lexy::error_token_kind, reader.position(), parser.end);
                reader.set_position(parser.end);

                return false;
            }
        }

        context.on(_ev::token{}, Derived{}, begin, parser.end);
        reader.set_position(parser.end);

        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!token_parse(context, reader))
                return false;
            else
                return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                           LEXY_FWD(args)...);
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
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Token, Reader> token_parser;
        typename Reader::iterator             end;

        constexpr bool try_parse(const Reader& reader)
        {
            // Forward.
            auto result = token_parser.try_parse(reader);
            end         = token_parser.end;
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            token_parser.report_error(context, reader);
        }
    };
};

template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>>
{
    template <typename Reader>
    struct tp : lexy::token_parser_for<Token, Reader>
    {
        constexpr explicit tp(const Reader& reader) : lexy::token_parser_for<Token, Reader>(reader)
        {}

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            // Report a different error.
            auto err = lexy::error<Reader, Tag>(reader.position(), this->end);
            context.on(_ev::error{}, err);
        }
    };
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

    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            // We match a dummy production that only consists of the rule.
            auto success = lexy::do_action<_production>(lexy::match_handler(), reader);
            end          = reader.position();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::missing_token>(reader.position(), end);
            context.on(_ev::error{}, err);
        }
    };
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

