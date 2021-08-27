// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED
#define LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

#include <climits>
#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/engine/base.hpp>

namespace lexy
{
/// Matches the inclusive range of characters (code units).
template <auto Min, auto Max>
struct engine_char_range : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        using encoding = typename Reader::encoding;

        auto cur = reader.peek();
        if (_char_to_int_type<encoding>(Min) <= cur && cur <= _char_to_int_type<encoding>(Max))
        {
            reader.bump();
            return error_code();
        }
        else
            return error_code::error;
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code)
    {
        // Consume bad character.
        if (reader.peek() != Reader::encoding::eof())
            reader.bump();
        return true;
    }
};
} // namespace lexy

namespace lexy
{
template <typename CharT, std::size_t TransitionCount>
struct _strie
{
    LEXY_CONSTEVAL auto transition_sequence() const
    {
        return lexy::_detail::make_index_sequence<TransitionCount>{};
    }

    template <typename Encoding>
    LEXY_CONSTEVAL auto transition(std::size_t transition) const
    {
        return _char_to_int_type<Encoding>(_transition[transition]);
    }

    CharT _transition[TransitionCount == 0 ? 1 : TransitionCount];
};

template <typename String>
LEXY_CONSTEVAL auto _make_strie()
{
    constexpr auto string = String::get();

    using strie_t = _strie<typename decltype(string)::char_type, string.size()>;
    strie_t result{};

    for (auto idx = 0u; idx != string.size(); ++idx)
        result._transition[idx] = string[idx];

    return result;
}

/// Produces a shallow trie that contains a transition to an accepting node for each character of
/// the string.
template <typename String>
constexpr auto shallow_trie = _make_strie<String>();

/// Matches one of the specified characters in the trie.
template <const auto& STrie>
struct engine_char_set : engine_matcher_base
{
    enum class error_code
    {
        error = 1,
    };

    template <typename Reader, std::size_t... Transitions>
    static constexpr auto _transition(Reader& reader, lexy::_detail::index_sequence<Transitions...>)
    {
        using encoding = typename Reader::encoding;

        auto cur     = reader.peek();
        auto matches = ((cur == STrie.template transition<encoding>(Transitions)) || ...);
        if (!matches)
            return error_code::error;

        reader.bump();
        return error_code();
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        return _transition(reader, STrie.transition_sequence());
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code)
    {
        // Consume bad character.
        if (reader.peek() != Reader::encoding::eof())
            reader.bump();
        return true;
    }
};

} // namespace lexy

namespace lexy
{
template <std::size_t Bits>
auto _int_n_t_impl()
{
    if constexpr (Bits <= CHAR_BIT)
        return static_cast<unsigned char>(0);
    else if constexpr (Bits <= sizeof(unsigned short) * CHAR_BIT)
        return static_cast<unsigned short>(0);
    else if constexpr (Bits <= sizeof(unsigned int) * CHAR_BIT)
        return static_cast<unsigned int>(0);
    else if constexpr (Bits <= sizeof(unsigned long) * CHAR_BIT)
        return static_cast<unsigned long>(0);
    else if constexpr (Bits <= sizeof(unsigned long long) * CHAR_BIT)
        return static_cast<unsigned long long>(0);
    else
        static_assert(Bits <= sizeof(unsigned long long) * CHAR_BIT, "ASCII table to big");
}
template <std::size_t Bits>
using _int_n_t = decltype(_int_n_t_impl<Bits>());

/// A lookup table for ASCII characters.
template <std::size_t CategoryCount>
class ascii_table
{
    using int_n = _int_n_t<CategoryCount>;

public:
    constexpr ascii_table() : _table() {}

    /// Adds the character to the given category.
    template <typename CharT>
    constexpr ascii_table& insert(CharT c, std::size_t category)
    {
        auto as_unsigned = static_cast<unsigned char>(c);
        LEXY_PRECONDITION(as_unsigned <= 0x7F);
        LEXY_PRECONDITION(category < CategoryCount);

        // Set the given bit.
        _table[as_unsigned] = int_n(_table[as_unsigned] | 1 << category);

        return *this;
    }

    /// Checks whether the character is in the given categories.
    template <typename Encoding, std::size_t... Categories>
    constexpr bool contains(typename Encoding::int_type i) const
    {
        static_assert(((Categories < CategoryCount) && ...));
        constexpr auto mask = ((1 << Categories) | ...);

        if (_char_to_int_type<Encoding>(0x00) <= i && i <= _char_to_int_type<Encoding>(0x7F))
        {
            // NOLINTNEXTLINE: We've checked that we're positive in the condition above.
            auto index = static_cast<std::size_t>(i);
            return (_table[index] & mask) != 0;
        }
        else
        {
            return false;
        }
    }

private:
    int_n _table[0x80];
};

/// Matches one of the specified categories of the ASCII table.
template <const auto& Table, std::size_t... Categories>
struct engine_ascii_table : engine_matcher_base
{
    static_assert(sizeof...(Categories) > 0);

    enum class error_code
    {
        error = 1,
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        auto cur = reader.peek();
        if (Table.template contains<typename Reader::encoding, Categories...>(cur))
        {
            reader.bump();
            return error_code();
        }
        else
        {
            return error_code::error;
        }
    }

    template <typename Reader>
    static constexpr bool recover(Reader& reader, error_code)
    {
        // Consume bad character.
        if (reader.peek() != Reader::encoding::eof())
            reader.bump();
        return true;
    }
};
} // namespace lexy

#endif // LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

