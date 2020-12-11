// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_FAILURE_HPP_INCLUDED
#define LEXY_ENGINE_FAILURE_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches nothing, i.e. always fails without consuming anything.
struct engine_failure : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader&)
    {
        return error_code::error;
    }
};

template <typename Reader>
inline constexpr bool engine_can_succeed<engine_failure, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_FAILURE_HPP_INCLUDED

