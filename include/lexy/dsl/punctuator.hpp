// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PUNCTUATOR_HPP_INCLUDED
#define LEXY_DSL_PUNCTUATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>

namespace lexyd
{
constexpr auto period    = LEXY_LIT(".");
constexpr auto comma     = LEXY_LIT(",");
constexpr auto colon     = LEXY_LIT(":");
constexpr auto semicolon = LEXY_LIT(";");

constexpr auto hyphen     = LEXY_LIT("-");
constexpr auto slash      = LEXY_LIT("/");
constexpr auto backslash  = LEXY_LIT("\\");
constexpr auto apostrophe = LEXY_LIT("'");

constexpr auto hash_sign   = LEXY_LIT("#");
constexpr auto dollar_sign = LEXY_LIT("$");
constexpr auto at_sign     = LEXY_LIT("@");
} // namespace lexyd

#endif // LEXY_DSL_PUNCTUATOR_HPP_INCLUDED

