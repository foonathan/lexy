// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>
#include <lexy/validate.hpp>

#define LEXY_TEST
#include "../../examples/json.cpp"

bool json_lexy(const lexy::buffer<lexy::utf8_encoding>& input)
{
    return lexy::validate<grammar::json>(input, lexy::noop).has_value();
}

