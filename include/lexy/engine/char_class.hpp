// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED
#define LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

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
        if (encoding::to_int_type(Min) <= cur && cur <= encoding::to_int_type(Max))
        {
            reader.bump();
            return error_code();
        }
        else
            return error_code::error;
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
        auto c = static_cast<typename Encoding::char_type>(_transition[transition]);
        return Encoding::to_int_type(c);
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
};

} // namespace lexy

#endif // LEXY_ENGINE_CHAR_CLASS_HPP_INCLUDED

