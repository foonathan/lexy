// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_LEXEME_HPP_INCLUDED
#define LEXY_LEXEME_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>

namespace lexy
{
template <typename Input>
class lexeme
{
public:
    using encoding  = typename Input::encoding;
    using char_type = typename encoding::char_type;
    using iterator  = typename Input::iterator;

    constexpr lexeme() noexcept = default;
    constexpr lexeme(iterator begin, iterator end) noexcept : _begin(begin), _end(end) {}

    constexpr explicit lexeme(const Input& input, iterator begin) noexcept
    : _begin(begin), _end(input.cur())
    {}

    constexpr iterator begin() const noexcept
    {
        return _begin;
    }
    constexpr iterator end() const noexcept
    {
        return _end;
    }

    constexpr const char_type* data() const noexcept
    {
        static_assert(std::is_pointer_v<iterator>);
        return _begin;
    }

    constexpr std::size_t size() const noexcept
    {
        return static_cast<std::size_t>(_end - _begin);
    }

    constexpr char_type operator[](std::size_t idx) noexcept
    {
        LEXY_PRECONDITION(idx < size());
        return _begin[idx];
    }

private:
    iterator _begin, _end;
};
} // namespace lexy

#endif // LEXY_LEXEME_HPP_INCLUDED

