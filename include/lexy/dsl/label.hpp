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
template <typename Label>
struct _lab : rule_base
{
    static constexpr auto has_matcher = false;

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
    LEXY_CONSTEVAL auto operator()(Rule rule) const
    {
        if constexpr (lexy::is_branch_rule<Rule>)
        {
            auto as_branch = branch(rule);
            return as_branch.condition() >> *this + as_branch.then();
        }
        else
        {
            return *this + rule;
        }
    }
};

/// Matches with the specified label.
template <typename Label>
constexpr auto label = _lab<Label>{};

/// Matches with the specified id.
template <auto Id>
constexpr auto id = _lab<std::integral_constant<int, Id>>{};
} // namespace lexyd

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
template <typename Fn>
struct _mem : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)..., lexy::member<Fn>{});
        }
    };
};

template <typename Fn>
struct _mem_dsl
{
    LEXY_CONSTEVAL _mem_dsl(Fn = {}) {}

    template <typename Rule>
    LEXY_CONSTEVAL auto operator=(Rule rule) const
    {
        using lambda   = std::conditional_t<std::is_default_constructible_v<Fn>, Fn,
                                          lexy::_detail::stateless_lambda<Fn>>;
        using mem_rule = _mem<lambda>;

        if constexpr (lexy::is_branch_rule<Rule>)
        {
            auto as_branch = branch(rule);
            return as_branch.condition() >> mem_rule{} + as_branch.then();
        }
        else
        {
            return mem_rule{} + rule;
        }
    }
};

/// Specifies that the output of the associated rule should be stored in the member pointer. Used
/// with `lexy::as_aggregate`.
template <auto MemPtr>
constexpr auto member = _mem_dsl<lexy::_mem_ptr_fn<MemPtr>>{};

#define LEXY_MEM(Name)                                                                             \
    ::lexyd::_mem_dsl([](auto& obj, auto&& value) { obj.Name = LEXY_FWD(value); })
} // namespace lexyd

#endif // LEXY_DSL_LABEL_HPP_INCLUDED
