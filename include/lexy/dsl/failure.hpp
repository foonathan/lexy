// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_FAILURE_HPP_INCLUDED
#define LEXY_DSL_FAILURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
template <typename Tag>
struct failure
{
    template <typename Reader>
    class error
    {
    public:
        constexpr explicit error(typename Reader::iterator pos) noexcept : _pos(pos) {}

        constexpr auto position() const noexcept
        {
            return _pos;
        }

    private:
        typename Reader::iterator _pos;
    };
};
} // namespace lexy

namespace lexyd
{
template <typename Tag>
struct _fail : atom_base<_fail<Tag>>
{
    template <typename Reader>
    LEXY_DSL_FUNC bool match(Reader&)
    {
        return false;
    }

    template <typename Reader>
    LEXY_DSL_FUNC auto error(const Reader&, typename Reader::iterator pos)
    {
        return typename lexy::failure<Tag>::template error<Reader>(pos);
    }
};

/// Matches nothing, produces a failure with the given tag.
template <typename Tag>
constexpr auto failure = _fail<Tag>{};
} // namespace lexyd

#endif // LEXY_DSL_FAILURE_HPP_INCLUDED
