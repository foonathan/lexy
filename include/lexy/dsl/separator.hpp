// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/recover.hpp>
#include <lexy/error.hpp>

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
    using trailing_rule = decltype(lexyd::if_(Branch{} >> lexyd::try_(lexyd::error<Tag>)));

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&                  context, Reader&,
                                                typename Reader::iterator sep_begin,
                                                typename Reader::iterator sep_end)
    {
        auto err = lexy::error<Reader, Tag>(sep_begin, sep_end);
        context.on(_ev::error{}, err);
    }

    //=== dsl ===//
    template <typename NewTag>
    static constexpr _sep<Branch, NewTag> trailing_error = {};
};

/// Defines a separator for a list.
template <typename Branch>
constexpr auto sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _sep<Branch, lexy::unexpected_trailing_separator>{};
}

template <typename Branch>
struct _tsep
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{}));

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&, Reader&, typename Reader::iterator,
                                                typename Reader::iterator)
    {}
};

/// Defines a separator for a list that can be trailing.
template <typename Branch>
constexpr auto trailing_sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _tsep<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED

