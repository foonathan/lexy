// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/file.hpp>
#include <rapidjson/reader.h>

bool json_rapid(const lexy::read_file_result<lexy::utf8_encoding>& input)
{
    rapidjson::MemoryStream stream(reinterpret_cast<const char*>(input.data()), input.size());
    rapidjson::BaseReaderHandler<> handler;

    rapidjson::Reader reader;
    return !reader.Parse(stream, handler).IsError();
}

