// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_SIGN_HPP_INCLUDED
#define LEXY_DSL_SIGN_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/value.hpp>

namespace lexy
{
template <typename T>
struct _signed_integer
{
    using return_type = T;

    template <typename Integer>
    constexpr T operator()(int sign, Integer value) const
    {
        return T(sign * value);
    }
};

// A callback that takes a sign and an integer and produces the signed integer.
template <typename T>
constexpr auto signed_integer = _signed_integer<T>{};
} // namespace lexy

namespace lexyd
{
/// Matches a plus sign or nothing, producing +1.
constexpr auto plus_sign = LEXY_LIT("+") >> value_c<+1> | else_ >> value_c<+1>;
/// Matches a minus sign or nothing, producing +1 or -1.
constexpr auto minus_sign = LEXY_LIT("-") >> value_c<-1> | else_ >> value_c<+1>;

/// Matches a plus or minus sign or nothing, producing +1 or -1.
constexpr auto sign
    = LEXY_LIT("+") >> value_c<+1> | LEXY_LIT("-") >> value_c<-1> | else_ >> value_c<+1>;
} // namespace lexyd

#endif // LEXY_DSL_SIGN_HPP_INCLUDED

