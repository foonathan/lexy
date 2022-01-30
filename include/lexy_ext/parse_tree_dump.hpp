// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_PARSE_TREE_DUMP_HPP_INCLUDED
#define LEXY_EXT_PARSE_TREE_DUMP_HPP_INCLUDED

#include <cctype>
#include <cstdio>
#include <lexy/parse_tree.hpp>
#include <vector>

namespace lexy_ext
{
struct dump_parse_tree_label
{
    const char* space;
    const char* line;
    const char* end;
    const char* branch;
};

constexpr dump_parse_tree_label simple_parse_tree_dump = {"  ", "  ", "- ", "- "};
constexpr dump_parse_tree_label fancy_parse_tree_dump  = {"   ", "│  ", "└──", "├──"};

template <typename Reader, typename TokenKind, typename MemoryResource>
[[deprecated("use `lexy_ext::visualize(file, tree)` instead")]] void dump_parse_tree(
    std::FILE* out, const lexy::parse_tree<Reader, TokenKind, MemoryResource>& tree,
    dump_parse_tree_label label = fancy_parse_tree_dump)
{
    std::vector<bool> prefix_info;

    auto print_prefix = [&](bool is_last_child) {
        if (prefix_info.empty())
            return; // Nothing to do for root node.

        // We need to skip the first entry.
        for (auto iter = std::next(prefix_info.begin()); iter != prefix_info.end(); ++iter)
        {
            if (*iter)
                // At this column we have no more children currently, so just indent.
                std::fputs(label.space, out);
            else
                // We do have more children, carry the line.
                std::fputs(label.line, out);
        }

        if (is_last_child)
            std::fputs(label.end, out);
        else
            std::fputs(label.branch, out);
    };

    for (auto [event, node] : tree.traverse())
    {
        auto last_child = node.is_last_child();
        switch (event)
        {
        case lexy::traverse_event::enter:
            print_prefix(last_child);
            prefix_info.push_back(last_child);
            std::fprintf(out, "%s:\n", node.kind().name());
            break;

        case lexy::traverse_event::exit:
            prefix_info.pop_back();
            break;

        case lexy::traverse_event::leaf:
            print_prefix(last_child);

            std::fprintf(out, "%s", node.kind().name());
            if (!node.lexeme().empty())
            {
                std::fputs(": \"", out);
                for (auto c : node.lexeme())
                {
                    if (c == '"')
                        std::fputs(R"(\")", out);
                    else if (c == '\n')
                        std::fputs("\\n", out);
                    else if (c == '\r')
                        std::fputs("\\r", out);
                    else if (std::isprint(c))
                        std::fputc(c, out);
                    else
                        std::fprintf(out, "\\x%02X", unsigned(c) & 0xFF);
                }
                std::fputs("\"", out);
            }
            std::fputs("\n", out);
            break;
        }
    }
}
} // namespace lexy_ext

#endif // LEXY_EXT_PARSE_TREE_DUMP_HPP_INCLUDED

