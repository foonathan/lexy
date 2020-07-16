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

namespace lexy::_detail
{
template <typename I>
constexpr auto range_size(I begin, I end) -> decltype(std::size_t(end - begin))
{
    return std::size_t(end - begin);
}
template <typename I, typename I2> // always worse match because two different params
constexpr auto range_size(I begin, I2 end)
{
    std::size_t result = 0;
    while (begin++ != end)
        ++result;
    return result;
}
} // namespace lexy::_detail

namespace lexy
{
template <typename Input>
using input_iterator_type = typename std::decay_t<Input>::iterator;

template <typename Input, typename CharT>
constexpr bool char_type_compatible_with_input
    = (std::is_same_v<CharT, typename Input::char_type>)
      || Input::encoding::template is_secondary_char_type<CharT>;
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED

