// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>

#if 0
/// Readers are non-owning, cheaply copyable types.
class Reader
{
public:
    /// The encoding the input uses.
    using encoding = XXX_encoding;
    using char_type = typename encoding::char_type;

    /// An iterator of char_type, not int_type.
    using iterator = ForwardIterator;

    /// A reader with the same encoding and iterator that is used for reporting errors/lexemes.
    /// If you're writing an Input, the Input's Reader is always canonical.
    using canonical_reader = Reader;

    /// Checks whether the reader is at EOF.
    bool eof() const;

    /// If the reader is at eof, returns Encoding::eof().
    /// Otherwise, returns Encoding::to_int_type(/* current character */).
    typename Encoding::int_type peek() const;

    /// Advances to the next character in the input.
    void bump();

    /// Returns an iterator to the current character.
    /// The following code must produce a valid range:
    /// ```
    /// auto begin = reader.cur();
    /// reader.bump();
    /// ... // more bumps
    /// auto end = reader.cur();
    /// ```
    iterator cur() const;
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
template <typename I>
constexpr auto range_size(I begin, I end) -> decltype(std::size_t(end - begin))
{
    return std::size_t(end - begin);
}
template <typename I, typename I2> // always worse match because two different params
constexpr auto range_size(I begin, I2 end)
{
    std::size_t result = 0;
    for (auto cur = begin; cur != end; ++cur)
        ++result;
    return result;
}

template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class range_reader
{
public:
    using encoding         = Encoding;
    using char_type        = typename encoding::char_type;
    using iterator         = Iterator;
    using canonical_reader = range_reader<Encoding, Iterator, Sentinel>;

    constexpr explicit range_reader(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end)
    {}

    constexpr bool eof() const noexcept
    {
        return _cur == _end;
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
        ++_cur;
    }

    constexpr iterator cur() const noexcept
    {
        return _cur;
    }

    constexpr void _make_eof() noexcept
    {
        static_assert(std::is_same_v<Iterator, Sentinel>);
        _cur = _end;
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
    = (std::is_same_v<CharT, typename Reader::char_type>)
      || Reader::encoding::template is_secondary_char_type<CharT>;

template <typename Reader>
constexpr bool is_canonical_reader = std::is_same_v<typename Reader::canonical_reader, Reader>;

/// Creates a reader that only reads until the given end.
template <typename Reader>
constexpr auto partial_reader(Reader reader, typename Reader::iterator end)
{
    struct partial_reader_t
    : _detail::range_reader<typename Reader::encoding, typename Reader::iterator>
    {
        using canonical_reader = Reader;
        using _detail::range_reader<typename Reader::encoding,
                                    typename Reader::iterator>::range_reader;
    };
    return partial_reader_t(reader.cur(), end);
}
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED
