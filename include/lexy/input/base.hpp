// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/encoding.hpp>

#if 0
/// Readers are non-owning, cheaply copyable types.
class Reader
{
public:
    /// The encoding the input uses.
    using encoding = XXX_encoding;

    /// An iterator of char_type, not int_type.
    using iterator = ForwardIterator;

    /// If the reader is at eof, returns Encoding::eof().
    /// Otherwise, returns Encoding::to_int_type(/* current character */).
    typename Encoding::int_type peek() const;

    /// Advances to the next character in the input.
    void bump();

    /// Returns an iterator to the current character.
    /// The following code must produce a valid range:
    /// ```
    /// auto begin = reader.position();
    /// reader.bump();
    /// ... // more bumps
    /// auto end = reader.position();
    /// ```
    iterator position() const;

    /// Sets the reader to a position.
    /// It must be returned by a previous call to `position()` of this reader or a copy,
    /// and can either backtrack the reader or move it forward.
    void set_position(iterator new_pos);
};

/// An Input produces a reader.
class Input
{
public:
    /// Returns a reader to the beginning of the input.
    Reader reader() const &;
};
#endif

namespace lexy::_detail
{
template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class range_reader
{
public:
    using encoding = Encoding;
    using iterator = Iterator;

    constexpr explicit range_reader(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end)
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(begin, end));
    }

    constexpr auto peek() const noexcept
    {
        if (_cur == _end)
            return encoding::eof();
        else
            return encoding::to_int_type(*_cur);
    }

    constexpr void bump() noexcept
    {
        LEXY_PRECONDITION(_cur != _end);
        ++_cur;
    }

    constexpr iterator position() const noexcept
    {
        return _cur;
    }

    constexpr void set_position(iterator new_pos) noexcept
    {
        LEXY_PRECONDITION(new_pos != Iterator());
        LEXY_PRECONDITION(lexy::_detail::precedes(_cur, _end));
        _cur = new_pos;
    }

private:
    Iterator                   _cur;
    LEXY_EMPTY_MEMBER Sentinel _end;
};
} // namespace lexy::_detail

namespace lexy
{
template <typename Input>
using input_reader = decltype(LEXY_DECLVAL(Input).reader());

template <typename Reader, typename CharT>
constexpr bool char_type_compatible_with_reader
    = (std::is_same_v<CharT, typename Reader::encoding::char_type>)
      || Reader::encoding::template is_secondary_char_type<CharT>();

template <typename Reader>
constexpr auto partial_reader(const Reader&, typename Reader::iterator begin,
                              typename Reader::iterator end)
{
    using reader_t = _detail::range_reader<typename Reader::encoding, typename Reader::iterator>;
    return reader_t(begin, end);
}

/// Creates a reader that only reads until the given end.
template <typename Reader>
constexpr auto partial_reader(const Reader& reader, typename Reader::iterator end)
{
    return partial_reader(reader, reader.position(), end);
}
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED

