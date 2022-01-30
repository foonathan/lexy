// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

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

