// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_RETURN_HPP_INCLUDED
#define LEXY_DSL_RETURN_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _ret : rule_base
{
    static constexpr auto has_matcher = false;

    // We unconditionally jump to the final parser.
    template <typename NextParser>
    using parser = lexy::final_parser;
};

/// Finishes parsing a production without considering subsequent rules.
constexpr auto return_ = _ret{};
} // namespace lexyd

#endif // LEXY_DSL_RETURN_HPP_INCLUDED

