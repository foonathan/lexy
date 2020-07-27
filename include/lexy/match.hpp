// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_MATCH_HPP_INCLUDED
#define LEXY_MATCH_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/lexeme.hpp>

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

template <typename Input, typename Pattern, typename = std::enable_if_t<is_dsl<Pattern>>>
LEXY_FORCE_INLINE constexpr auto pattern_match(Input&& input, Pattern)
{
    static_assert(is_pattern<Pattern>);
    using matcher     = typename Pattern::matcher;
    using result_type = pattern_match_result<std::decay_t<Input>, matcher::max_capture_count>;

    result_type result;
    result._id = 0;

    auto begin = input.cur();
    if (!matcher::match(result, input))
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

#endif // LEXY_MATCH_HPP_INCLUDED

