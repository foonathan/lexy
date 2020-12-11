// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED
#define LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches one of the specified characters (code units) in `String::get()`.
template <typename String>
struct engine_char_set : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        using encoding        = typename Reader::encoding;
        constexpr auto string = String::get();

        for (auto c : string)
        {
            if (reader.peek() == encoding::to_int_type(c))
            {
                reader.bump();
                return error_code();
            }
        }

        return error_code::error;
    }
};

/// Matches the inclusive range of characters (code units).
template <auto Min, auto Max>
struct engine_char_range : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        using encoding = typename Reader::encoding;

        auto cur = reader.peek();
        if (encoding::to_int_type(Min) <= cur && cur <= encoding::to_int_type(Max))
        {
            reader.bump();
            return error_code();
        }
        else
            return error_code::error;
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

