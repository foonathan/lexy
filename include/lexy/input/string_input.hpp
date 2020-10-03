// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_INPUT_STRING_INPUT_HPP_INCLUDED
#define LEXY_INPUT_STRING_INPUT_HPP_INCLUDED

#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
template <typename View>
using _string_view_char_type = std::decay_t<decltype(*LEXY_DECLVAL(View).data())>;

/// An input that refers to a string.
template <typename Encoding = default_encoding>
class string_input
{
public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    using iterator = const char_type*;

    //=== constructors ===//
    constexpr string_input() noexcept : _begin(nullptr), _end(nullptr) {}

    constexpr string_input(const char_type* begin, const char_type* end) noexcept
    : _begin(begin), _end(end)
    {}
    constexpr string_input(const char_type* data, std::size_t size) noexcept
    : string_input(data, data + size)
    {}

    template <typename CharT, typename = _require_secondary_char_type<Encoding, CharT>>
    string_input(const CharT* begin, const CharT* end) noexcept
    : _begin(reinterpret_cast<iterator>(begin)), _end(reinterpret_cast<iterator>(end))
    {}
    template <typename CharT, typename = _require_secondary_char_type<Encoding, CharT>>
    string_input(const CharT* data, std::size_t size) noexcept : string_input(data, data + size)
    {}

    template <typename View, typename CharT = _string_view_char_type<View>>
    constexpr explicit string_input(const View& view) noexcept
    {
        if constexpr (std::is_same_v<CharT, char_type>)
        {
            _begin = view.data();
            _end   = _begin + view.size();
        }
        else
        {
            static_assert(Encoding::template is_secondary_char_type<CharT>);
            _begin = reinterpret_cast<iterator>(view.data());
            _end   = _begin + view.size();
        }
    }

    //=== access ===//
    constexpr iterator begin() const noexcept
    {
        return _begin;
    }

    constexpr iterator end() const noexcept
    {
        return _end;
    }

    //=== reader ===//
    constexpr auto reader() const& noexcept
    {
        return _detail::range_reader<encoding, iterator>(_begin, _end);
    }

private:
    iterator _begin, _end;
};

template <typename CharT>
string_input(const CharT* begin, const CharT* end) -> string_input<deduce_encoding<CharT>>;
template <typename CharT>
string_input(const CharT* data, std::size_t size) -> string_input<deduce_encoding<CharT>>;
template <typename View>
string_input(const View&) -> string_input<deduce_encoding<_string_view_char_type<View>>>;

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

//=== convenience typedefs ===//
template <typename Encoding = default_encoding>
using string_lexeme = lexeme_for<string_input<Encoding>>;

template <typename Tag, typename Encoding = default_encoding>
using string_error = error_for<string_input<Encoding>, Tag>;

template <typename Production, typename Encoding = default_encoding>
using string_error_context = error_context<Production, string_input<Encoding>>;
} // namespace lexy

#endif // LEXY_INPUT_STRING_INPUT_HPP_INCLUDED
