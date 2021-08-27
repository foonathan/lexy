// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_MINUS_HPP_INCLUDED
#define LEXY_ENGINE_MINUS_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches `Matcher` but only if `Except` does not match.
template <typename Matcher, typename Except>
struct engine_minus : lexy::engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Matcher> && lexy::engine_is_matcher<Except>);

    enum class error_code
    {
        minus_failure = 1,
    };

    static constexpr error_code error_from_matcher(typename Matcher::error_code ec)
    {
        LEXY_PRECONDITION(ec != typename Matcher::error_code());
        return error_code(int(ec) + 1);
    }
    static constexpr auto error_to_matcher(error_code ec)
    {
        LEXY_PRECONDITION(int(ec) > 1);
        return typename Matcher::error_code(int(ec) - 1);
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        auto save = reader;

        // First match on the original input.
        if (auto ec = Matcher::match(reader); ec != typename Matcher::error_code())
            return error_from_matcher(ec);

        // Then check whether Except matches on the same input.
        if (auto partial = lexy::partial_reader(save, reader.cur());
            lexy::engine_try_match<Except>(partial) && partial.peek() == Reader::encoding::eof())
            // They did, so we don't match.
            return error_code::minus_failure;

        return error_code();
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code ec)
    {
        if (ec == error_code::minus_failure)
            // We've already consumed the input.
            return true;
        else
            return Matcher::recover(reader, error_to_matcher(ec));
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_MINUS_HPP_INCLUDED

