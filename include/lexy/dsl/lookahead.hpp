// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
/// We've failed to match a lookahead.
struct lookahead_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "lookahead failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Needle, typename End, typename Tag>
struct _look : branch_base
{
    template <typename Context, typename Reader>
    struct bp
    {
        static constexpr bool try_parse(Context& context, Reader reader)
        {
            auto begin = reader.position();

            auto result = [&] {
                while (true)
                {
                    // Try to match Needle.
                    if (lexy::try_match_token(Needle{}, reader))
                        // We found it.
                        return true;
                    // Check whether we've reached the End.
                    else if (reader.peek() == Reader::encoding::eof()
                             || lexy::try_match_token(End{}, reader))
                        // We've failed.
                        return false;
                    else
                        // Try again.
                        reader.bump();
                }

                return false; // unreachable
            }();

            auto end = reader.position();

            // Report that we've backtracked.
            context.on(_ev::backtracked{}, begin, end);

            return result;
        }

        template <typename NextParser, typename... Args>
        LEXY_PARSER_FUNC static bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!bp<Context, Reader>::try_parse(context, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::error<Reader, tag>(reader.position());
                context.on(_ev::error{}, err);
                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _look<Needle, End, Error> error = {};
};

/// Looks for the Needle before End.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
constexpr auto lookahead(Needle, End)
{
    static_assert(lexy::is_token_rule<Needle> && lexy::is_token_rule<End>);
    return _look<Needle, End, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

