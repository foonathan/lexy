// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SIGN_HPP_INCLUDED
#define LEXY_DSL_SIGN_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/literal.hpp>

namespace lexy
{
template <int I>
struct _sign
{
    constexpr operator int() const
    {
        return I;
    }
};

struct plus_sign : _sign<+1>
{};
struct minus_sign : _sign<-1>
{};
} // namespace lexy

namespace lexyd
{
template <typename Sign>
struct _sign : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., Sign{});
        }
    };
};

/// Matches a plus sign or nothing, producing +1.
constexpr auto plus_sign = if_(LEXY_LIT("+") >> _sign<lexy::plus_sign>{});
/// Matches a minus sign or nothing, producing +1 or -1.
constexpr auto minus_sign = if_(LEXY_LIT("-") >> _sign<lexy::minus_sign>{});

/// Matches a plus or minus sign or nothing, producing +1 or -1.
constexpr auto sign
    = if_(LEXY_LIT("+") >> _sign<lexy::plus_sign>{} | LEXY_LIT("-") >> _sign<lexy::minus_sign>{});
} // namespace lexyd

#endif // LEXY_DSL_SIGN_HPP_INCLUDED

