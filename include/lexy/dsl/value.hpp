// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_VALUE_HPP_INCLUDED
#define LEXY_DSL_VALUE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <auto Value>
struct _valc : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            return NextParser::parse(context, input, LEXY_FWD(args)..., Value);
        }
    };
};

/// Produces the specified value without parsing anything.
template <auto Value>
constexpr auto value_c = _valc<Value>{};
} // namespace lexyd

namespace lexyd
{
template <auto F>
struct _valf : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            return NextParser::parse(context, input, LEXY_FWD(args)..., F());
        }
    };
};

/// Produces the value returned by the function without parsing anything.
template <auto F>
constexpr auto value_f = _valf<F>{};
} // namespace lexyd

namespace lexyd
{
template <typename T>
struct _valt : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            return NextParser::parse(context, input, LEXY_FWD(args)..., T());
        }
    };
};

/// Produces a default constructed value of the specified type without parsing anything.
template <typename T>
constexpr auto value_t = _valt<T>{};
} // namespace lexyd

#endif // LEXY_DSL_VALUE_HPP_INCLUDED

