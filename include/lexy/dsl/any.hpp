// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ANY_HPP_INCLUDED
#define LEXY_DSL_ANY_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
struct _any : token_base<_any, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr std::true_type try_parse(Reader reader)
        {
            while (reader.peek() != Reader::encoding::eof())
                reader.bump();
            end = reader.position();
            return {};
        }
    };
};

/// Matches anything and consumes all remaining characters.
constexpr auto any = _any{};
} // namespace lexyd

#endif // LEXY_DSL_ANY_HPP_INCLUDED

