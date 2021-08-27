// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_EOF_HPP_INCLUDED
#define LEXY_ENGINE_EOF_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches EOF.
struct engine_eof : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        return reader.peek() == Reader::encoding::eof() ? error_code() : error_code::error;
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_EOF_HPP_INCLUDED

