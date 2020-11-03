// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_NULL_INPUT_HPP_INCLUDED
#define LEXY_INPUT_NULL_INPUT_HPP_INCLUDED

#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
template <typename Encoding = default_encoding>
class null_input
{
public:
    constexpr auto reader() const& noexcept
    {
        class reader_type
        {
        public:
            using encoding         = Encoding;
            using char_type        = typename encoding::char_type;
            using iterator         = const char_type*;
            using canonical_reader = reader_type;

            constexpr bool eof() const noexcept
            {
                return true;
            }

            constexpr auto peek() const noexcept
            {
                return Encoding::eof();
            }

            constexpr void bump() noexcept {}

            constexpr iterator cur() const noexcept
            {
                return nullptr;
            }
        };

        return reader_type();
    }
};

template <typename Encoding = default_encoding>
using null_lexeme = lexeme_for<null_input<Encoding>>;

template <typename Tag, typename Encoding = default_encoding>
using null_error = error_for<null_input<Encoding>, Tag>;

template <typename Production, typename Encoding = default_encoding>
using null_error_context = error_context<Production, null_input<Encoding>>;
} // namespace lexy

#endif // LEXY_INPUT_NULL_INPUT_HPP_INCLUDED

