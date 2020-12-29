// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_MEMBER_HPP_INCLUDED
#define LEXY_DSL_MEMBER_HPP_INCLUDED

#include <lexy/_detail/stateless_lambda.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexy
{
template <auto Ptr>
struct _mem_ptr_fn
{
    template <typename Object, typename Value>
    constexpr void operator()(Object& object, Value&& value) const
    {
        object.*Ptr = LEXY_FWD(value);
    }
};

template <typename Fn>
struct member
{};

template <auto Ptr>
using make_member_ptr = member<_mem_ptr_fn<Ptr>>;
} // namespace lexy

namespace lexyd
{
template <typename Fn, typename Rule>
struct _mem : rule_base
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
                                                    lexy::member<Fn>{});
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            return lexy::rule_parser<Rule, NextParser>::parse(handler, reader, LEXY_FWD(args)...,
                                                              lexy::member<Fn>{});
        }
    };
};

template <typename Fn>
struct _mem_dsl
{
    LEXY_CONSTEVAL _mem_dsl(Fn = {}) {}

    template <typename Rule>
    LEXY_CONSTEVAL auto operator=(Rule) const
    {
        using lambda = std::conditional_t<std::is_default_constructible_v<Fn>, Fn,
                                          lexy::_detail::stateless_lambda<Fn>>;
        return _mem<lambda, Rule>{};
    }
};

/// Specifies that the output of the associated rule should be stored in the member pointer. Used
/// with `lexy::as_aggregate`.
template <auto MemPtr>
constexpr auto member = _mem_dsl<lexy::_mem_ptr_fn<MemPtr>>{};

#define LEXY_MEM(Name)                                                                             \
    ::lexyd::_mem_dsl([](auto& obj, auto&& value) { obj.Name = LEXY_FWD(value); })
} // namespace lexyd

#endif // LEXY_DSL_MEMBER_HPP_INCLUDED

