// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DETAIL_TRIE_HPP_INCLUDED
#define LEXY_DETAIL_TRIE_HPP_INCLUDED

#include <climits>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/input/base.hpp>

namespace lexy::_detail
{
template <typename Encoding, std::size_t NodeCount>
struct _trie
{
    static_assert(NodeCount > 0);

    static auto _index_type()
    {
        // We need to store:
        // * A value for every node, which is the index of the string.
        //   There are at most NodeCount - 1 strings (root node plus one for every one character
        //   string). One value must be free for the invalid value.
        // * An index to a node in the range [0, NodeCount).
        // * An index to a transition, there are at most NodeCount - 1 transitions in a tree.
        // As such, we need values in [0, ..., NodeCount].
        if constexpr (NodeCount <= UCHAR_MAX)
            return static_cast<unsigned char>(0);
        else if constexpr (NodeCount <= USHRT_MAX)
            return static_cast<unsigned short>(0);
        else if constexpr (NodeCount <= UINT_MAX)
            return static_cast<unsigned int>(0);
        else
            return std::size_t(0);
    }
    using index_type = decltype(_index_type());

    static constexpr std::size_t invalid_value = NodeCount;

    LEXY_CONSTEVAL bool empty() const
    {
        return NodeCount == 1 && _node_value[0] == invalid_value;
    }
    LEXY_CONSTEVAL bool accepts_empty() const
    {
        return _node_value[0] != invalid_value;
    }

    // The index of the string.
    LEXY_CONSTEVAL std::size_t node_value(std::size_t node) const
    {
        return _node_value[node];
    }

    LEXY_CONSTEVAL std::size_t transition_count(std::size_t node) const
    {
        auto begin = node == 0 ? 0 : _node_transition_idx[node - 1];
        auto end   = _node_transition_idx[node];
        return std::size_t(end - begin);
    }

    LEXY_CONSTEVAL auto transition_int(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : std::size_t(_node_transition_idx[node - 1]);
        return Encoding::to_int_type(_transition_char[begin + transition]);
    }
    LEXY_CONSTEVAL std::size_t transition_next(std::size_t node, std::size_t transition) const
    {
        auto begin = node == 0 ? 0 : std::size_t(_node_transition_idx[node - 1]);
        return _transition_node[begin + transition];
    }

    // Arrays indexed by nodes.
    // The node has the transitions in the range [_node_transition_idx[node] - 1,
    // _node_transition_idx[node]].
    index_type _node_value[NodeCount];
    index_type _node_transition_idx[NodeCount];

    // Shared array for all transitions.
    index_type                   _transition_node[NodeCount == 1 ? 1 : NodeCount - 1];
    typename Encoding::char_type _transition_char[NodeCount == 1 ? 1 : NodeCount - 1];
};

template <typename Encoding, typename... Strings, std::size_t... Idxs>
LEXY_CONSTEVAL auto _make_trie(lexy::_detail::index_sequence<Idxs...>)
{
    using char_type = typename Encoding::char_type;
    using int_type  = typename Encoding::int_type;

    // We can estimate the number of nodes in the trie by adding all strings together.
    // This is the worst case where the strings don't share any nodes.
    // The plus one comes from the additional root node.
    constexpr auto node_count_upper_bound = (Strings::size + ... + 1);

    // We cannot construct the `_trie` directly as we don't know how many transitions each node has.
    // So we use this temporary representation using an adjacency matrix.
    struct builder_t
    {
        std::size_t node_count;

        std::size_t node_value[node_count_upper_bound];
        int_type    node_transition[node_count_upper_bound][node_count_upper_bound];

        constexpr builder_t() : node_count(1), node_value{std::size_t(-1)}, node_transition{}
        {
            for (auto from = 0u; from != node_count_upper_bound; ++from)
                for (auto to = 0u; to != node_count_upper_bound; ++to)
                    node_transition[from][to] = Encoding::eof();
        }

        constexpr void insert(std::size_t value, const char_type* str, std::size_t size)
        {
            auto cur_node = std::size_t(0);
            for (auto ptr = str; ptr != str + size; ++ptr)
            {
                auto c = Encoding::to_int_type(*ptr);

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

                    cur_node = next_node;
                }
            }

            // A trie (alternative, symbol table) contains duplicate values.
            LEXY_PRECONDITION(node_value[cur_node] == std::size_t(-1));
            node_value[cur_node] = value;
        }
    };
    // We build the trie by inserting all strings.
    constexpr auto builder = [] {
        builder_t builder;
        (builder.insert(Idxs, Strings::template c_str<char_type>, Strings::size), ...);
        return builder;
    }();

    // Now we also now the exact number of nodes in the trie.
    _trie<Encoding, builder.node_count> result{};
    using index_type = typename decltype(result)::index_type;

    // Translate the adjacency matrix representation into the actual trie representation.
    auto transition_idx = 0u;
    for (auto node = 0u; node != builder.node_count; ++node)
    {
        auto value = builder.node_value[node];
        result._node_value[node]
            = value == std::size_t(-1) ? result.invalid_value : index_type(value);

        for (auto next_node = node + 1; next_node != builder.node_count; ++next_node)
            if (auto c = builder.node_transition[node][next_node]; c != Encoding::eof())
            {
                // We've found a transition, add it to the shared transition array.
                result._transition_node[transition_idx] = index_type(next_node);
                result._transition_char[transition_idx] = static_cast<char_type>(c);
                ++transition_idx;
            }

        // The node transition end at the current transition index.
        result._node_transition_idx[node] = index_type(transition_idx);
    }

    return result;
}

/// A trie containing the given strings.
template <typename Encoding, typename... Strings>
constexpr auto trie
    = _make_trie<Encoding, Strings...>(lexy::_detail::index_sequence_for<Strings...>{});

template <const auto& Trie>
struct trie_parser;
template <typename Encoding, std::size_t NodeCount, const _trie<Encoding, NodeCount>& Trie>
struct trie_parser<Trie>
{
    // 0 ... number-of-transitions-of-Node
    template <std::size_t Node>
    using transition_sequence = lexy::_detail::make_index_sequence<Trie.transition_count(Node)>;

    template <std::size_t Node, typename Transitions = transition_sequence<Node>>
    struct handle_node;
    template <std::size_t Node, std::size_t... Transitions>
    struct handle_node<Node, lexy::_detail::index_sequence<Transitions...>>
    {
        template <typename Reader>
        static constexpr std::size_t parse(Reader& reader)
        {
            constexpr auto cur_value = Trie.node_value(Node);
            auto           cur_pos   = reader.position();

            // Compute a longer match.
            auto next_value = [&] {
                auto result = Trie.invalid_value;

                // Find a transition that would match.
                auto next = reader.peek();
                (void)((next == Trie.transition_int(Node, Transitions)
                        ? // We did find a transition that matches, consume the character and take.
                        reader.bump(),
                        result
                        = handle_node<Trie.transition_next(Node, Transitions)>::parse(reader),
                        true // short-circuit
                        : false)
                       || ...);

                return result;
            }();

            // We prefer to return a longer match.
            if (next_value != Trie.invalid_value)
                return next_value;

            // We don't have a longer match, undo everything consumed and return our match.
            reader.set_position(cur_pos);
            return cur_value;
        }
    };
    template <std::size_t Node>
    struct handle_node<Node, lexy::_detail::index_sequence<>>
    {
        template <typename Reader>
        static constexpr std::size_t parse(Reader&)
        {
            // We don't have any transitions, so we always return the current value.
            return Trie.node_value(Node);
        }
    };

    template <typename Reader>
    static constexpr std::size_t parse([[maybe_unused]] Reader& reader)
    {
        if constexpr (Trie.empty())
            return Trie.invalid_value;
        else
            // We start parsing at the root node.
            return handle_node<0>::parse(reader);
    }

    template <typename Reader>
    static constexpr auto try_match([[maybe_unused]] Reader& reader)
    {
        if constexpr (Trie.empty())
            return std::false_type{};
        else
            return parse(reader) != Trie.invalid_value;
    }
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_TRIE_HPP_INCLUDED

