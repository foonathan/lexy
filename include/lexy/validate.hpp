// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_VALIDATE_HPP_INCLUDED
#define LEXY_VALIDATE_HPP_INCLUDED

namespace lexy::_detail
{
[[deprecated("`<lexy/validate.hpp>` is deprecated; include `<lexy/action/validate.hpp>`")]] //
static int validate_hpp_is_deprecated;
static int trigger_validate_hpp_deprecated = validate_hpp_is_deprecated;
} // namespace lexy::_detail

#include <lexy/action/validate.hpp>

#endif // LEXY_VALIDATE_HPP_INCLUDED

