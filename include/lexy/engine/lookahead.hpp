// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_LOOKAHEAD_HPP_INCLUDED
#define LEXY_ENGINE_LOOKAHEAD_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Searches for `Needle`.
/// Succeeds, if `Needle` is found before `End`.
/// Does not consume anything.
template <typename Needle, typename End>
struct engine_lookahead : engine_matcher_base
{
    static_assert(lexy::engine_is_matcher<Needle> && lexy::engine_is_matcher<End>);

    enum class error_code
    {
        not_found_end = 1,
        not_found_eof,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& _reader)
    {
        auto reader = _reader;
        while (true)
        {
            if (auto ec = Needle::match(reader); ec == typename Needle::error_code())
                // We have found the needle.
                break;
            else if (reader.eof())
                // We have reached the end of the input without finding the needle.
                return error_code::not_found_eof;
            else if (auto ec = End::match(reader); ec == typename End::error_code())
                // We have found the end condition before founding needle.
                return error_code::not_found_end;
            else
                // Try again on the next input.
                reader.bump();
        }

        return error_code();
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_LOOKAHEAD_HPP_INCLUDED

