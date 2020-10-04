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
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&... args) ->
            typename Handler::result_type
        {
            return NextParser::parse(handler, reader, LEXY_FWD(args)..., reader.cur());
        }
    };
};

/// Produces an iterator to the current reader position without parsing anything.
constexpr auto position = _pos{};
} // namespace lexyd

#endif // LEXY_DSL_POSITION_HPP_INCLUDED

