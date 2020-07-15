// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_RANGE_INPUT_HPP_INCLUDED
#define LEXY_INPUT_RANGE_INPUT_HPP_INCLUDED

#include <lexy/input/base.hpp>

namespace lexy
{
template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class range_input
{
public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    using iterator = Iterator;

    //=== constructors ===//
    constexpr range_input() noexcept : _cur(), _end() {}

    constexpr range_input(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end) {}

    //=== input functions ===//
    constexpr auto peek() const noexcept
    {
        if (_cur == _end)
            return Encoding::eof();
        else
            return Encoding::to_int_type(*_cur);
    }

    constexpr void bump() noexcept
    {
        ++_cur;
    }

    constexpr iterator cur() const noexcept
    {
        return _cur;
    }

private:
    Iterator                   _cur;
    LEXY_EMPTY_MEMBER Sentinel _end;
};

template <typename Iterator, typename Sentinel>
range_input(Iterator begin, Sentinel end)
    -> range_input<deduce_encoding<std::decay_t<decltype(*begin)>>, Iterator, Sentinel>;
} // namespace lexy

#endif // LEXY_INPUT_RANGE_INPUT_HPP_INCLUDED

