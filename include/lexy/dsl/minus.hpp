// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MINUS_HPP_INCLUDED
#define LEXY_DSL_MINUS_HPP_INCLUDED

#include <lexy/dsl/alternative.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy
{
/// We've matched the Except of a minus.
struct minus_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "minus failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Token, typename Except>
struct _minus : token_base<_minus<Token, Except>>
{
    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Token, Reader> token_parser;
        typename Reader::iterator             end;
        bool                                  minus_failure;

        constexpr explicit tp(const Reader& reader)
        : token_parser(reader), end(reader.position()), minus_failure(false)
        {}

        constexpr bool try_parse(const Reader& reader)
        {
            // Try to parse the token.
            if (!token_parser.try_parse(reader))
            {
                // It didn't match, so we fail.
                minus_failure = false;
                return false;
            }
            // We already remember the end to have it during error reporting as well.
            end = token_parser.end;

            // Check whether Except matches on the same input and we're then at EOF.
            if (auto partial = lexy::partial_reader(reader, token_parser.end);
                lexy::try_match_token(Except{}, partial)
                && partial.peek() == Reader::encoding::eof())
            {
                // Except did match, so we fail.
                minus_failure = true;
                return false;
            }

            // Success.
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (minus_failure)
            {
                auto err = lexy::error<Reader, lexy::minus_failure>(reader.position(), this->end);
                context.on(_ev::error{}, err);
            }
            else
            {
                // Delegate error to the actual token.
                token_parser.report_error(context, reader);
            }
        }
    };
};

/// Matches Token unless Except matches on the input Token matched.
template <typename Token, typename Except>
constexpr auto operator-(Token, Except)
{
    static_assert(lexy::is_token_rule<Token>);
    static_assert(lexy::is_token_rule<Except>);
    return _minus<Token, Except>{};
}
template <typename Token, typename E, typename Except>
constexpr auto operator-(_minus<Token, E>, Except except)
{
    static_assert(lexy::is_token_rule<Except>);
    return _minus<Token, decltype(E{} / except)>{};
}
} // namespace lexyd

#endif // LEXY_DSL_MINUS_HPP_INCLUDED

