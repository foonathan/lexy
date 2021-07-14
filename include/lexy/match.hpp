// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

namespace lexy::_detail
{
[[deprecated("`<lexy/match.hpp>` is deprecated; include `<lexy/action/match.hpp>`")]] //
static int match_hpp_is_deprecated;
static int trigger_match_hpp_deprecated = match_hpp_is_deprecated;
} // namespace lexy::_detail

#include <lexy/action/match.hpp>

#endif // LEXY_MATCH_HPP_INCLUDED

