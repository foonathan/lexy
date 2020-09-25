// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LABEL_HPP_INCLUDED
#define LEXY_DSL_LABEL_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexy
{
template <typename T, typename = void>
struct label
{};
template <typename T>
struct label<T, decltype(void(T::value))>
{
    LEXY_CONSTEVAL operator decltype(T::value)() const
    {
        return T::value;
    }
};

template <auto Id>
using id = label<std::integral_constant<int, Id>>;
} // namespace lexy

namespace lexyd
{
template <typename Label>
struct _lab : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Reader& reader, Args&&... args) ->
            typename Context::result_type
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., lexy::label<Label>{});
        }
    };
};

/// Matches with the specified label.
template <typename Label>
constexpr auto label = _lab<Label>{};

/// Matches with the specified id.
template <auto Id>
constexpr auto id = _lab<std::integral_constant<int, Id>>{};
} // namespace lexyd

#endif // LEXY_DSL_LABEL_HPP_INCLUDED
