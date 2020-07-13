// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_STRING_INPUT_HPP_INCLUDED
#define LEXY_INPUT_STRING_INPUT_HPP_INCLUDED

#include <lexy/input/base.hpp>

namespace lexy
{
template <typename Encoding = default_encoding>
class string_input
{
public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    using iterator = const char_type*;

    //=== constructors ===//
    constexpr string_input() noexcept : _cur(nullptr), _end(nullptr) {}

    constexpr string_input(const char_type* begin, const char_type* end) noexcept
    : _cur(begin), _end(end)
    {}
    constexpr string_input(const char_type* data, std::size_t size) noexcept
    : string_input(data, data + size)
    {}

    template <typename CharT, typename = _require_secondary_char_type<Encoding, CharT>>
    string_input(const CharT* begin, const CharT* end) noexcept
    : _cur(reinterpret_cast<iterator>(begin)), _end(reinterpret_cast<iterator>(end))
    {}
    template <typename CharT, typename = _require_secondary_char_type<Encoding, CharT>>
    string_input(const CharT* data, std::size_t size) noexcept : string_input(data, data + size)
    {}

    template <typename View, typename = decltype(LEXY_DECLVAL(View).data())>
    constexpr explicit string_input(const View& view) noexcept
    {
        if constexpr (std::is_same_v<typename View::char_type, char_type>)
        {
            _cur = view.data();
            _end = _cur + view.size();
        }
        else
        {
            static_assert(Encoding::template is_secondary_char_type<typename View::char_type>);
            _cur = reinterpret_cast<iterator>(view.data());
            _end = _cur + view.size();
        }
    }

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
    iterator _cur, _end;
};

template <typename CharT>
string_input(const CharT* begin, const CharT* end) -> string_input<deduce_encoding<CharT>>;
template <typename CharT>
string_input(const CharT* data, std::size_t size) -> string_input<deduce_encoding<CharT>>;
template <typename View>
string_input(const View&) -> string_input<deduce_encoding<typename View::char_type>>;

template <typename Encoding, typename CharT>
constexpr string_input<Encoding> zstring_input(const CharT* str) noexcept
{
    auto end = str;
    while (*end)
        ++end;

    return string_input<Encoding>(str, end);
}
template <typename CharT>
constexpr auto zstring_input(const CharT* str) noexcept
{
    return zstring_input<deduce_encoding<CharT>>(str);
}
} // namespace lexy

#endif // LEXY_INPUT_STRING_INPUT_HPP_INCLUDED
