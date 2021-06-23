// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/json.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

bool json_pegtl(const lexy::read_file_result<lexy::utf8_encoding>& _input)
{
    using grammar = pegtl::seq<pegtl::json::text, pegtl::eof>;

    pegtl::memory_input input(reinterpret_cast<const char*>(_input.data()), _input.size(), "");
    try
    {
        return pegtl::parse<grammar>(input);
    }
    catch (const pegtl::parse_error&)
    {
        return false;
    }
}

