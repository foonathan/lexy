// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_LITERAL_HPP_INCLUDED
#define LEXY_ENGINE_LITERAL_HPP_INCLUDED

#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches the literal `String::get()`.
template <typename String>
struct engine_literal : engine_matcher_base
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

        if constexpr (string.size() == 1)
        {
            if (reader.peek() != encoding::to_int_type(string[0]))
                return error_code::error;
            reader.bump();

            return error_code();
        }
        else if constexpr (string.size() == 2)
        {
            if (reader.peek() != encoding::to_int_type(string[0]))
                return error_code::error;
            reader.bump();

            if (reader.peek() != encoding::to_int_type(string[1]))
                return error_code::error;
            reader.bump();

            return error_code();
        }
        else if constexpr (string.size() == 3)
        {
            if (reader.peek() != encoding::to_int_type(string[0]))
                return error_code::error;
            reader.bump();

            if (reader.peek() != encoding::to_int_type(string[1]))
                return error_code::error;
            reader.bump();

            if (reader.peek() != encoding::to_int_type(string[2]))
                return error_code::error;
            reader.bump();

            return error_code();
        }
        else if constexpr (string.size() == 4)
        {
            if (reader.peek() != encoding::to_int_type(string[0]))
                return error_code::error;
            reader.bump();

            if (reader.peek() != encoding::to_int_type(string[1]))
                return error_code::error;
            reader.bump();

            if (reader.peek() != encoding::to_int_type(string[2]))
                return error_code::error;
            reader.bump();

            if (reader.peek() != encoding::to_int_type(string[3]))
                return error_code::error;
            reader.bump();

            return error_code();
        }
        else
        {
            for (auto c : string)
            {
                if (reader.peek() != encoding::to_int_type(c))
                    return error_code::error;

                reader.bump();
            }

            return error_code();
        }
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_LITERAL_HPP_INCLUDED

