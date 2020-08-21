// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_POSITION_HPP_INCLUDED
#define LEXY_DSL_POSITION_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _pos : rule_base
{
    static constexpr auto has_matcher = false;

    template <typename NextParser>
    struct parser
    {
        template <typename Context, typename Input, typename... Args>
        LEXY_DSL_FUNC auto parse(Context& context, Input& input, Args&&... args) ->
            typename Context::result_type
        {
            return NextParser::parse(context, input, LEXY_FWD(args)..., input.cur());
        }
    };
};

/// Produces an iterator to the current input position without parsing anything.
constexpr auto position = _pos{};
} // namespace lexyd

#endif // LEXY_DSL_POSITION_HPP_INCLUDED

