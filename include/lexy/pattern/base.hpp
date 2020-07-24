// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PATTERN_BASE_HPP_INCLUDED
#define LEXY_PATTERN_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/atom/base.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

#define LEXY_PATTERN_FUNC LEXY_FORCE_INLINE static constexpr

// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct pattern_base
{};
} // namespace lexyd

#if 0
class Pattern : pattern_base
{
    static constexpr std::size_t max_capture_count;

    // If matches, consumes characters from input, update context, and return true.
    // If it doesn't match, leave input as-is and return false.
    template <typename Context, typename Input>
    LEXY_PATTERN_FUNC bool match(Context& context, Input& input);
};
#endif

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_pattern = std::is_base_of_v<dsl::pattern_base, T>;

template <typename... T>
using _enable_pattern = std::enable_if_t<((is_pattern<T> || is_atom<T>)&&...)>;
} // namespace lexy

namespace lexyd
{
template <typename Atom>
struct _atomp : pattern_base
{
    static constexpr auto max_capture_count = 0;

    template <typename Context, typename Input>
    LEXY_PATTERN_FUNC bool match(Context&, Input& input)
    {
        auto reset = input;
        if (Atom::match(input))
            return true;

        input = LEXY_MOV(reset);
        return false;
    }
};

/// A pattern that matches an atom.
template <typename AtomOrPattern>
LEXY_CONSTEVAL auto pattern(AtomOrPattern)
{
    if constexpr (lexy::is_atom<AtomOrPattern>)
        return _atomp<AtomOrPattern>{};
    else
    {
        static_assert(lexy::is_pattern<AtomOrPattern>);
        return AtomOrPattern{};
    }
}
} // namespace lexyd

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
LEXY_FORCE_INLINE constexpr auto pattern_match(Input&& input, Pattern p)
{
    using pattern     = decltype(dsl::pattern(p));
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

