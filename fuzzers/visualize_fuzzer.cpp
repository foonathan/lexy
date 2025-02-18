#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <iterator>
#include <string>

#define LEXY_TEST
#include <lexy/dsl/any.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/visualize.hpp>

// Keep token kinds for visualization testing
enum class token_kind
{
    a,
    b,
    c,
};

const char* token_kind_name(token_kind k)
{
    switch (k)
    {
    case token_kind::a:
        return "a";
    case token_kind::b:
        return "b";
    case token_kind::c:
        return "c";
    default:
        return "";
    }
}

// Helper to generate potentially problematic strings for write_str
std::string generate_deep_string(FuzzedDataProvider& fuzz_data)
{
    std::string result;

    // Create deeply nested string with special characters
    size_t depth = fuzz_data.ConsumeIntegralInRange<size_t>(1, 1000);
    for (size_t i = 0; i < depth; i++)
    {
        // Add potentially problematic patterns that will cause write_str recursion
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 3))
        {
        case 0: // Unicode escapes
            result += "\\u";
            result += fuzz_data.ConsumeRandomLengthString(4);
            break;
        case 1: // Nested escapes
            result += "\\\\\\";
            break;
        case 2: // Control characters
            result += std::string(1, fuzz_data.ConsumeIntegralInRange<char>(0, 31));
            break;
        case 3: // Special characters
            result += "\t\n\r\"'<>&";
            break;
        }
    }
    return result;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    if (Size < 4)
        return 0;

    FuzzedDataProvider fuzz_data(Data, Size);

    // Create output string and iterator
    std::string output;
    auto        out = std::back_inserter(output);

    // Create visualization options with fuzz data
    lexy::visualization_options opts;
    opts.flags
        = static_cast<lexy::visualization_flags>(fuzz_data.ConsumeIntegralInRange<unsigned>(0, 15));
    opts.max_tree_depth   = fuzz_data.ConsumeIntegralInRange<unsigned char>(0, 32);
    opts.max_lexeme_width = fuzz_data.ConsumeIntegralInRange<unsigned char>(0, 255);
    opts.tab_width        = fuzz_data.ConsumeIntegralInRange<unsigned char>(0, 8);

    try
    {
        // Test 1: Code point visualization
        {
            // Generate some interesting code points
            std::vector<lexy::code_point> code_points;

            // Test control characters
            code_points.push_back(lexy::code_point('\0'));
            code_points.push_back(lexy::code_point('\n'));
            code_points.push_back(lexy::code_point('\r'));
            code_points.push_back(lexy::code_point('\t'));
            code_points.push_back(lexy::code_point('\\'));

            // Add fuzzed code points
            while (fuzz_data.remaining_bytes() > 4)
            {
                auto cp_value = fuzz_data.ConsumeIntegral<char32_t>();
                code_points.push_back(lexy::code_point(cp_value));
            }

            // Test all visualization flag combinations
            for (const auto& cp : code_points)
            {
                lexy::visualize_to(out, cp, opts);
                lexy::visualize_to(out, cp, opts | lexy::visualize_use_unicode);
                lexy::visualize_to(out, cp, opts | lexy::visualize_use_color);
                lexy::visualize_to(out, cp, opts | lexy::visualize_use_symbols);
                lexy::visualize_to(out, cp, opts | lexy::visualize_space);
            }
        }

        // Test 2: Deep string visualization targeting write_str
        {
            auto deep_string = generate_deep_string(fuzz_data);
            auto input       = lexy::zstring_input(deep_string.c_str());
            auto lexeme      = lexy::lexeme_for<decltype(input)>(input.data(),
                                                                 input.data() + deep_string.size());

            // Test with different encodings
            lexy::visualize_to(out, lexeme, opts);
            lexy::visualization_display_width(lexeme, opts);
        }

        // Test 3: Regular lexeme visualization
        {
            std::string test_input
                = fuzz_data.ConsumeRandomLengthString(std::min(Size, size_t(64)));

            auto input = lexy::zstring_input(test_input.c_str());
            auto lexeme
                = lexy::lexeme_for<decltype(input)>(input.data(), input.data() + test_input.size());

            lexy::visualize_to(out, lexeme, opts);
            lexy::visualize_to(out, lexeme, {opts.flags | lexy::visualize_use_unicode});
            lexy::visualize_to(out, lexeme, {opts.flags | lexy::visualize_use_symbols});
            lexy::visualization_display_width(lexeme, opts);
        }
    }
    catch (...)
    {}

    return 0;
}
