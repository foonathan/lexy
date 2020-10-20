// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ANY_HPP_INCLUDED
#define LEXY_DSL_ANY_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _any : atom_base<_any>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader& reader)
    {
        while (!reader.eof())
            reader.bump();
        return true;
    }
    template <typename Encoding, typename Iterator>
    LEXY_DSL_FUNC bool match(lexy::_detail::range_reader<Encoding, Iterator, Iterator>& reader)
    {
        reader._make_eof();
        return true;
    }

    template <typename Reader>
    LEXY_DSL_FUNC void error(const Reader&, typename Reader::iterator);
};

/// Matches anything and consumes all remaining characters.
constexpr auto any = _any{};
} // namespace lexyd

#endif // LEXY_DSL_ANY_HPP_INCLUDED
