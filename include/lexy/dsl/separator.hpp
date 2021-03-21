// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/error.hpp>

#ifdef LEXY_IGNORE_DEPRECATED_SEP
#    define LEXY_DEPRECATED_SEP
#else
#    define LEXY_DEPRECATED_SEP                                                                    \
        [[deprecated("no_trailing_sep() has been deprecated; use sep() instead")]]
#endif

namespace lexy
{
struct unexpected_trailing_separator
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected trailing separator";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Branch, typename Tag>
struct _sep
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{} >> lexyd::error<Tag>));

    template <typename Context, typename Reader>
    LEXY_DSL_FUNC void report_trailing_error(Context&                  context, Reader&,
                                             typename Reader::iterator sep_pos)
    {
        auto err = lexy::make_error<Reader, Tag>(sep_pos);
        context.error(err);
    }

    //=== dsl ===//
    template <typename NewTag>
    static constexpr _sep<Branch, NewTag> trailing_error = {};
};

/// Defines a separator for a list.
template <typename Branch>
LEXY_CONSTEVAL auto sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _sep<Branch, lexy::unexpected_trailing_separator>{};
}

template <typename Branch>
struct _tsep
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{}));

    template <typename Context, typename Reader>
    LEXY_DSL_FUNC void report_trailing_error(Context&, Reader&, typename Reader::iterator)
    {}
};

/// Defines a separator for a list that can be trailing.
template <typename Branch>
LEXY_CONSTEVAL auto trailing_sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _tsep<Branch>{};
}

template <typename Branch>
LEXY_DEPRECATED_SEP LEXY_CONSTEVAL auto no_trailing_sep(Branch)
{
    static_assert(lexy::is_branch<Branch>);
    return _sep<Branch, lexy::unexpected_trailing_separator>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED

