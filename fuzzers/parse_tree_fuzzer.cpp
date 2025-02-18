#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define LEXY_TEST
#include <lexy/action/parse.hpp>
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/buffer.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/parse_tree.hpp>

enum class token_kind
{
    a,
    b,
    c,
};

struct child_p
{
    static constexpr auto name = "child_p";
    static constexpr auto rule = lexy::dsl::any;
};

struct root_p
{
    static constexpr auto name = "root_p";
    static constexpr auto rule = lexy::dsl::any;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    if (Size < 4)
        return 0;

    FuzzedDataProvider fuzz_data(Data, Size);
    std::string        input = fuzz_data.ConsumeRandomLengthString(Size);

    auto buffer = lexy::buffer<lexy::utf8_encoding>(input.data(), input.size());

    using Tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    Tree tree;

    {
        Tree::builder builder(root_p{});

        // Always add at least one token to root to ensure valid positioning
        if (input.size() > 0)
        {
            builder.token(token_kind::a, input.data(), input.data() + 1);
        }

        // Add additional tokens
        size_t num_tokens = fuzz_data.ConsumeIntegralInRange<size_t>(0, 5);
        for (size_t i = 0; i < num_tokens && input.size() > 1; ++i)
        {
            size_t pos = fuzz_data.ConsumeIntegralInRange<size_t>(0, input.size() - 2);
            size_t len = fuzz_data.ConsumeIntegralInRange<size_t>(1, input.size() - pos - 1);

            token_kind kind = static_cast<token_kind>(fuzz_data.ConsumeIntegralInRange<int>(0, 2));

            builder.token(kind, input.data() + pos, input.data() + pos + len);
        }

        // Add productions with guaranteed token content
        if (fuzz_data.ConsumeBool() && input.size() > 2)
        {
            auto marker = builder.start_production(child_p{});

            // Ensure at least one token in production
            size_t pos = fuzz_data.ConsumeIntegralInRange<size_t>(0, input.size() - 2);
            builder.token(token_kind::a, input.data() + pos, input.data() + pos + 1);

            // Add optional additional tokens
            size_t num_child_tokens = fuzz_data.ConsumeIntegralInRange<size_t>(0, 3);
            for (size_t i = 0; i < num_child_tokens && input.size() > pos + 2; ++i)
            {
                size_t token_pos
                    = fuzz_data.ConsumeIntegralInRange<size_t>(pos + 1, input.size() - 2);
                size_t len
                    = fuzz_data.ConsumeIntegralInRange<size_t>(1, input.size() - token_pos - 1);
                builder.token(token_kind::b, input.data() + token_pos,
                              input.data() + token_pos + len);
            }

            builder.finish_production(std::move(marker));
        }

        // Create valid remaining input
        tree = std::move(builder).finish(input.data() + input.size());
    }

    if (!tree.empty())
    {
        auto root = tree.root();

        // Safe traversal checking node type before operations
        for (auto [event, node] : tree.traverse(root))
        {
            // Check node kind first
            auto kind = node.kind();

            if (kind.is_token())
            {
                // Safe operations for token nodes
                node.token();
                node.lexeme();
                node.position();
            }

            if (kind.is_production())
            {
                // Only do production operations if we have children
                auto children = node.children();
                if (!children.empty())
                {
                    // Position is only valid if the production has token descendants
                    node.position();
                    node.covering_lexeme();
                }
                node.parent();
            }

            // Safe operations for all nodes
            if (!kind.is_root())
            {
                auto siblings = node.siblings();
                node.is_last_child();
            }
        }
    }

    return 0;
}
