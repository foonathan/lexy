// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
struct missing_until_condition
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing until condition";
    }
};
} // namespace lexy

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
            if (reader.peek() == Reader::encoding::eof())
                break;

            reader.bump();
        }

        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC void error(const Reader&, typename Reader::iterator);
};

template <typename Condition>
struct _until : atom_base<_until<Condition>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        while (!Condition::matcher::match(reader))
        {
            if (reader.peek() == Reader::encoding::eof())
                return false;

            reader.bump();
        }

        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader& reader, typename Reader::iterator pos)
    {
        return lexy::error<Reader, lexy::missing_until_condition>(pos, reader.cur());
    }

    /// Also accepts EOF as the closing condition.
    LEXY_CONSTEVAL auto or_eof() const
    {
        return _until_eof<Condition>{};
    }
};

/// Matches anything, until Condition matches.
/// Then matches Condition.
template <typename Condition>
LEXY_CONSTEVAL auto until(Condition)
{
    static_assert(lexy::is_pattern<Condition>);
    return _until<Condition>{};
}
} // namespace lexyd

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED

