// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename Condition>
struct _until_eof : atom_base<_until_eof<Condition>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        while (!Condition::matcher::match(reader))
        {
            if (reader.eof())
                break;

            reader.bump();
        }

        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC void error(const Reader&, typename Reader::iterator);
};

template <typename Condition>
struct _until : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader& reader)
        {
            auto save = reader;

            while (!Condition::matcher::match(reader))
            {
                if (reader.eof())
                {
                    reader = LEXY_MOV(save);
                    return false;
                }

                reader.bump();
            }

            return true;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            while (!Condition::matcher::match(reader))
            {
                if (reader.eof())
                {
                    // We're trying to parse the condition now.
                    // This will fail, but it will create an appropriate error.
                    return Condition::template parser<NextParser>::parse(handler, reader,
                                                                         LEXY_FWD(args)...);
                }

                reader.bump();
            }

            return NextParser::parse(handler, reader, LEXY_FWD(args)...);
        }
    };

    /// Also accepts EOF as the closing condition.
    LEXY_CONSTEVAL auto or_eof() const
    {
        return _until_eof<Condition>{};
    }
};

/// Matches anything until Condition matches.
/// Then matches Condition.
template <typename Condition>
LEXY_CONSTEVAL auto until(Condition)
{
    static_assert(lexy::is_pattern<Condition>);
    return _until<Condition>{};
}
} // namespace lexyd

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED

