// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_BASE_HPP_INCLUDED
#define LEXY_ENGINE_BASE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

#if 0
/// Matches something, i.e. consumes input and returns success or failure.
struct Matcher : engine_matcher_base
{
    /// The error code of the match operation.
    /// A value constructed `error_code` corresponds to success.
    enum class error_code;

    /// Tries to match the input of the reader.
    /// If it works, consumes all matched input and returns `error_code()`.
    /// Otherwise, leaves the reader at the position of the error and returns some other error_code.
    template <typename Reader>
    static error_code match(Reader& reader);
};
#endif

namespace lexy
{
struct engine_matcher_base
{};

/// Whether or not the engine is a matcher.
template <typename Engine>
constexpr bool engine_is_matcher = std::is_base_of_v<engine_matcher_base, Engine>;

/// Whether or not the engine can fail on the given input.
template <typename Engine, typename Reader>
constexpr bool engine_can_fail = true;

/// Whether or not the engine can succeed on the given input.
template <typename Engine, typename Reader>
constexpr bool engine_can_succeed = true;
} // namespace lexy

namespace lexy
{
/// Matches the `Matcher` returning a boolean.
/// Input is only consumed if the match was successful.
template <typename Matcher, typename Reader>
constexpr bool engine_try_match(Reader& reader)
{
    if constexpr (engine_can_fail<Matcher, Reader>)
    {
        auto save = reader;
        if (Matcher::match(reader) == typename Matcher::error_code())
            return true;
        else
        {
            reader = LEXY_MOV(save);
            return false;
        }
    }
    else
    {
        return Matcher::match(reader) == typename Matcher::error_code();
    }
}

/// Matches the `Matcher` consuming nothing.
template <typename Matcher, typename Reader>
constexpr bool engine_peek(Reader reader)
{
    return Matcher::match(reader) == typename Matcher::error_code();
}
} // namespace lexy

#endif // LEXY_ENGINE_BASE_HPP_INCLUDED

