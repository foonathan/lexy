// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_BASE_HPP_INCLUDED
#define LEXY_PATTERN_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
struct _pattern_base
{};
template <typename Pattern>
struct pattern_base : _pattern_base
{
    using pattern = Pattern;
};
} // namespace lexyd

#if 0
class Pattern : pattern_base
{
    static constexpr std::size_t max_capture_count;

    // If matches, consumes characters from input, update context, and return true.
    // If it doesn't match, leave input as-is and return false.
    template <typename Context, typename Input>
    LEXY_DSL_FUNC bool match(Context& context, Input& input);
};
#endif

namespace lexy
{
template <typename T>
constexpr bool is_pattern = std::is_base_of_v<dsl::_pattern_base, T>;

template <typename... T>
using _enable_pattern = std::enable_if_t<((is_pattern<T> || is_atom<T>)&&...)>;
} // namespace lexy

namespace lexy
{
template <typename Input, std::size_t N>
struct pattern_match_result
{
    int           _id;
    int           _capture_count;
    lexeme<Input> _matched;
    lexeme<Input> _captures[N];

    constexpr pattern_match_result() noexcept : _id(-1), _capture_count(0), _matched(), _captures()
    {}

    /// Whether or not we matched.
    constexpr explicit operator bool() const noexcept
    {
        return _id >= 0;
    }

    /// The id of our match.
    constexpr int id() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _id;
    }

    // What matched.
    constexpr lexeme<Input> match() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _matched;
    }

    /// How many captures were populated.
    constexpr std::size_t capture_count() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return std::size_t(_capture_count);
    }

    /// The n-th capture.
    constexpr lexeme<Input> capture(std::size_t i) const noexcept
    {
        LEXY_PRECONDITION(*this);
        LEXY_PRECONDITION(i < capture_count());
        return _captures[i];
    }
};
template <typename Input>
struct pattern_match_result<Input, 0>
{
    int           _id;
    lexeme<Input> _matched;

    constexpr pattern_match_result() noexcept : _id(-1), _matched() {}

    /// Whether or not we matched.
    constexpr explicit operator bool() const noexcept
    {
        return _id >= 0;
    }

    /// The id of our match.
    constexpr int id() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _id;
    }

    // What matched.
    constexpr lexeme<Input> match() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _matched;
    }
};

template <typename Input, typename Pattern,
          typename = std::enable_if_t<is_atom<Pattern> || is_pattern<Pattern>>>
LEXY_FORCE_INLINE constexpr auto pattern_match(Input&& input, Pattern)
{
    using pattern     = typename Pattern::pattern;
    using result_type = pattern_match_result<std::decay_t<Input>, pattern::max_capture_count>;

    result_type result;
    result._id = 0;

    auto begin = input.cur();
    if (!pattern::match(result, input))
    {
        result._id = -1;
        return result;
    }
    auto end = input.cur();

    result._matched = {begin, end};
    return result;
}

template <typename Input, typename PatternHolder>
constexpr auto pattern_match(Input&& input, PatternHolder holder)
    -> decltype(pattern_match(input, holder()))
{
    using pattern = decltype(holder());
    return pattern_match(input, pattern{});
}
} // namespace lexy

#endif // LEXY_PATTERN_BASE_HPP_INCLUDED
