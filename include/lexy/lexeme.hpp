// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_LEXEME_HPP_INCLUDED
#define LEXY_LEXEME_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>
#include <lexy/input/base.hpp>

namespace lexy
{
template <typename Reader>
class lexeme
{
public:
    using encoding  = typename Reader::encoding;
    using char_type = typename encoding::char_type;
    using iterator  = typename Reader::iterator;

    constexpr lexeme() noexcept = default;
    constexpr lexeme(iterator begin, iterator end) noexcept : _begin(begin), _end(end) {}

    constexpr explicit lexeme(const Reader& reader, iterator begin) noexcept
    : _begin(begin), _end(reader.cur())
    {}

    constexpr bool empty() const noexcept
    {
        return _begin == _end;
    }

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

    constexpr char_type operator[](std::size_t idx) const noexcept
    {
        LEXY_PRECONDITION(idx < size());
        return _begin[idx];
    }

    template <typename String, typename = decltype(String(iterator(), iterator()))>
    constexpr explicit operator String() const
    {
        if constexpr (std::is_pointer_v<iterator>)
            return String(data(), size());
        else
            return String(begin(), end());
    }

private:
    iterator _begin, _end;
};

template <typename Input>
using lexeme_for = lexeme<input_reader<Input>>;
} // namespace lexy

namespace lexy
{
template <typename String>
struct _as_string
{
    using return_type = String;

    template <typename Reader>
    constexpr String operator()(const lexeme<Reader>& lex) const
    {
        return String(lex);
    }
};

/// A callback that turns a lexeme into a string.
template <typename String>
constexpr auto as_string = _as_string<String>{};
} // namespace lexy

#endif // LEXY_LEXEME_HPP_INCLUDED
