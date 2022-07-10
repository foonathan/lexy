// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ANY_HPP_INCLUDED
#define LEXY_DSL_ANY_HPP_INCLUDED

#include <lexy/_detail/swar.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
struct _any : token_base<_any, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;
            if constexpr (lexy::_detail::is_swar_reader<Reader>)
            {
                constexpr auto eof = lexy::_detail::swar_fill(encoding::eof());
                while (reader.peek_swar() != eof)
                    reader.bump_swar();

                // We've reached the swar that represents EOF, go back to the first char that is
                // EOF. This works as for a swar reader, char_type == int_type.
                auto ptr = reader.position();
                while (ptr[-1] == encoding::eof())
                    --ptr;
                reader.set_position(ptr);
            }
            else
            {
                while (reader.peek() != encoding::eof())
                    reader.bump();
            }

            end = reader.position();
            return {};
        }
    };
};

/// Matches anything and consumes all remaining characters.
constexpr auto any = _any{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_any> = lexy::any_token_kind;
}

#endif // LEXY_DSL_ANY_HPP_INCLUDED

