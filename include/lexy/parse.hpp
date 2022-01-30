// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_PARSE_HPP_INCLUDED
#define LEXY_PARSE_HPP_INCLUDED

namespace lexy::_detail
{
[[deprecated("`<lexy/parse.hpp>` is deprecated; include `<lexy/action/parse.hpp>`")]] //
static int parse_hpp_is_deprecated;
static int trigger_parse_hpp_deprecated = parse_hpp_is_deprecated;
} // namespace lexy::_detail

#include <lexy/action/parse.hpp>

#endif // LEXY_PARSE_HPP_INCLUDED

