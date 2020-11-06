// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexy
{
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
template <typename Needle, typename End>
struct _look : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool _match(Reader& reader)
        {
            while (true)
            {
                if (Needle::matcher::match(reader))
                    // We found the lookahead condition.
                    break;
                else if (End::matcher::match(reader))
                    // We haven't found the lookahead condition and need to cancel.
                    return false;
                else if (reader.eof())
                    // Dito.
                    return false;
                else
                    reader.bump();
            }

            return true;
        }
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            // We're matching on a copy of the reader so we don't change the input.
            auto copy = reader;
            return _match(copy);
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            if (auto copy = reader; matcher::_match(copy))
                return NextParser::parse(handler, reader, LEXY_FWD(args)...);
            else
            {
                auto e
                    = lexy::make_error<Reader, lexy::lookahead_failure>(reader.cur(), copy.cur());
                return LEXY_MOV(handler).error(reader, e);
            }
        }
    };

    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator[](Whitespace ws) const
    {
        return whitespaced(*this, ws);
    }
};

/// Looks for the Needle pattern before End pattern.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
LEXY_CONSTEVAL auto lookahead(Needle, End)
{
    static_assert(lexy::is_pattern<Needle> && lexy::is_pattern<End>);
    return _look<Needle, End>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

