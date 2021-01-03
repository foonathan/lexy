// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>

bool json_baseline(const lexy::buffer<lexy::utf8_encoding>& input)
{
    // Just do something with the input.
    std::size_t sum = 0;
    for (auto c : input)
        sum += c;
    return sum % 11 == 0;
}

