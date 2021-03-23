// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
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

    /// Tries to recover after an error (optional).
    /// If possible, advances the input to the recovered position and returns true.
    /// If not possible, keeps input at the error position and returns false.
    template <typename Reader>
    static bool recover(Reader& reader, error_code ec);
};

/// Parses something, i.e. consumes and input and returns a result or error.
struct Parser : engine_parser_base
{
    /// The error code of the match operation.
    /// A value constructed `error_code` corresponds to success.
    enum class error_code;

    /// Tries to match the input of the reader.
    /// If it works, consumes all matched input and returns the result leaving error code unchanged.
    /// Otherwise, leaves the reader at the position of the error, sets the error and returns some partial result.
    template <typename Reader>
    static auto parse(error_code& ec, Reader& reader);

    /// Same as for Matcher.
    template <typename Reader>
    static bool recover(Reader& reader, error_code ec);
};
#endif

namespace lexy
{
struct engine_matcher_base
{
    template <typename Reader, typename EC>
    static bool recover(Reader&, EC)
    {
        return false;
    }
};
struct engine_parser_base
{
    template <typename Reader, typename EC>
    static bool recover(Reader&, EC)
    {
        return false;
    }
};

/// Whether or not the engine is a matcher.
template <typename Engine>
constexpr bool engine_is_matcher = std::is_base_of_v<engine_matcher_base, Engine>;
/// Whether or not the engine is a parser.
template <typename Engine>
constexpr bool engine_is_parser = std::is_base_of_v<engine_parser_base, Engine>;

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

