// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_PUNCTUATOR_HPP_INCLUDED
#define LEXY_DSL_PUNCTUATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
template <typename String>
struct _punct : _lit<String>
{
    template <typename Whitespace>
    LEXY_CONSTEVAL auto operator()(Whitespace ws) const
    {
        static_assert(lexy::is_pattern<Whitespace>, "whitespace must be a pattern");
        return ws + _lit<String>{};
    }
};

#if LEXY_HAS_NTTP
/// Matches the punctuator.
template <lexy::_detail::string_literal Str>
constexpr auto punct = _punct<lexy::_detail::type_string<Str>>;
#endif

#define LEXY_PUNCT(Str)                                                                            \
    ::lexyd::_punct<LEXY_NTTP_STRING(Str)> {}

constexpr auto period    = LEXY_PUNCT(".");
constexpr auto comma     = LEXY_PUNCT(",");
constexpr auto colon     = LEXY_PUNCT(":");
constexpr auto semicolon = LEXY_PUNCT(";");

constexpr auto hyphen     = LEXY_PUNCT("-");
constexpr auto slash      = LEXY_PUNCT("/");
constexpr auto backslash  = LEXY_PUNCT("\\");
constexpr auto apostrophe = LEXY_PUNCT("'");

constexpr auto hash_sign   = LEXY_PUNCT("#");
constexpr auto dollar_sign = LEXY_PUNCT("$");
constexpr auto at_sign     = LEXY_PUNCT("@");
} // namespace lexyd

#endif // LEXY_DSL_PUNCTUATOR_HPP_INCLUDED

