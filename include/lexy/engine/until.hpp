// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_UNTIL_HPP_INCLUDED
#define LEXY_ENGINE_UNTIL_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches everything until and including Condition.
template <typename Condition>
struct engine_until : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition>);

    using error_code = typename Condition::error_code;

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_try_match<Condition>(reader))
        {
            if (reader.peek() == Reader::encoding::eof())
            {
                // This match fails but gives us an appropriate error code.
                return Condition::match(reader);
            }

            reader.bump();
        }

        return error_code();
    }
};
} // namespace lexy

namespace lexy
{
/// Matches everything until and including Condition or EOF.
template <typename Condition>
struct engine_until_eof : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Condition>);

    enum class error_code
    {
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        while (!engine_try_match<Condition>(reader))
        {
            if (reader.peek() == Reader::encoding::eof())
                break;

            reader.bump();
        }

        return error_code();
    }
};

template <typename Condition, typename Reader>
inline constexpr bool engine_can_fail<engine_until_eof<Condition>, Reader> = false;
} // namespace lexy

#endif // LEXY_ENGINE_UNTIL_HPP_INCLUDED

