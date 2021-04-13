// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_ENGINE_TRIE_HPP_INCLUDED
#define LEXY_ENGINE_TRIE_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/engine/base.hpp>

namespace lexy
{
template <typename CharT, std::size_t NodeCount, std::size_t TransitionCount>
struct _trie
{
    static constexpr auto invalid_value = std::size_t(-1);

    LEXY_CONSTEVAL bool empty() const
    {
        return NodeCount == 1 && _node_value[0] == invalid_value;
    }
    LEXY_CONSTEVAL bool accepts_empty() const
    {
        return _node_value[0] != invalid_value;
    }

    LEXY_CONSTEVAL std::size_t node_value(std::size_t node) const
    {
        return _node_value[node];
    }

    LEXY_CONSTEVAL auto transition_count(std::size_t node) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        auto end   = _node_transition_idx[node];
        return end - begin;
    }

    LEXY_CONSTEVAL CharT transition_char(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        return _transition_char[begin + transition];
    }
    LEXY_CONSTEVAL std::size_t transition_next(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        return _transition_node[begin + transition];
    }

    // Arrays indexed by nodes.
    // The node has the transitions in the range [_node_transition_idx[node] - 1,
    // _node_transition_idx[node]].
    std::size_t _node_value[NodeCount];
    std::size_t _node_transition_idx[NodeCount];

    // Shared array for all transitions.
    CharT       _transition_char[TransitionCount == 0 ? 1 : TransitionCount];
    std::size_t _transition_node[TransitionCount == 0 ? 1 : TransitionCount];
};

template <typename CharT, typename... Strings, std::size_t... Idxs>
LEXY_CONSTEVAL auto _make_trie(lexy::_detail::index_sequence<Idxs...>)
{
    // We can estimate the number of nodes in the trie by adding all strings together.
    // This is the worst case where the strings don't share any nodes.
    // The plus one comes from the additional root node.
    constexpr auto node_count_upper_bound = (Strings::get().size() + ... + 1);

    // We cannot construct the `_trie` directly as we don't know how many transitions each node has.
    // So we use this temporary representation using an adjacency matrix.
    struct builder_t
    {
        std::size_t node_count       = 1;
        std::size_t transition_count = 0;

        std::size_t node_value[node_count_upper_bound] = {std::size_t(-1)};
        CharT       node_transition[node_count_upper_bound][node_count_upper_bound] = {};

        constexpr void insert(std::size_t value, const CharT* str, std::size_t size)
        {
            auto cur_node = std::size_t(0);
            for (auto ptr = str; ptr != str + size; ++ptr)
            {
                auto c = *ptr;
                LEXY_PRECONDITION(c);

                auto found = false;
                for (auto next_node = cur_node + 1; next_node < node_count; ++next_node)
                {
                    if (node_transition[cur_node][next_node] == c)
                    {
                        // The transition from `cur_node` to `next_node` using `c` is already in
                        // the trie. Follow it.
                        cur_node = next_node;
                        found    = true;
                        break;
                    }
                }

                if (!found)
                {
                    // We haven't found the transition, need to create a new node.
                    auto next_node                       = node_count++;
                    node_value[next_node]                = std::size_t(-1);
                    node_transition[cur_node][next_node] = c;
                    transition_count++;

                    cur_node = next_node;
                }
            }

            LEXY_PRECONDITION(node_value[cur_node]
                              == std::size_t(-1)); // duplicate string in alternative
            node_value[cur_node] = value;
        }
    };
    // We build the trie by inserting all strings.
    constexpr auto builder = [] {
        builder_t builder;
        (builder.insert(Idxs, Strings::get().data(), Strings::get().size()), ...);
        return builder;
    }();

    // Now we also now the exact number of nodes and transitions in the trie.
    _trie<CharT, builder.node_count, builder.transition_count> result{};

    // Translate the adjacency matrix representation into the actual trie representation.
    auto transition_idx = 0u;
    for (auto node = 0u; node != builder.node_count; ++node)
    {
        result._node_value[node] = builder.node_value[node];

        for (auto next_node = node + 1; next_node != builder.node_count; ++next_node)
            if (auto c = builder.node_transition[node][next_node])
            {
                // We've found a transition, add it to the shared transition array.
                result._transition_char[transition_idx] = c;
                result._transition_node[transition_idx] = next_node;
                ++transition_idx;
            }

        // The node transition end at the current transition index.
        result._node_transition_idx[node] = transition_idx;
    }

    return result;
}

/// A trie containing `Strings::get()` for every string.
template <typename CharT, typename... Strings>
constexpr auto trie
    = _make_trie<CharT, Strings...>(lexy::_detail::index_sequence_for<Strings...>{});

/// Matches one of the strings contained in the trie.
template <const auto& Trie>
struct engine_trie : engine_matcher_base, engine_parser_base
{
    enum class error_code
    {
        error = 1,
    };

    template <std::size_t Node>
    using _transition_sequence = lexy::_detail::make_index_sequence<Trie.transition_count(Node)>;

    template <std::size_t Node, typename Transitions>
    struct _node                              // Base case if we pass void as transitions.
    : _node<Node, _transition_sequence<Node>> // Compute transition and forward.
    {};
    template <std::size_t Node, std::size_t... Transitions>
    struct _node<Node, lexy::_detail::index_sequence<Transitions...>>
    {
        template <std::size_t Transition>
        using transition = _node<Trie.transition_next(Node, Transition), void>;

        template <typename Reader>
        static constexpr auto parse(Reader& reader)
        {
            using encoding = typename Reader::encoding;
            auto save      = reader;
            auto cur       = reader.peek();

            auto result = Trie.invalid_value;
            // Check the character of each transition.
            // If it matches, we advance by one and go to that node.
            // As soon as we do that, we return true to short circuit the search.
            (void)((cur == _char_to_int_type<encoding>(Trie.transition_char(Node, Transitions))
                        ? (reader.bump(), result = transition<Transitions>::parse(reader), true)
                        : false)
                   || ...);
            (void)cur;

            if constexpr (Trie.node_value(Node) != Trie.invalid_value)
            {
                // The current node accepts.
                // Check if we have a longer match.
                if (result != Trie.invalid_value)
                    return result;

                // We were unable to find a longer match, but the current node accepts anyway.
                // Return that match.
                reader = LEXY_MOV(save);
                return Trie.node_value(Node);
            }
            else
            {
                // The current node does not accept, so the result is completely determined by the
                // actual result.
                return result;
            }
        }
    };

    template <typename Reader>
    static constexpr error_code match(Reader& reader)
    {
        // We begin in the root node of the trie.
        return _node<0, void>::parse(reader) == Trie.invalid_value ? error_code::error
                                                                   : error_code();
    }

    template <typename Reader>
    static constexpr std::size_t parse(error_code& ec, Reader& reader)
    {
        auto result = _node<0, void>::parse(reader);
        if (result == Trie.invalid_value)
            ec = error_code::error;
        else
            ec = error_code();
        return result;
    }

    template <typename Reader>
    static constexpr bool recover(Reader&, error_code)
    {
        return false;
    }
};

template <const auto& Trie, typename Reader>
inline constexpr bool engine_can_fail<engine_trie<Trie>, Reader> = !Trie.accepts_empty();
template <const auto& Trie, typename Reader>
inline constexpr bool engine_can_succeed<engine_trie<Trie>, Reader> = !Trie.empty();
} // namespace lexy

#endif // LEXY_ENGINE_TRIE_HPP_INCLUDED

