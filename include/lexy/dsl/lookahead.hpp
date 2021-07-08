// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/engine/find.hpp>
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
struct _look : rule_base
{
    static constexpr auto is_branch = true;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto try_parse(Context& context, Reader& reader, Args&&... args)
            -> lexy::rule_try_parse_result
        {
            using engine = lexy::engine_find_before<Needle, End>;
            if (!lexy::engine_peek<engine>(reader))
                return lexy::rule_try_parse_result::backtracked;

            return NextParser::parse(context, reader, LEXY_FWD(args)...)
                       ? lexy::rule_try_parse_result::ok
                       : lexy::rule_try_parse_result::canceled;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using engine = lexy::engine_find_before<Needle, End>;
            if (!lexy::engine_peek<engine>(reader))
            {
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::make_error<Reader, tag>(reader.cur());
                context.error(err);
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
    return _look<typename Needle::token_engine, typename End::token_engine, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

