// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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
    template <typename Reader>
    struct bp
    {
        typename Reader::iterator begin;
        typename Reader::iterator end;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock*, Reader reader)
        {
            begin = reader.position();

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

            end = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end);
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            context.on(_ev::backtracked{}, impl.begin, impl.end);
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

