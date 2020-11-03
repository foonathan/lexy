// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_NOT_HPP_INCLUDED
#define LEXY_DSL_NOT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
struct unexpected
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Pattern>
struct _not : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            return !Pattern::matcher::match(reader);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (auto pos = reader.cur(); Pattern::matcher::match(reader))
            {
                auto e = lexy::make_error<Reader, lexy::unexpected>(pos, reader.cur());
                return LEXY_MOV(handler).error(reader, e);
            }
            else
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };
};

/// Only matches if the pattern didn't match.
/// If this fails, it still has consumed the pattern.
/// It is designed for something like `opt(!pattern >> then)`,
/// which matches the pattern if we don't match then.
template <typename Pattern, typename = std::enable_if_t<lexy::is_rule<Pattern>>>
LEXY_CONSTEVAL auto operator!(Pattern)
{
    static_assert(lexy::is_pattern<Pattern>);
    return _not<Pattern>{};
}
} // namespace lexyd

#endif // LEXY_DSL_NOT_HPP_INCLUDED

