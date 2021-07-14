// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

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

