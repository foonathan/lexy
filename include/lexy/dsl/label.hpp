// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_LABEL_HPP_INCLUDED
#define LEXY_DSL_LABEL_HPP_INCLUDED

#include <lexy/_detail/stateless_lambda.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

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
template <typename Label, typename Rule>
struct _labr;

template <typename Label>
struct _lab : rule_base
{
    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)..., lexy::label<Label>{});
        }
    };

    template <typename Rule>
    LEXY_CONSTEVAL auto operator()(Rule) const
    {
        return _labr<Label, Rule>{};
    }
};

template <typename Label, typename Rule>
struct _labr : rule_base
{
    static constexpr auto is_branch = lexy::is_branch<Rule>;

    template <typename Reader>
    struct branch_matcher
    {
        lexy::branch_matcher<Rule, Reader> _impl;

        static constexpr auto is_unconditional = decltype(_impl)::is_unconditional;

        constexpr bool match(Reader& reader)
        {
            return _impl.match(reader);
        }

        template <typename NextParser, typename Handler, typename... Args>
        constexpr auto parse(Handler& handler, Reader& reader, Args&&... args)
        {
            return _impl.template parse<NextParser>(handler, reader, LEXY_FWD(args)...,
                                                    lexy::label<Label>{});
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            return Rule::template parser<NextParser>::parse(handler, reader, LEXY_FWD(args)...,
                                                            lexy::label<Label>{});
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
