// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>

#if 0
/// Inputs are non-owning, cheaply copyable types.
class Input
{
public:
    /// The encoding the input uses.
    using encoding = XXX_encoding;
    using char_type = typename encoding::char_type;

    /// An iterator of char_type, not int_type.
    using iterator = ForwardIterator;

    /// If the input is at eof, returns Encoding::eof().
    /// Otherwise, returns Encoding::to_int_type(/* current character */).
    typename Encoding::int_type peek() const;

    /// Advances to the next character in the input.
    void bump();

    /// Returns an iterator to the current character.
    /// The following code must produce a valid range:
    /// ```
    /// auto begin = input.cur();
    /// input.bump();
    /// ... // more bumps
    /// auto end = input.cur();
    /// ```
    iterator cur() const;
};
#endif

namespace lexy
{
template <typename Encoding, typename CharT>
using _require_secondary_char_type
    = std::enable_if_t<Encoding::template is_secondary_char_type<CharT>>;
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED

