// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_FIND_HPP_INCLUDED
#define LEXY_ENGINE_FIND_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches everything until and excluding Condition.
template <typename Condition>
struct engine_find : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition>);

    enum class error_code
    {
        not_found = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_peek<Condition>(reader))
        {
            if (reader.peek() == Reader::encoding::eof())
                return error_code::not_found;
            else
                reader.bump();
        }

        return error_code();
    }
};

/// Matches everything until and excluding Condition.
/// Fails when it matches Limit.
template <typename Condition, typename Limit>
struct engine_find_before : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition> && lexy::engine_is_matcher<Limit>);

    enum class error_code
    {
        not_found_eof = 1,
        not_found_limit,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_peek<Condition>(reader))
        {
            if (reader.peek() == Reader::encoding::eof())
                return error_code::not_found_eof;
            else if (engine_peek<Limit>(reader))
                return error_code::not_found_limit;
            else
                reader.bump();
        }

        return error_code();
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_FIND_HPP_INCLUDED

