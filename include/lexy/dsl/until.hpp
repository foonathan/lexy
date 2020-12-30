// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/engine/until.hpp>

namespace lexyd
{
template <typename Condition>
struct _until_eof : token_base<_until_eof<Condition>>
{
    using token_engine = lexy::engine_until_eof<typename Condition::token_engine>;
};

template <typename Condition>
struct _until : token_base<_until<Condition>>
{
    using token_engine = lexy::engine_until<typename Condition::token_engine>;

    template <typename Context, typename Reader>
    static constexpr auto token_error(Context& context, const Reader& reader,
                                      typename token_engine::error_code ec,
                                      typename Reader::iterator)
    {
        // We don't pass the passed position, as this would be the beginning of until.
        // Instead we always use the current reader position (i.e. EOF) as that's where the
        // condition is missing.
        return Condition::token_error(context, reader, ec, reader.cur());
    }

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
    static_assert(lexy::is_token<Condition>);
    return _until<Condition>{};
}
} // namespace lexyd

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED

