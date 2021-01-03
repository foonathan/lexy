// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_ANY_HPP_INCLUDED
#define LEXY_ENGINE_ANY_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches the entire input.
struct engine_any : engine_matcher_base
{
    enum class error_code
    {
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!reader.eof())
            reader.bump();
        return error_code();
    }
};

template <typename Reader>
inline constexpr bool engine_can_fail<engine_any, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_ANY_HPP_INCLUDED

