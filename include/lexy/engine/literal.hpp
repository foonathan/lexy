// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_LITERAL_HPP_INCLUDED
#define LEXY_ENGINE_LITERAL_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/engine/base.hpp>

namespace lexy
{
template <typename CharT, std::size_t NodeCount>
struct _ltrie
{
    LEXY_CONSTEVAL bool empty() const
    {
        return NodeCount == 0;
    }

    LEXY_CONSTEVAL auto node_sequence() const
    {
        return lexy::_detail::make_index_sequence<NodeCount>{};
    }

    template <typename Encoding>
    LEXY_CONSTEVAL auto transition(std::size_t node) const
    {
        return _char_to_int_type<Encoding>(_transition[node]);
    }

    CharT _transition[NodeCount == 0 ? 1 : NodeCount];
};

template <typename String>
LEXY_CONSTEVAL auto _make_ltrie()
{
    constexpr auto string = String::get();

    using ltrie_t = _ltrie<typename decltype(string)::char_type, string.size()>;
    ltrie_t result{};

    for (auto idx = 0u; idx != string.size(); ++idx)
        result._transition[idx] = string[idx];

    return result;
}

/// Produces a linear trie, i.e. one that consists of only one string.
template <typename String>
constexpr auto linear_trie = _make_ltrie<String>();

/// Matches the linear trie.
template <const auto& LTrie>
struct engine_literal : engine_matcher_base
{
    enum class error_code
    {
    };

    static LEXY_CONSTEVAL error_code index_to_error(std::size_t idx)
    {
        return error_code(idx + 1);
    }

    static constexpr std::size_t index_from_error(error_code ec)
    {
        return std::size_t(ec) - 1;
    }

    template <typename Reader, std::size_t... Nodes>
    static constexpr auto _transition(Reader& reader, lexy::_detail::index_sequence<Nodes...>)
    {
        using encoding = typename Reader::encoding;

        auto result = error_code();
        (void)((reader.peek() == LTrie.template transition<encoding>(Nodes)
                    ? (reader.bump(), true)
                    : (result = index_to_error(Nodes), false))
               && ...);
        return result;
    }

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        return _transition(reader, LTrie.node_sequence());
    }
};

template <const auto& LTrie, typename Reader>
inline constexpr bool engine_can_fail<engine_literal<LTrie>, Reader> = !LTrie.empty();
} // namespace lexy

#endif // LEXY_ENGINE_LITERAL_HPP_INCLUDED

